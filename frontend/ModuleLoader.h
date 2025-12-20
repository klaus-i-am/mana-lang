#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>
#include <set>

#include "AstModule.h"
#include "AstDecl.h"
#include "Symbol.h"
#include "Diagnostic.h"

namespace mana::frontend {

    // Represents a loaded module with its exports
    struct LoadedModule {
        std::string name;                           // Module name (e.g., "std::io")
        std::string file_path;                      // Absolute file path
        std::unique_ptr<AstModule> ast;             // Parsed AST
        std::unordered_map<std::string, Symbol> exports;  // Public symbols
        std::vector<std::string> dependencies;      // Modules this depends on
        bool analyzed = false;                      // Has been semantically analyzed
    };

    // Module resolution result
    struct ModuleResolution {
        bool found = false;
        std::string file_path;
        std::string error;
    };

    // Handles loading and caching of modules
    class ModuleLoader {
    public:
        explicit ModuleLoader(DiagnosticEngine& diag);

        // Add search paths for module resolution
        void add_search_path(const std::string& path);
        void set_project_root(const std::string& path);

        // Resolve module path to file
        // "std::io" -> "/usr/local/mana/lib/std/io.mana"
        // "my_module" -> "./src/my_module.mana"
        ModuleResolution resolve_module(const std::string& module_path,
                                        const std::string& from_file = "");

        // Resolve file import
        // import "path/to/file.mana"
        ModuleResolution resolve_file_import(const std::string& import_path,
                                             const std::string& from_file);

        // Load and parse a module (uses cache)
        LoadedModule* load_module(const std::string& module_path,
                                  const std::string& from_file = "");

        // Load from file path directly
        LoadedModule* load_file(const std::string& file_path);

        // Get cached module
        LoadedModule* get_module(const std::string& module_path);
        LoadedModule* get_module_by_path(const std::string& file_path);

        // Clear module cache
        void clear_cache();

        // Get all loaded modules
        const std::unordered_map<std::string, std::unique_ptr<LoadedModule>>& modules() const {
            return modules_;
        }

        // Register exports from a module
        void register_exports(LoadedModule* module);

        // Get exported symbol from module
        Symbol* get_export(const std::string& module_path, const std::string& name);

        // Get all exports from module (for glob imports)
        std::vector<std::pair<std::string, Symbol>> get_all_exports(const std::string& module_path);

        // Standard library module paths
        static bool is_std_module(const std::string& module_path);
        std::string get_std_lib_path() const { return std_lib_path_; }
        void set_std_lib_path(const std::string& path) { std_lib_path_ = path; }

    private:
        DiagnosticEngine& diag_;

        // Module cache: module_path -> LoadedModule
        std::unordered_map<std::string, std::unique_ptr<LoadedModule>> modules_;

        // File path to module path mapping
        std::unordered_map<std::string, std::string> file_to_module_;

        // Search paths for module resolution
        std::vector<std::string> search_paths_;
        std::string project_root_;
        std::string std_lib_path_;

        // Modules currently being loaded (for cycle detection)
        std::set<std::string> loading_;

        // Convert module path to possible file paths
        std::vector<std::string> module_path_to_files(const std::string& module_path);

        // Normalize path
        std::string normalize_path(const std::string& path);

        // Extract module name from file path
        std::string file_path_to_module(const std::string& file_path);
    };

    // Module path utilities
    namespace module_utils {
        // "std::io::file" -> ["std", "io", "file"]
        std::vector<std::string> split_module_path(const std::string& path);

        // ["std", "io", "file"] -> "std::io::file"
        std::string join_module_path(const std::vector<std::string>& parts);

        // "std::io::file" -> "std::io"
        std::string parent_module(const std::string& path);

        // "std::io::file" -> "file"
        std::string module_name(const std::string& path);

        // Check if path is valid module path
        bool is_valid_module_path(const std::string& path);
    }

} // namespace mana::frontend
