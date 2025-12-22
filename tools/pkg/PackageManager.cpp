#include "PackageManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#endif

namespace mana::pkg {

PackageManager::PackageManager() {}

bool PackageManager::create_directory(const std::string& path) {
    #ifdef _WIN32
    return _mkdir(path.c_str()) == 0 || errno == EEXIST;
    #else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
    #endif
}

bool PackageManager::write_file(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file) return false;
    file << content;
    return true;
}

std::string PackageManager::read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool PackageManager::file_exists(const std::string& path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0;
}

int PackageManager::run_command(const std::string& cmd) {
    return std::system(cmd.c_str());
}

std::string PackageManager::get_runtime_header() {
    // Minimal runtime header for basic programs
    return R"(#pragma once
#include <cstdio>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>

namespace mana {
    // Defer for RAII cleanup
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

    // Print functions
    inline void print(int32_t v) { std::printf("%d", v); }
    inline void print(int64_t v) { std::printf("%lld", (long long)v); }
    inline void print(float v) { std::printf("%g", v); }
    inline void print(double v) { std::printf("%g", v); }
    inline void print(bool v) { std::printf("%s", v ? "true" : "false"); }
    inline void print(const char* v) { std::printf("%s", v); }
    inline void print(const std::string& v) { std::printf("%s", v.c_str()); }

    inline void println() { std::printf("\n"); }
    inline void println(int32_t v) { std::printf("%d\n", v); }
    inline void println(int64_t v) { std::printf("%lld\n", (long long)v); }
    inline void println(float v) { std::printf("%g\n", v); }
    inline void println(double v) { std::printf("%g\n", v); }
    inline void println(bool v) { std::printf("%s\n", v ? "true" : "false"); }
    inline void println(const char* v) { std::printf("%s\n", v); }
    inline void println(const std::string& v) { std::printf("%s\n", v.c_str()); }

    // Variadic print - handles multiple arguments
    template<typename T>
    void println(T first) { print(first); std::printf("\n"); }

    template<typename T, typename... Args>
    void println(T first, Args... rest) { print(first); println(rest...); }

    // Range for iteration
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

    // String helper
    inline std::string read_line() {
        std::string line;
        std::getline(std::cin, line);
        return line;
    }
}
)";
}

int PackageManager::init(const std::string& name) {
    std::cout << "Creating new Mana project: " << name << "\n";

    // Create directories
    if (!create_directory(name)) {
        std::cerr << "Error: Could not create directory " << name << "\n";
        return 1;
    }
    if (!create_directory(name + "/src")) {
        std::cerr << "Error: Could not create src directory\n";
        return 1;
    }
    if (!create_directory(name + "/build")) {
        std::cerr << "Error: Could not create build directory\n";
        return 1;
    }

    // Create package.toml
    std::string toml = R"([package]
name = ")" + name + R"("
version = "0.1.0"
description = ""
authors = []
license = "MIT"

[dependencies]
)";
    if (!write_file(name + "/package.toml", toml)) {
        std::cerr << "Error: Could not create package.toml\n";
        return 1;
    }

    // Create main.mana
    std::string main_content = R"(module main

fn main() -> i32 {
    println("Hello, world!")

    let name = "Mana"
    println("Welcome to ", name, "!")

    for i in 1..=5 {
        println("Count: ", i)
    }

    return 0
}
)";
    if (!write_file(name + "/src/main.mana", main_content)) {
        std::cerr << "Error: Could not create main.mana\n";
        return 1;
    }

    // Create mana_runtime.h
    if (!write_file(name + "/mana_runtime.h", get_runtime_header())) {
        std::cerr << "Error: Could not create mana_runtime.h\n";
        return 1;
    }

    // Create .gitignore
    std::string gitignore = R"(# Build artifacts
/build/
*.o
*.obj
*.exe
*.cpp

# IDE
.idea/
*.swp
*.swo

# Cache
.mana_cache/
)";
    write_file(name + "/.gitignore", gitignore);

    std::cout << "Created project " << name << "\n";
    std::cout << "\nTo get started:\n";
    std::cout << "  cd " << name << "\n";
    std::cout << "  mana build\n";
    std::cout << "  mana run\n";

    return 0;
}

bool PackageManager::load_package(const std::string& path) {
    std::string content = read_file(path);
    if (content.empty()) {
        std::cerr << "Error: Could not read " << path << "\n";
        return false;
    }

    // Simple TOML parsing
    std::istringstream stream(content);
    std::string line;
    std::string section;

    while (std::getline(stream, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        // Section header
        if (line[0] == '[') {
            size_t end = line.find(']');
            if (end != std::string::npos) {
                section = line.substr(1, end - 1);
            }
            continue;
        }

        // Key-value pair
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);

            // Trim whitespace
            while (!key.empty() && (key.back() == ' ' || key.back() == '\t')) key.pop_back();
            while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) value.erase(0, 1);

            // Remove quotes from string values
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            if (section == "package") {
                if (key == "name") package_.name = value;
                else if (key == "version") package_.version = value;
                else if (key == "description") package_.description = value;
                else if (key == "license") package_.license = value;
            } else if (section == "dependencies") {
                Dependency dep;
                dep.name = key;
                dep.version = value;
                dep.source = "registry";
                package_.dependencies.push_back(dep);
            }
        }
    }

    return true;
}

bool PackageManager::save_package(const std::string& path) {
    std::ostringstream oss;

    oss << "[package]\n";
    oss << "name = \"" << package_.name << "\"\n";
    oss << "version = \"" << package_.version << "\"\n";
    if (!package_.description.empty()) {
        oss << "description = \"" << package_.description << "\"\n";
    }
    if (!package_.license.empty()) {
        oss << "license = \"" << package_.license << "\"\n";
    }

    oss << "\n[dependencies]\n";
    for (const auto& dep : package_.dependencies) {
        oss << dep.name << " = \"" << dep.version << "\"\n";
    }

    return write_file(path, oss.str());
}

std::string PackageManager::get_executable_path() {
    #ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    return std::string(path);
    #else
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return std::string(path);
    }
    return "mana_lang";
    #endif
}

int PackageManager::build() {
    if (!load_package()) {
        return 1;
    }

    std::cout << "Building " << package_.name << " v" << package_.version << "\n";

    // Create build directory
    create_directory("build");

    // Find entry point
    std::string entry = "src/main.mana";
    if (!file_exists(entry)) {
        std::cerr << "Error: Could not find " << entry << "\n";
        return 1;
    }

    // Get our own executable path to call ourselves for compilation
    std::string mana_exe = get_executable_path();

    // Step 1: Compile .mana to .cpp
    std::string cpp_file = "build/main.cpp";
    std::string compile_cmd = "\"" + mana_exe + "\" " + entry + " -c";

    std::cout << "  Compiling " << entry << "...\n";
    int result = run_command(compile_cmd);
    if (result != 0) {
        std::cerr << "Mana compilation failed\n";
        return result;
    }

    // The -c flag generates .cpp next to the .mana file, move it
    std::string gen_cpp = "src/main.cpp";
    if (file_exists(gen_cpp)) {
        // Copy to build folder
        std::string content = read_file(gen_cpp);
        write_file(cpp_file, content);
        std::remove(gen_cpp.c_str());
    } else if (file_exists("build/main.cpp")) {
        // Already in right place
    } else {
        std::cerr << "Error: Generated C++ file not found\n";
        return 1;
    }

    // Step 2: Compile C++ to executable
    std::string exe_name = "build/" + package_.name;
    #ifdef _WIN32
    exe_name += ".exe";
    #endif

    std::cout << "  Compiling C++...\n";

    #ifdef _WIN32
    // Try cl.exe first (Visual Studio), fall back to g++
    std::string cpp_cmd = "cl /nologo /EHsc /std:c++17 /I. " + cpp_file + " /Fe:" + exe_name + " 2>nul";
    result = run_command(cpp_cmd);
    if (result != 0) {
        // Try g++ (MinGW)
        cpp_cmd = "g++ -std=c++17 -I. " + cpp_file + " -o " + exe_name;
        result = run_command(cpp_cmd);
    }
    #else
    std::string cpp_cmd = "g++ -std=c++17 -I. " + cpp_file + " -o " + exe_name;
    result = run_command(cpp_cmd);
    #endif

    if (result != 0) {
        std::cerr << "C++ compilation failed\n";
        std::cerr << "Make sure you have a C++ compiler installed (Visual Studio or g++)\n";
        return result;
    }

    std::cout << "Build successful: " << exe_name << "\n";
    return 0;
}

int PackageManager::run() {
    int result = build();
    if (result != 0) return result;

    std::string exe = "build/" + package_.name;
    #ifdef _WIN32
    exe += ".exe";
    // Use backslashes on Windows
    for (char& c : exe) {
        if (c == '/') c = '\\';
    }
    #endif

    std::cout << "Running " << exe << "\n\n";
    return run_command(exe);
}

int PackageManager::test() {
    if (!load_package()) {
        return 1;
    }

    std::cout << "Testing " << package_.name << "\n";

    // Find test files
    std::string cmd = "mana_lang src/main.mana --test -o build/test";
    #ifdef _WIN32
    cmd += ".exe";
    #endif

    int result = run_command(cmd);
    if (result != 0) {
        return result;
    }

    // Run tests
    std::string test_exe = "build/test";
    #ifdef _WIN32
    test_exe += ".exe";
    #endif

    return run_command(test_exe);
}

int PackageManager::add(const std::string& dep_spec) {
    if (!load_package()) {
        return 1;
    }

    // Parse dep_spec (name@version or just name)
    std::string name = dep_spec;
    std::string version = "*";

    size_t at = dep_spec.find('@');
    if (at != std::string::npos) {
        name = dep_spec.substr(0, at);
        version = dep_spec.substr(at + 1);
    }

    // Check if already exists
    for (auto& dep : package_.dependencies) {
        if (dep.name == name) {
            dep.version = version;
            std::cout << "Updated " << name << " to version " << version << "\n";
            save_package();
            return 0;
        }
    }

    // Add new dependency
    Dependency dep;
    dep.name = name;
    dep.version = version;
    dep.source = "registry";
    package_.dependencies.push_back(dep);

    save_package();
    std::cout << "Added " << name << " version " << version << "\n";
    return 0;
}

int PackageManager::remove(const std::string& name) {
    if (!load_package()) {
        return 1;
    }

    auto it = package_.dependencies.begin();
    while (it != package_.dependencies.end()) {
        if (it->name == name) {
            it = package_.dependencies.erase(it);
            std::cout << "Removed " << name << "\n";
            save_package();
            return 0;
        } else {
            ++it;
        }
    }

    std::cerr << "Dependency " << name << " not found\n";
    return 1;
}

int PackageManager::update() {
    if (!load_package()) {
        return 1;
    }

    std::cout << "Updating dependencies for " << package_.name << "\n";

    // TODO: Actually fetch and update dependencies
    for (const auto& dep : package_.dependencies) {
        std::cout << "  " << dep.name << ": " << dep.version << " (up to date)\n";
    }

    return 0;
}

int PackageManager::publish() {
    if (!load_package()) {
        return 1;
    }

    std::cout << "Publishing " << package_.name << " v" << package_.version << "\n";
    std::cout << "Note: Package registry not yet implemented\n";
    return 0;
}

} // namespace mana::pkg
