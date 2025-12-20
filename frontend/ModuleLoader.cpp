#include "ModuleLoader.h"
#include "Lexer.h"
#include "Parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace mana::frontend {

    ModuleLoader::ModuleLoader(DiagnosticEngine& diag)
        : diag_(diag) {
        // Set default std lib path
#ifdef _WIN32
        // Windows: check common locations
        const char* home = std::getenv("USERPROFILE");
        if (home) {
            std_lib_path_ = std::string(home) + "/.mana/lib";
        }
#else
        // Unix: ~/.mana/lib
        const char* home = std::getenv("HOME");
        if (home) {
            std_lib_path_ = std::string(home) + "/.mana/lib";
        }
#endif
        // Also check MANA_LIB environment variable
        const char* mana_lib = std::getenv("MANA_LIB");
        if (mana_lib) {
            std_lib_path_ = mana_lib;
        }
    }

    void ModuleLoader::add_search_path(const std::string& path) {
        search_paths_.push_back(normalize_path(path));
    }

    void ModuleLoader::set_project_root(const std::string& path) {
        project_root_ = normalize_path(path);
    }

    std::string ModuleLoader::normalize_path(const std::string& path) {
        try {
            if (std::filesystem::exists(path)) {
                return std::filesystem::canonical(path).string();
            }
            return std::filesystem::absolute(path).string();
        } catch (...) {
            return path;
        }
    }

    std::vector<std::string> ModuleLoader::module_path_to_files(const std::string& module_path) {
        std::vector<std::string> results;

        // Convert "std::io::file" to "std/io/file"
        std::string rel_path;
        for (char c : module_path) {
            if (c == ':') continue;  // Skip ::
            rel_path += (c == ':') ? '/' : c;
        }

        // Fix double replacement - actually convert :: to /
        rel_path.clear();
        auto parts = module_utils::split_module_path(module_path);
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i > 0) rel_path += '/';
            rel_path += parts[i];
        }

        // Try with .mana extension
        std::string file_path = rel_path + ".mana";

        // Also try as directory with mod.mana (like Rust)
        std::string dir_path = rel_path + "/mod.mana";

        // Check standard library first for std:: modules
        if (module_utils::is_valid_module_path(module_path) &&
            module_path.substr(0, 4) == "std:") {
            if (!std_lib_path_.empty()) {
                results.push_back(std_lib_path_ + "/" + file_path);
                results.push_back(std_lib_path_ + "/" + dir_path);
            }
        }

        // Check project root
        if (!project_root_.empty()) {
            results.push_back(project_root_ + "/src/" + file_path);
            results.push_back(project_root_ + "/src/" + dir_path);
            results.push_back(project_root_ + "/" + file_path);
            results.push_back(project_root_ + "/" + dir_path);
        }

        // Check search paths
        for (const auto& search : search_paths_) {
            results.push_back(search + "/" + file_path);
            results.push_back(search + "/" + dir_path);
        }

        // Check current directory
        results.push_back(file_path);
        results.push_back(dir_path);

        return results;
    }

    ModuleResolution ModuleLoader::resolve_module(const std::string& module_path,
                                                   const std::string& from_file) {
        ModuleResolution result;

        // Check if already loaded
        auto it = modules_.find(module_path);
        if (it != modules_.end()) {
            result.found = true;
            result.file_path = it->second->file_path;
            return result;
        }

        // Get possible file paths
        auto candidates = module_path_to_files(module_path);

        // If we have a source file, also check relative to it
        if (!from_file.empty()) {
            std::filesystem::path from_dir = std::filesystem::path(from_file).parent_path();
            auto parts = module_utils::split_module_path(module_path);
            std::string rel_path;
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) rel_path += '/';
                rel_path += parts[i];
            }
            candidates.insert(candidates.begin(), (from_dir / (rel_path + ".mana")).string());
            candidates.insert(candidates.begin(), (from_dir / rel_path / "mod.mana").string());
        }

        // Try each candidate
        for (const auto& candidate : candidates) {
            if (std::filesystem::exists(candidate)) {
                result.found = true;
                result.file_path = normalize_path(candidate);
                return result;
            }
        }

        result.error = "module not found: " + module_path;
        return result;
    }

    ModuleResolution ModuleLoader::resolve_file_import(const std::string& import_path,
                                                        const std::string& from_file) {
        ModuleResolution result;

        // Remove quotes if present
        std::string path = import_path;
        if (!path.empty() && path.front() == '"') path = path.substr(1);
        if (!path.empty() && path.back() == '"') path.pop_back();

        // If absolute path, use directly
        if (std::filesystem::path(path).is_absolute()) {
            if (std::filesystem::exists(path)) {
                result.found = true;
                result.file_path = normalize_path(path);
                return result;
            }
            result.error = "file not found: " + path;
            return result;
        }

        // Resolve relative to importing file
        if (!from_file.empty()) {
            std::filesystem::path from_dir = std::filesystem::path(from_file).parent_path();
            std::filesystem::path resolved = from_dir / path;
            if (std::filesystem::exists(resolved)) {
                result.found = true;
                result.file_path = normalize_path(resolved.string());
                return result;
            }
        }

        // Try project root
        if (!project_root_.empty()) {
            std::filesystem::path resolved = std::filesystem::path(project_root_) / path;
            if (std::filesystem::exists(resolved)) {
                result.found = true;
                result.file_path = normalize_path(resolved.string());
                return result;
            }
        }

        // Try current directory
        if (std::filesystem::exists(path)) {
            result.found = true;
            result.file_path = normalize_path(path);
            return result;
        }

        result.error = "file not found: " + path;
        return result;
    }

    LoadedModule* ModuleLoader::load_module(const std::string& module_path,
                                             const std::string& from_file) {
        // Check cache
        auto it = modules_.find(module_path);
        if (it != modules_.end()) {
            return it->second.get();
        }

        // Check for cycles
        if (loading_.count(module_path)) {
            diag_.error("circular module dependency: " + module_path, 0, 0);
            return nullptr;
        }

        // Resolve module path
        auto resolution = resolve_module(module_path, from_file);
        if (!resolution.found) {
            diag_.error(resolution.error, 0, 0);
            return nullptr;
        }

        // Mark as loading
        loading_.insert(module_path);

        // Load from file
        auto* loaded = load_file(resolution.file_path);

        // Unmark loading
        loading_.erase(module_path);

        if (loaded) {
            loaded->name = module_path;
            modules_[module_path] = std::unique_ptr<LoadedModule>(loaded);
            file_to_module_[resolution.file_path] = module_path;
            return modules_[module_path].get();
        }

        return nullptr;
    }

    LoadedModule* ModuleLoader::load_file(const std::string& file_path) {
        // Check if already loaded by path
        auto fit = file_to_module_.find(file_path);
        if (fit != file_to_module_.end()) {
            return get_module(fit->second);
        }

        // Read file
        std::ifstream file(file_path);
        if (!file.is_open()) {
            diag_.error("cannot open file: " + file_path, 0, 0);
            return nullptr;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        // Lex
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        // Parse
        Parser parser(tokens, diag_);
        auto ast = parser.parse_module();

        if (!ast) {
            diag_.error("failed to parse: " + file_path, 0, 0);
            return nullptr;
        }

        // Create loaded module
        auto* loaded = new LoadedModule();
        loaded->file_path = file_path;
        loaded->name = file_path_to_module(file_path);
        loaded->ast = std::move(ast);

        return loaded;
    }

    LoadedModule* ModuleLoader::get_module(const std::string& module_path) {
        auto it = modules_.find(module_path);
        return it != modules_.end() ? it->second.get() : nullptr;
    }

    LoadedModule* ModuleLoader::get_module_by_path(const std::string& file_path) {
        auto fit = file_to_module_.find(file_path);
        if (fit != file_to_module_.end()) {
            return get_module(fit->second);
        }
        return nullptr;
    }

    void ModuleLoader::clear_cache() {
        modules_.clear();
        file_to_module_.clear();
    }

    void ModuleLoader::register_exports(LoadedModule* module) {
        if (!module || !module->ast) return;

        for (auto& decl : module->ast->decls) {
            // Only export public declarations
            if (!decl->is_public()) continue;

            Symbol sym;
            sym.is_public = true;
            sym.source_module = module->name;

            if (auto* func = dynamic_cast<AstFuncDecl*>(decl.get())) {
                sym.name = func->name;
                sym.type = Type::unknown();  // Will be filled during analysis
                module->exports[func->name] = sym;
            }
            else if (auto* strct = dynamic_cast<AstStructDecl*>(decl.get())) {
                sym.name = strct->name;
                sym.type = Type::struct_(strct->name);
                module->exports[strct->name] = sym;
            }
            else if (auto* enm = dynamic_cast<AstEnumDecl*>(decl.get())) {
                sym.name = enm->name;
                sym.type = Type::enum_(enm->name);
                module->exports[enm->name] = sym;
            }
            else if (auto* trt = dynamic_cast<AstTraitDecl*>(decl.get())) {
                sym.name = trt->name;
                sym.type = Type::struct_(trt->name);  // Traits treated as struct-like
                module->exports[trt->name] = sym;
            }
            else if (auto* alias = dynamic_cast<AstTypeAliasDecl*>(decl.get())) {
                sym.name = alias->alias_name;
                sym.type = Type::unknown();  // Will be resolved during analysis
                module->exports[alias->alias_name] = sym;
            }
        }
    }

    Symbol* ModuleLoader::get_export(const std::string& module_path, const std::string& name) {
        auto* module = get_module(module_path);
        if (!module) return nullptr;

        auto it = module->exports.find(name);
        return it != module->exports.end() ? &it->second : nullptr;
    }

    std::vector<std::pair<std::string, Symbol>> ModuleLoader::get_all_exports(const std::string& module_path) {
        std::vector<std::pair<std::string, Symbol>> result;

        auto* module = get_module(module_path);
        if (!module) return result;

        for (auto& [name, sym] : module->exports) {
            result.emplace_back(name, sym);
        }

        return result;
    }

    std::string ModuleLoader::file_path_to_module(const std::string& file_path) {
        // Extract module name from file path
        // e.g., "src/utils/math.mana" -> "utils::math"
        std::filesystem::path p(file_path);
        std::string name = p.stem().string();  // Remove extension

        // If it's mod.mana, use parent directory name
        if (name == "mod") {
            name = p.parent_path().filename().string();
        }

        return name;
    }

    bool ModuleLoader::is_std_module(const std::string& module_path) {
        return module_path.size() >= 3 &&
               (module_path.substr(0, 3) == "std" &&
                (module_path.size() == 3 || module_path[3] == ':'));
    }

    // Module path utilities
    namespace module_utils {

        std::vector<std::string> split_module_path(const std::string& path) {
            std::vector<std::string> parts;
            std::string current;

            for (size_t i = 0; i < path.size(); ++i) {
                if (i + 1 < path.size() && path[i] == ':' && path[i + 1] == ':') {
                    if (!current.empty()) {
                        parts.push_back(current);
                        current.clear();
                    }
                    ++i;  // Skip second :
                } else {
                    current += path[i];
                }
            }

            if (!current.empty()) {
                parts.push_back(current);
            }

            return parts;
        }

        std::string join_module_path(const std::vector<std::string>& parts) {
            std::string result;
            for (size_t i = 0; i < parts.size(); ++i) {
                if (i > 0) result += "::";
                result += parts[i];
            }
            return result;
        }

        std::string parent_module(const std::string& path) {
            auto parts = split_module_path(path);
            if (parts.size() <= 1) return "";
            parts.pop_back();
            return join_module_path(parts);
        }

        std::string module_name(const std::string& path) {
            auto parts = split_module_path(path);
            return parts.empty() ? "" : parts.back();
        }

        bool is_valid_module_path(const std::string& path) {
            if (path.empty()) return false;

            auto parts = split_module_path(path);
            for (const auto& part : parts) {
                if (part.empty()) return false;

                // Check valid identifier
                if (!std::isalpha(part[0]) && part[0] != '_') return false;
                for (char c : part) {
                    if (!std::isalnum(c) && c != '_') return false;
                }
            }

            return true;
        }

    } // namespace module_utils

} // namespace mana::frontend
