#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <unordered_set>

#include "../frontend/Lexer.h"
#include "../frontend/Parser.h"
#include "../frontend/Semantic.h"
#include "../frontend/AstPrinter.h"
#include "../frontend/AstDeclarations.h"
#include "../backend-cpp/CppEmitter.h"
#include "../backend-cpp/DocGenerator.h"
#include "../frontend/Cache.h"
#include "../middle/ForLowering.h"
#include "../middle/DeadCodeElimination.h"
#include "../middle/Inlining.h"
#include "../tools/fmt/Formatter.h"
#include "../tools/repl/Repl.h"
#include "../tools/pkg/PackageManager.h"
#include "../tools/test/TestRunner.h"

using namespace mana::frontend;
using namespace mana::backend;
namespace fs = std::filesystem;

// Forward declaration for recursive import resolution
static bool resolve_imports(AstModule* module, const fs::path& base_dir,
                           DiagnosticEngine& diag,
                           std::unordered_set<std::string>& imported_files);

static std::unique_ptr<AstModule> parse_file(const std::string& filepath, DiagnosticEngine& diag) {
    std::ifstream in(filepath);
    if (!in) {
        diag.error("cannot open imported file: " + filepath, 0, 0);
        return nullptr;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    Lexer lex(source);
    auto tokens = lex.tokenize();

    Parser parser(tokens, diag);
    return parser.parse_module();
}

static bool resolve_imports(AstModule* module, const fs::path& base_dir,
                           DiagnosticEngine& diag,
                           std::unordered_set<std::string>& imported_files) {
    std::vector<std::unique_ptr<AstDecl>> imported_decls;

    for (auto& decl : module->decls) {
        if (auto* imp = dynamic_cast<AstImportDecl*>(decl.get())) {
            if (imp->is_file_import) {
                // Resolve file path relative to current file
                fs::path import_path = base_dir / (imp->path + ".mana");
                std::string canonical = fs::weakly_canonical(import_path).string();

                // Skip if already imported (avoid circular imports)
                if (imported_files.count(canonical)) continue;
                imported_files.insert(canonical);

                // Parse the imported file
                auto imported_module = parse_file(canonical, diag);
                if (!imported_module || diag.has_errors()) {
                    return false;
                }

                // Recursively resolve imports in the imported file
                if (!resolve_imports(imported_module.get(), import_path.parent_path(), diag, imported_files)) {
                    return false;
                }

                // Import all declarations (both public and private)
                // Visibility is enforced at semantic analysis, not import time
                // This allows public functions to call private helpers
                std::string module_name = imported_module->name.empty() ? canonical : imported_module->name;
                for (auto& imported_decl : imported_module->decls) {
                    // Don't re-import import or use declarations
                    if (dynamic_cast<AstImportDecl*>(imported_decl.get())) continue;
                    if (dynamic_cast<AstUseDecl*>(imported_decl.get())) continue;

                    // Track source module for all imported declarations
                    imported_decl->source_module = module_name;
                    imported_decls.push_back(std::move(imported_decl));
                }
            }
            // Standard library imports handled separately
        }
    }

    // Add imported declarations to the beginning of the module (in correct order)
    // We need to insert them in order, so insert from the end to the beginning
    for (auto it = imported_decls.rbegin(); it != imported_decls.rend(); ++it) {
        module->decls.insert(module->decls.begin(), std::move(*it));
    }

    return true;
}

// Embedded runtime header content
static const char* MANA_RUNTIME_H = R"(#pragma once
#include <utility>
#include <cstdio>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cctype>

namespace mana {
    template <typename F>
    struct Defer {
        F fn;
        explicit Defer(F f) : fn(std::move(f)) {}
        ~Defer() { fn(); }
        Defer(const Defer&) = delete;
        Defer& operator=(const Defer&) = delete;
    };

    template <typename F>
    Defer<F> defer(F f) { return Defer<F>(std::move(f)); }

    template <typename T>
    inline void drop(T&) {}

    // Forward declaration for None
    struct None_t {};
    inline constexpr None_t None{};

    // Option<T> - represents an optional value
    template <typename T>
    class Option {
        bool has_value_;
        alignas(T) unsigned char storage_[sizeof(T)];

        T* ptr() { return reinterpret_cast<T*>(storage_); }
        const T* ptr() const { return reinterpret_cast<const T*>(storage_); }

    public:
        Option() : has_value_(false) {}
        Option(None_t) : has_value_(false) {}  // Allow implicit conversion from None
        Option(T value) : has_value_(true) { new (storage_) T(std::move(value)); }
        Option(const Option& other) : has_value_(other.has_value_) {
            if (has_value_) new (storage_) T(*other.ptr());
        }
        Option(Option&& other) noexcept : has_value_(other.has_value_) {
            if (has_value_) { new (storage_) T(std::move(*other.ptr())); other.has_value_ = false; }
        }
        ~Option() { if (has_value_) ptr()->~T(); }

        Option& operator=(const Option& other) {
            if (this != &other) {
                if (has_value_) ptr()->~T();
                has_value_ = other.has_value_;
                if (has_value_) new (storage_) T(*other.ptr());
            }
            return *this;
        }
        Option& operator=(Option&& other) noexcept {
            if (this != &other) {
                if (has_value_) ptr()->~T();
                has_value_ = other.has_value_;
                if (has_value_) { new (storage_) T(std::move(*other.ptr())); other.has_value_ = false; }
            }
            return *this;
        }

        bool is_some() const { return has_value_; }
        bool is_none() const { return !has_value_; }
        explicit operator bool() const { return has_value_; }

        T& unwrap() {
            if (!has_value_) throw std::runtime_error("unwrap called on None");
            return *ptr();
        }
        const T& unwrap() const {
            if (!has_value_) throw std::runtime_error("unwrap called on None");
            return *ptr();
        }
        T unwrap_or(T default_val) const {
            return has_value_ ? *ptr() : default_val;
        }

        // For ? operator support
        bool __is_err() const { return !has_value_; }
        T __unwrap_ok() { return std::move(*ptr()); }
    };

    // Wrapper for Some to enable implicit conversion to Option<T>
    template <typename T>
    struct SomeWrapper {
        T value;
        explicit SomeWrapper(T v) : value(std::move(v)) {}
        operator Option<T>() const { return Option<T>(value); }
    };

    template <typename T>
    SomeWrapper<T> Some(T value) { return SomeWrapper<T>(std::move(value)); }

    template <typename T>
    Option<T> make_none() { return Option<T>(); }

    // Result<T, E> - represents either success (Ok) or failure (Err)
    template <typename T, typename E = std::string>
    class Result {
        bool is_ok_;
        union {
            T ok_value_;
            E err_value_;
        };

    public:
        Result(const Result& other) : is_ok_(other.is_ok_) {
            if (is_ok_) new (&ok_value_) T(other.ok_value_);
            else new (&err_value_) E(other.err_value_);
        }
        Result(Result&& other) noexcept : is_ok_(other.is_ok_) {
            if (is_ok_) new (&ok_value_) T(std::move(other.ok_value_));
            else new (&err_value_) E(std::move(other.err_value_));
        }
        ~Result() {
            if (is_ok_) ok_value_.~T();
            else err_value_.~E();
        }

        Result& operator=(const Result& other) {
            if (this != &other) {
                this->~Result();
                is_ok_ = other.is_ok_;
                if (is_ok_) new (&ok_value_) T(other.ok_value_);
                else new (&err_value_) E(other.err_value_);
            }
            return *this;
        }

        // Factory functions
        static Result Ok(T value) {
            Result r;
            r.is_ok_ = true;
            new (&r.ok_value_) T(std::move(value));
            return r;
        }
        static Result Err(E error) {
            Result r;
            r.is_ok_ = false;
            new (&r.err_value_) E(std::move(error));
            return r;
        }

        bool is_ok() const { return is_ok_; }
        bool is_err() const { return !is_ok_; }
        explicit operator bool() const { return is_ok_; }

        T& unwrap() {
            if (!is_ok_) throw std::runtime_error("unwrap called on Err");
            return ok_value_;
        }
        const T& unwrap() const {
            if (!is_ok_) throw std::runtime_error("unwrap called on Err");
            return ok_value_;
        }
        T unwrap_or(T default_val) const {
            return is_ok_ ? ok_value_ : default_val;
        }
        E& unwrap_err() {
            if (is_ok_) throw std::runtime_error("unwrap_err called on Ok");
            return err_value_;
        }

        // For ? operator support
        bool __is_err() const { return !is_ok_; }
        T __unwrap_ok() { return std::move(ok_value_); }
        E __unwrap_err() { return std::move(err_value_); }

    private:
        Result() {}
    };

    // Wrapper types for Ok and Err to enable implicit conversion to Result<T, E>
    template <typename T>
    struct OkWrapper {
        T value;
        explicit OkWrapper(T v) : value(std::move(v)) {}
        template <typename E>
        operator Result<T, E>() const { return Result<T, E>::Ok(value); }
    };

    template <typename E>
    struct ErrWrapper {
        E value;
        explicit ErrWrapper(E v) : value(std::move(v)) {}
        template <typename T>
        operator Result<T, E>() const { return Result<T, E>::Err(value); }
    };

    // Specialization for const char* to allow conversion to Result<T, std::string>
    template <>
    struct ErrWrapper<const char*> {
        const char* value;
        explicit ErrWrapper(const char* v) : value(v) {}
        template <typename T>
        operator Result<T, std::string>() const { return Result<T, std::string>::Err(std::string(value)); }
    };

    template <typename T>
    OkWrapper<T> Ok(T value) { return OkWrapper<T>(std::move(value)); }

    template <typename E>
    ErrWrapper<E> Err(E error) { return ErrWrapper<E>(std::move(error)); }

    inline void print(int32_t v) { std::printf("%d", v); }
    inline void print(int64_t v) { std::printf("%lld", (long long)v); }
    inline void print(size_t v) { std::printf("%zu", v); }
    inline void print(float v) { std::printf("%g", v); }
    inline void print(bool v) { std::printf("%s", v ? "true" : "false"); }
    inline void print(const char* v) { std::printf("%s", v); }
    inline void print(const std::string& v) { std::printf("%s", v.c_str()); }

    inline void println() { std::printf("\n"); }
    inline void println(int32_t v) { std::printf("%d\n", v); }
    inline void println(int64_t v) { std::printf("%lld\n", (long long)v); }
    inline void println(size_t v) { std::printf("%zu\n", v); }
    inline void println(float v) { std::printf("%g\n", v); }
    inline void println(bool v) { std::printf("%s\n", v ? "true" : "false"); }
    inline void println(const char* v) { std::printf("%s\n", v); }
    inline void println(const std::string& v) { std::printf("%s\n", v.c_str()); }

    // Range type for iteration
    template <typename T>
    struct Range {
        T start;
        T end_;
        bool inclusive;

        struct Iterator {
            T current;
            T end_;
            bool inclusive;

            Iterator(T c, T e, bool inc) : current(c), end_(e), inclusive(inc) {}

            T operator*() const { return current; }
            Iterator& operator++() { ++current; return *this; }
            bool operator!=(const Iterator& other) const {
                if (inclusive) return current <= other.end_;
                return current < other.end_;
            }
        };

        Iterator begin() const { return Iterator(start, end_, inclusive); }
        Iterator end() const { return Iterator(inclusive ? end_ + 1 : end_, end_, inclusive); }
    };

    // ========== Standard Library ==========

    // Vec<T> - Dynamic array type
    template <typename T>
    class Vec {
        std::vector<T> data_;
    public:
        Vec() = default;
        Vec(std::initializer_list<T> init) : data_(init) {}
        Vec(size_t count, const T& value) : data_(count, value) {}

        void push(T value) { data_.push_back(std::move(value)); }
        Option<T> pop() {
            if (data_.empty()) return Option<T>();
            T val = std::move(data_.back());
            data_.pop_back();
            return Option<T>(std::move(val));
        }

        T& operator[](size_t index) { return data_[index]; }
        const T& operator[](size_t index) const { return data_[index]; }

        T& at(size_t index) {
            if (index >= data_.size()) throw std::runtime_error("index out of bounds");
            return data_[index];
        }

        size_t len() const { return data_.size(); }
        bool is_empty() const { return data_.empty(); }
        void clear() { data_.clear(); }
        void reserve(size_t cap) { data_.reserve(cap); }

        T* begin() { return data_.data(); }
        T* end() { return data_.data() + data_.size(); }
        const T* begin() const { return data_.data(); }
        const T* end() const { return data_.data() + data_.size(); }
    };

    // String functions
    inline size_t len(const std::string& s) { return s.size(); }
    inline bool is_empty(const std::string& s) { return s.empty(); }

    inline std::string to_string(int32_t v) { return std::to_string(v); }
    inline std::string to_string(int64_t v) { return std::to_string(v); }
    inline std::string to_string(size_t v) { return std::to_string(v); }
    inline std::string to_string(float v) { return std::to_string(v); }
    inline std::string to_string(double v) { return std::to_string(v); }
    inline std::string to_string(bool v) { return v ? "true" : "false"; }
    inline std::string to_string(const std::string& v) { return v; }

    inline bool starts_with(const std::string& s, const std::string& prefix) {
        if (prefix.size() > s.size()) return false;
        return s.compare(0, prefix.size(), prefix) == 0;
    }

    inline bool ends_with(const std::string& s, const std::string& suffix) {
        if (suffix.size() > s.size()) return false;
        return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    inline bool contains(const std::string& s, const std::string& sub) {
        return s.find(sub) != std::string::npos;
    }

    inline std::string trim(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(s[start])) start++;
        size_t end = s.size();
        while (end > start && std::isspace(s[end - 1])) end--;
        return s.substr(start, end - start);
    }

    inline std::string substr(const std::string& s, size_t start, size_t len) {
        return s.substr(start, len);
    }

    inline std::string replace(const std::string& s, const std::string& old_str, const std::string& new_str) {
        std::string result = s;
        size_t pos = 0;
        while ((pos = result.find(old_str, pos)) != std::string::npos) {
            result.replace(pos, old_str.length(), new_str);
            pos += new_str.length();
        }
        return result;
    }

    inline std::string to_uppercase(const std::string& s) {
        std::string result = s;
        for (char& c : result) {
            c = std::toupper(static_cast<unsigned char>(c));
        }
        return result;
    }

    inline std::string to_lowercase(const std::string& s) {
        std::string result = s;
        for (char& c : result) {
            c = std::tolower(static_cast<unsigned char>(c));
        }
        return result;
    }

    inline Vec<std::string> split(const std::string& s, const std::string& delimiter) {
        Vec<std::string> result;
        if (delimiter.empty()) {
            result.push(s);
            return result;
        }
        size_t start = 0;
        size_t end = s.find(delimiter);
        while (end != std::string::npos) {
            result.push(s.substr(start, end - start));
            start = end + delimiter.length();
            end = s.find(delimiter, start);
        }
        result.push(s.substr(start));
        return result;
    }

    inline std::string join(const Vec<std::string>& vec, const std::string& delimiter) {
        std::string result;
        bool first = true;
        for (size_t i = 0; i < vec.len(); ++i) {
            if (!first) result += delimiter;
            result += vec[i];
            first = false;
        }
        return result;
    }

    inline std::string repeat(const std::string& s, size_t count) {
        std::string result;
        result.reserve(s.size() * count);
        for (size_t i = 0; i < count; ++i) {
            result += s;
        }
        return result;
    }

    inline std::string reverse(const std::string& s) {
        return std::string(s.rbegin(), s.rend());
    }

    // Math functions
    template <typename T>
    inline T abs(T x) { return x < 0 ? -x : x; }

    template <typename T>
    inline T min(T a, T b) { return a < b ? a : b; }

    template <typename T>
    inline T max(T a, T b) { return a > b ? a : b; }

    template <typename T>
    inline T clamp(T x, T lo, T hi) { return x < lo ? lo : (x > hi ? hi : x); }

    // I/O functions
    inline std::string read_line() {
        std::string line;
        std::getline(std::cin, line);
        return line;
    }

    inline Option<int32_t> parse_int(const std::string& s) {
        try {
            size_t pos;
            int32_t result = std::stoi(s, &pos);
            if (pos == s.size()) return Option<int32_t>(result);
            return Option<int32_t>();
        } catch (...) {
            return Option<int32_t>();
        }
    }

    inline Option<float> parse_float(const std::string& s) {
        try {
            size_t pos;
            float result = std::stof(s, &pos);
            if (pos == s.size()) return Option<float>(result);
            return Option<float>();
        } catch (...) {
            return Option<float>();
        }
    }

    // Assert function
    inline void assert_true(bool condition, const char* msg = "assertion failed") {
        if (!condition) throw std::runtime_error(msg);
    }
}
)";

static void print_usage() {
    std::cerr << "Mana Compiler v0.1.0\n\n";
    std::cerr << "Usage: mana <command> [options] [file]\n\n";
    std::cerr << "Commands:\n";
    std::cerr << "  build        Build the current project\n";
    std::cerr << "  run          Build and run the project\n";
    std::cerr << "  test         Run tests\n";
    std::cerr << "  new <name>   Create a new project\n";
    std::cerr << "  fmt          Format source files\n";
    std::cerr << "  repl         Start interactive REPL\n";
    std::cerr << "  <file>       Compile a single file\n\n";
    std::cerr << "Options:\n";
    std::cerr << "  -o <file>    Output executable name\n";
    std::cerr << "  -c           Compile only (generate .cpp, don't link)\n";
    std::cerr << "  --emit-cpp   Print generated C++ to stdout\n";
    std::cerr << "  --ast        Print AST to stdout\n";
    std::cerr << "  --doc        Generate Markdown documentation\n";
    std::cerr << "  --no-cache   Disable incremental compilation\n";
    std::cerr << "  --clear-cache  Clear compilation cache\n";
    std::cerr << "  -h, --help   Show this help\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string first_arg = argv[1];

    // Handle subcommands
    if (first_arg == "repl") {
        mana::repl::Repl repl;
        repl.run();
        return 0;
    }

    if (first_arg == "new" && argc >= 3) {
        mana::pkg::PackageManager pkg;
        return pkg.init(argv[2]);
    }

    if (first_arg == "build") {
        mana::pkg::PackageManager pkg;
        return pkg.build();
    }

    if (first_arg == "run") {
        mana::pkg::PackageManager pkg;
        return pkg.run();
    }

    if (first_arg == "test") {
        mana::pkg::PackageManager pkg;
        return pkg.test();
    }

    if (first_arg == "fmt") {
        // Format files
        if (argc < 3) {
            std::cerr << "Usage: mana fmt <file.mana>\n";
            return 1;
        }
        std::ifstream in(argv[2]);
        if (!in) {
            std::cerr << "Cannot open: " << argv[2] << "\n";
            return 1;
        }
        std::stringstream buffer;
        buffer << in.rdbuf();

        Lexer lex(buffer.str());
        auto tokens = lex.tokenize();
        DiagnosticEngine diag;
        Parser parser(tokens, diag);
        auto module = parser.parse_module();

        if (diag.has_errors()) {
            for (const auto& e : diag.errors()) {
                std::cerr << e.message << "\n";
            }
            return 1;
        }

        mana::fmt::Formatter formatter;
        std::cout << formatter.format(module.get());
        return 0;
    }

    if (first_arg == "add" && argc >= 3) {
        mana::pkg::PackageManager pkg;
        return pkg.add(argv[2]);
    }

    if (first_arg == "remove" && argc >= 3) {
        mana::pkg::PackageManager pkg;
        return pkg.remove(argv[2]);
    }

    std::string input_file;
    std::string output_file;
    bool compile_only = false;
    bool emit_cpp = false;
    bool print_ast = false;
    bool gen_doc = false;
    bool use_cache = true;
    bool clear_cache = false;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg == "-c") {
            compile_only = true;
        } else if (arg == "--emit-cpp") {
            emit_cpp = true;
        } else if (arg == "--ast") {
            print_ast = true;
        } else if (arg == "--doc") {
            gen_doc = true;
        } else if (arg == "--no-cache") {
            use_cache = false;
        } else if (arg == "--clear-cache") {
            clear_cache = true;
        } else if (arg == "-h" || arg == "--help") {
            print_usage();
            return 0;
        } else if (arg[0] != '-') {
            input_file = arg;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        }
    }

    // Handle clear-cache without requiring a file
    if (clear_cache && input_file.empty()) {
        CompilationCache cache;
        fs::path cache_dir = fs::temp_directory_path() / "mana_cache";
        cache.set_cache_dir(cache_dir);
        cache.clear();
        std::cout << "Cleared compilation cache\n";
        return 0;
    }

    if (input_file.empty()) {
        std::cerr << "error: no input file\n";
        return 1;
    }

    // Read source file
    std::ifstream in(input_file);
    if (!in) {
        std::cerr << "error: cannot open file: " << input_file << "\n";
        return 1;
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    // Setup compilation cache
    CompilationCache cache;
    fs::path cache_dir = fs::temp_directory_path() / "mana_cache";
    cache.set_cache_dir(cache_dir);

    if (clear_cache) {
        cache.clear();
        std::cout << "Cleared compilation cache\n";
        if (input_file.empty()) return 0;
    }

    // Setup diagnostics
    DiagnosticEngine diag;
    diag.set_source(input_file, source);

    // Lexing
    Lexer lex(source);
    auto tokens = lex.tokenize();

    // Parsing
    Parser parser(tokens, diag);
    auto module = parser.parse_module();
    if (!module || diag.has_errors()) {
        diag.print_all(std::cerr);
        return 1;
    }

    // Resolve imports
    fs::path input_path(input_file);
    std::unordered_set<std::string> imported_files;
    imported_files.insert(fs::weakly_canonical(input_path).string());
    if (!resolve_imports(module.get(), input_path.parent_path(), diag, imported_files)) {
        diag.print_all(std::cerr);
        return 1;
    }

    // Semantic analysis
    SemanticAnalyzer sema(diag);
    sema.analyze(module.get());
    if (diag.has_errors()) {
        diag.print_all(std::cerr);
        return 1;
    }

    // Print warnings even when compilation succeeds
    if (diag.has_any()) {
        diag.print_all(std::cerr);
    }

    // Run middle-end optimization passes
    mana::middle::ForLowering::run(module.get());
    mana::middle::DeadCodeElimination::run(module.get());
    mana::middle::Inlining::run(module.get());

    // Generate documentation if requested
    if (gen_doc) {
        DocGenerator doc_gen;
        std::string markdown = doc_gen.generate(*module);

        // Output to file
        fs::path doc_file = input_path.parent_path() / (input_path.stem().string() + ".md");
        std::ofstream doc_out(doc_file);
        if (!doc_out) {
            std::cerr << "error: cannot write documentation file: " << doc_file << "\n";
            return 1;
        }
        doc_out << markdown;
        std::cout << "Generated documentation: " << doc_file.string() << "\n";
        return 0;
    }

    // Print AST if requested
    if (print_ast) {
        AstPrinter printer;
        printer.print(module.get(), std::cout);
        std::cout << "\n";
    }

    // Check cache for incremental compilation
    std::string cpp_code;
    bool cache_hit = false;
    if (use_cache) {
        auto cached = cache.get_cached_cpp(input_file);
        if (cached && cache.is_cached(input_file, source)) {
            cpp_code = *cached;
            cache_hit = true;
            std::cout << "Using cached output for " << input_file << "\n";
        }
    }

    // Generate C++ code if not cached
    if (!cache_hit) {
        std::ostringstream cpp_stream;
        CppEmitter emit;
        emit.emit(module.get(), cpp_stream);
        cpp_code = cpp_stream.str();

        // Store in cache
        if (use_cache) {
            cache.store(input_file, source, cpp_code);
        }
    }

    // Emit C++ to stdout if requested
    if (emit_cpp) {
        // (header already included by emitter)
        std::cout << cpp_code;
        return 0;
    }

    // Determine output paths
    fs::path base_name = input_path.stem();
    fs::path output_dir = input_path.parent_path();
    if (output_dir.empty()) output_dir = ".";

    fs::path cpp_file = output_dir / (base_name.string() + ".cpp");
    fs::path runtime_file = output_dir / "mana_runtime.h";

#ifdef _WIN32
    fs::path exe_file = output_file.empty()
        ? output_dir / (base_name.string() + ".exe")
        : fs::path(output_file);
#else
    fs::path exe_file = output_file.empty()
        ? output_dir / base_name
        : fs::path(output_file);
#endif

    // Write runtime header
    {
        std::ofstream runtime_out(runtime_file);
        if (!runtime_out) {
            std::cerr << "error: cannot write runtime header: " << runtime_file << "\n";
            return 1;
        }
        runtime_out << MANA_RUNTIME_H;
    }

    // Write generated C++
    {
        std::ofstream cpp_out(cpp_file);
        if (!cpp_out) {
            std::cerr << "error: cannot write C++ file: " << cpp_file << "\n";
            return 1;
        }
        cpp_out << "// Generated by mana-compiler from " << input_file << "\n";
        cpp_out << cpp_code;
    }

    std::cout << "Generated: " << cpp_file.string() << "\n";

    if (compile_only) {
        return 0;
    }

    // Create CMakeLists.txt for building
    fs::path cmake_file = output_dir / "CMakeLists.txt";
    {
        std::ofstream cmake_out(cmake_file);
        if (!cmake_out) {
            std::cerr << "error: cannot write CMakeLists.txt\n";
            return 1;
        }
        cmake_out << "cmake_minimum_required(VERSION 3.16)\n";
        cmake_out << "project(" << base_name.string() << ")\n";
        cmake_out << "set(CMAKE_CXX_STANDARD 20)\n";
        cmake_out << "add_executable(" << base_name.string() << " " << base_name.string() << ".cpp)\n";
    }

    // Build with cmake
    fs::path build_dir = output_dir / "build";
    std::string cmake_config = "cmake -B \"" + build_dir.string() + "\" -S \"" + output_dir.string() + "\" >nul 2>&1";
    std::string cmake_build = "cmake --build \"" + build_dir.string() + "\" --config Release >nul 2>&1";

    std::cout << "Compiling...\n";
    int result = std::system(cmake_config.c_str());
    if (result != 0) {
        std::cerr << "error: cmake configuration failed\n";
        return 1;
    }

    result = std::system(cmake_build.c_str());
    if (result != 0) {
        std::cerr << "error: compilation failed\n";
        return 1;
    }

#ifdef _WIN32
    fs::path built_exe = build_dir / "Release" / (base_name.string() + ".exe");
#else
    fs::path built_exe = build_dir / base_name;
#endif

    // Copy to final location if -o was specified
    if (!output_file.empty()) {
        try {
            fs::copy_file(built_exe, exe_file, fs::copy_options::overwrite_existing);
            std::cout << "Success: " << exe_file.string() << "\n";
        } catch (const fs::filesystem_error& e) {
            std::cerr << "error: cannot copy executable: " << e.what() << "\n";
            return 1;
        }
    } else {
        std::cout << "Success: " << built_exe.string() << "\n";
    }

    return 0;
}
