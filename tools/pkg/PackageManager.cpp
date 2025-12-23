#include "PackageManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <algorithm>
#include <regex>
#include <iomanip>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#define mkdir(path, mode) _mkdir(path)
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
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

std::string PackageManager::get_graphics_header() {
    // Minimal graphics header - users should copy full version for advanced features
    return R"(#pragma once
// Mana Graphics Runtime - GLFW/OpenGL bindings
// For full features, copy mana_graphics.h from mana-lang/examples/graphics/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <cmath>

// Window management
inline int mana_glfwInit() { return glfwInit(); }
inline void mana_glfwTerminate() { glfwTerminate(); }

inline int64_t mana_glfwCreateWindow(int w, int h, const std::string& title, int64_t m, int64_t s) {
    return reinterpret_cast<int64_t>(glfwCreateWindow(w, h, title.c_str(),
        reinterpret_cast<GLFWmonitor*>(m), reinterpret_cast<GLFWwindow*>(s)));
}

inline void mana_glfwDestroyWindow(int64_t w) { glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(w)); }
inline void mana_glfwMakeContextCurrent(int64_t w) { glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(w)); }
inline int mana_glfwWindowShouldClose(int64_t w) { return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(w)); }
inline void mana_glfwSwapBuffers(int64_t w) { glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(w)); }
inline void mana_glfwPollEvents() { glfwPollEvents(); }
inline void mana_glfwSwapInterval(int i) { glfwSwapInterval(i); }
inline double mana_glfwGetTime() { return glfwGetTime(); }
inline int mana_gladLoadGL() { return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); }

// OpenGL
inline void mana_glClearColor(float r, float g, float b, float a) { glClearColor(r, g, b, a); }
inline void mana_glClear(int mask) { glClear(mask); }
inline void mana_glViewport(int x, int y, int w, int h) { glViewport(x, y, w, h); }
inline int32_t mana_GL_COLOR_BUFFER_BIT() { return GL_COLOR_BUFFER_BIT; }

// Input
inline int32_t mana_glfwGetKey(int64_t w, int32_t key) { return glfwGetKey(reinterpret_cast<GLFWwindow*>(w), key); }
inline int32_t mana_GLFW_KEY_ESCAPE() { return GLFW_KEY_ESCAPE; }
inline int32_t mana_GLFW_PRESS() { return GLFW_PRESS; }
)";
}

std::string PackageManager::get_graphics_mana() {
    return R"(module graphics

// GLFW window functions
pub extern fn mana_glfwInit() -> i32
pub extern fn mana_glfwTerminate() -> void
pub extern fn mana_glfwCreateWindow(width: i32, height: i32, title: string, monitor: i64, share: i64) -> i64
pub extern fn mana_glfwDestroyWindow(window: i64) -> void
pub extern fn mana_glfwMakeContextCurrent(window: i64) -> void
pub extern fn mana_glfwWindowShouldClose(window: i64) -> i32
pub extern fn mana_glfwSwapBuffers(window: i64) -> void
pub extern fn mana_glfwPollEvents() -> void
pub extern fn mana_glfwSwapInterval(interval: i32) -> void
pub extern fn mana_glfwGetTime() -> f64
pub extern fn mana_gladLoadGL() -> i32

// OpenGL functions
pub extern fn mana_glClearColor(r: f32, g: f32, b: f32, a: f32) -> void
pub extern fn mana_glClear(mask: i32) -> void
pub extern fn mana_glViewport(x: i32, y: i32, width: i32, height: i32) -> void
pub extern fn mana_GL_COLOR_BUFFER_BIT() -> i32

// Input
pub extern fn mana_glfwGetKey(window: i64, key: i32) -> i32
pub extern fn mana_GLFW_KEY_ESCAPE() -> i32
pub extern fn mana_GLFW_PRESS() -> i32

// High-level API
pub fn create_window(title: string, width: i32, height: i32) -> i64 {
    if mana_glfwInit() == 0 {
        println("Failed to initialize GLFW")
        return 0
    }
    let window: i64 = mana_glfwCreateWindow(width, height, title, 0, 0)
    if window == 0 {
        println("Failed to create window")
        mana_glfwTerminate()
        return 0
    }
    mana_glfwMakeContextCurrent(window)
    if mana_gladLoadGL() == 0 {
        println("Failed to load OpenGL")
        mana_glfwDestroyWindow(window)
        mana_glfwTerminate()
        return 0
    }
    mana_glfwSwapInterval(1)
    mana_glViewport(0, 0, width, height)
    return window
}

pub fn window_open(window: i64) -> bool {
    return mana_glfwWindowShouldClose(window) == 0
}

pub fn clear(r: f32, g: f32, b: f32) -> void {
    mana_glClearColor(r, g, b, 1.0)
    mana_glClear(mana_GL_COLOR_BUFFER_BIT())
}

pub fn present(window: i64) -> void {
    mana_glfwSwapBuffers(window)
    mana_glfwPollEvents()
}

pub fn close_window(window: i64) -> void {
    mana_glfwDestroyWindow(window)
    mana_glfwTerminate()
}

pub fn get_time() -> f64 {
    return mana_glfwGetTime()
}

pub fn key_pressed(window: i64, key: i32) -> bool {
    return mana_glfwGetKey(window, key) == mana_GLFW_PRESS()
}
)";
}

int PackageManager::init(const std::string& name, bool graphics) {
    std::cout << "Creating new Mana project: " << name;
    if (graphics) std::cout << " (with graphics)";
    std::cout << "\n";

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

    // Create main.mana (different for graphics vs console)
    std::string main_content;
    if (graphics) {
        main_content = R"(module main

import "graphics"

fn main() -> i32 {
    let window = create_window(")" + name + R"(", 800, 600)
    if window == 0 { return 1 }

    let time: f64 = 0.0

    while window_open(window) {
        // ESC to quit
        if key_pressed(window, mana_GLFW_KEY_ESCAPE()) {
            break
        }

        // Animate background color
        time = get_time()
        let r: f32 = 0.3 + 0.2 * sin(time)
        let g: f32 = 0.4 + 0.2 * sin(time * 1.3)
        let b: f32 = 0.5 + 0.2 * sin(time * 0.7)

        clear(r, g, b)
        present(window)
    }

    close_window(window)
    return 0
}

extern fn sin(x: f64) -> f32
)";
    } else {
        main_content = R"(module main

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
    }
    if (!write_file(name + "/src/main.mana", main_content)) {
        std::cerr << "Error: Could not create main.mana\n";
        return 1;
    }

    // Graphics-specific files
    if (graphics) {
        // Create graphics.mana
        if (!write_file(name + "/src/graphics.mana", get_graphics_mana())) {
            std::cerr << "Error: Could not create graphics.mana\n";
            return 1;
        }

        // Create CMakeLists.txt for graphics
        std::string cmake = R"(cmake_minimum_required(VERSION 3.16)
project()" + name + R"()

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

file(GLOB SOURCES "build/*.cpp")

add_executable()" + name + R"( ${SOURCES})
target_include_directories()" + name + R"( PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/build)
target_link_libraries()" + name + R"( PRIVATE glfw glad::glad)

if(WIN32)
    target_link_libraries()" + name + R"( PRIVATE opengl32)
endif()
)";
        write_file(name + "/CMakeLists.txt", cmake);
    }

    // Create .gitignore
    std::string gitignore = R"(/build/
.mana_cache/
)";
    write_file(name + "/.gitignore", gitignore);

    std::cout << "Created project " << name << "\n";
    std::cout << "\nTo get started:\n";
    std::cout << "  cd " << name << "\n";
    if (graphics) {
        std::cout << "\n  # First time setup:\n";
        std::cout << "  mana src/graphics.mana -c\n";
        std::cout << "  mana src/main.mana -c\n";
        std::cout << "  cmake -B cmake-build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake\n";
        std::cout << "  cmake --build cmake-build --config Release\n";
        std::cout << "  ./cmake-build/Release/" << name << ".exe\n";
        std::cout << "\n  # Requires: vcpkg install glfw3 glad\n";
    } else {
        std::cout << "  mana build\n";
        std::cout << "  mana run\n";
    }

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
    DWORD len = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (len > 0 && len < MAX_PATH) {
        return std::string(path);
    }
    // Fallback: try to find mana in PATH or current directory
    return "mana";
    #else
    char path[1024];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len != -1) {
        path[len] = '\0';
        return std::string(path);
    }
    return "mana";
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

    // Debug: show the command if MANA_DEBUG is set
    if (getenv("MANA_DEBUG")) {
        std::cout << "  [debug] Executable: " << mana_exe << "\n";
        std::cout << "  [debug] Command: " << compile_cmd << "\n";
    }

    int result = run_command(compile_cmd);
    if (result != 0) {
        std::cerr << "Mana compilation failed\n";
        std::cerr << "  Command was: " << compile_cmd << "\n";
        return result;
    }

    // The -c flag generates files next to the .mana file, move them to build/
    std::string gen_cpp = "src/main.cpp";
    std::string gen_runtime = "src/mana_runtime.h";

    if (file_exists(gen_cpp)) {
        std::string content = read_file(gen_cpp);
        write_file(cpp_file, content);
        std::remove(gen_cpp.c_str());
    } else if (!file_exists("build/main.cpp")) {
        std::cerr << "Error: Generated C++ file not found\n";
        return 1;
    }

    // Move runtime header to build/ and clean up from src/
    if (file_exists(gen_runtime)) {
        std::string content = read_file(gen_runtime);
        write_file("build/mana_runtime.h", content);
        std::remove(gen_runtime.c_str());
    }

    // Step 2: Compile C++ to executable
    std::string exe_name = "build/" + package_.name;
    #ifdef _WIN32
    exe_name += ".exe";
    #endif

    std::cout << "  Compiling C++...\n";

    #ifdef _WIN32
    // Try cl.exe first (Visual Studio), fall back to g++
    std::string cpp_cmd = "cl /nologo /EHsc /std:c++17 /Ibuild " + cpp_file + " /Fe:" + exe_name + " 2>nul";
    result = run_command(cpp_cmd);
    if (result != 0) {
        // Try g++ (MinGW)
        cpp_cmd = "g++ -std=c++17 -Ibuild " + cpp_file + " -o " + exe_name;
        result = run_command(cpp_cmd);
    }
    #else
    std::string cpp_cmd = "g++ -std=c++17 -Ibuild " + cpp_file + " -o " + exe_name;
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

    // Get our executable path
    std::string mana_exe = get_executable_path();

    // Find test files
    std::string test_exe = "build/test";
    #ifdef _WIN32
    test_exe += ".exe";
    #endif

    std::string cmd = "\"" + mana_exe + "\" src/main.mana --test -o " + test_exe;

    int result = run_command(cmd);
    if (result != 0) {
        return result;
    }

    // Run tests
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

    if (!load_auth()) {
        std::cerr << "Error: Not logged in. Run 'mana login <token>' first.\n";
        return 1;
    }

    std::cout << "Publishing " << package_.name << " v" << package_.version << "...\n";

    // Validate package
    if (package_.name.empty()) {
        std::cerr << "Error: Package name is required\n";
        return 1;
    }
    if (package_.version.empty()) {
        std::cerr << "Error: Package version is required\n";
        return 1;
    }

    // Check entry point exists
    if (!file_exists(package_.entry_point)) {
        std::cerr << "Error: Entry point not found: " << package_.entry_point << "\n";
        return 1;
    }

    if (upload_package(package_)) {
        std::cout << "Successfully published " << package_.name << " v" << package_.version << "\n";
        return 0;
    } else {
        std::cerr << "Failed to publish package\n";
        return 1;
    }
}

// ============================================================================
// Registry Commands
// ============================================================================

int PackageManager::search(const std::string& query) {
    std::cout << "Searching for '" << query << "'...\n\n";

    auto results = search_registry(query);

    if (results.empty()) {
        std::cout << "No packages found.\n";
        return 0;
    }

    // Print results table
    std::cout << std::left << std::setw(30) << "NAME"
              << std::setw(15) << "VERSION"
              << std::setw(10) << "DOWNLOADS"
              << "DESCRIPTION\n";
    std::cout << std::string(80, '-') << "\n";

    for (const auto& pkg : results) {
        std::cout << std::left << std::setw(30) << pkg.name
                  << std::setw(15) << pkg.latest_version
                  << std::setw(10) << pkg.downloads;

        // Truncate description
        std::string desc = pkg.description;
        if (desc.length() > 40) desc = desc.substr(0, 37) + "...";
        std::cout << desc << "\n";
    }

    std::cout << "\n" << results.size() << " packages found.\n";
    return 0;
}

int PackageManager::info(const std::string& package_name) {
    auto pkg_opt = fetch_package_info(package_name);

    if (!pkg_opt.has_value()) {
        std::cerr << "Package '" << package_name << "' not found.\n";
        return 1;
    }

    auto& pkg = pkg_opt.value();

    std::cout << "\n";
    std::cout << "  " << pkg.name << " v" << pkg.latest_version << "\n";
    std::cout << "  " << pkg.description << "\n";
    std::cout << "\n";

    if (!pkg.authors.empty()) {
        std::cout << "  Authors:\n";
        for (const auto& author : pkg.authors) {
            std::cout << "    - " << author << "\n";
        }
    }

    if (!pkg.license.empty()) {
        std::cout << "  License: " << pkg.license << "\n";
    }

    if (!pkg.repository.empty()) {
        std::cout << "  Repository: " << pkg.repository << "\n";
    }

    if (!pkg.keywords.empty()) {
        std::cout << "  Keywords: ";
        for (size_t i = 0; i < pkg.keywords.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << pkg.keywords[i];
        }
        std::cout << "\n";
    }

    std::cout << "  Downloads: " << pkg.downloads << "\n";

    if (!pkg.versions.empty()) {
        std::cout << "\n  Available versions:\n";
        for (const auto& v : pkg.versions) {
            std::cout << "    - " << v;
            if (v == pkg.latest_version) std::cout << " (latest)";
            std::cout << "\n";
        }
    }

    std::cout << "\n  Install with: mana add " << pkg.name << "\n\n";
    return 0;
}

int PackageManager::install() {
    if (!load_package()) {
        return 1;
    }

    std::cout << "Installing dependencies for " << package_.name << "...\n";

    if (package_.dependencies.empty()) {
        std::cout << "No dependencies to install.\n";
        return 0;
    }

    // Create cache directory
    std::string cache_dir = get_cache_dir();
    create_directory(cache_dir);

    // Resolve all dependencies
    auto resolved = resolve_dependencies();

    if (resolved.empty() && !package_.dependencies.empty()) {
        std::cerr << "Failed to resolve dependencies.\n";
        return 1;
    }

    int installed = 0;
    for (const auto& dep : resolved) {
        if (is_package_cached(dep.name, dep.version)) {
            std::cout << "  " << dep.name << "@" << dep.version << " (cached)\n";
        } else {
            std::cout << "  Downloading " << dep.name << "@" << dep.version << "...\n";
            if (download_package(dep.name, dep.version)) {
                installed++;
            } else {
                std::cerr << "  Failed to download " << dep.name << "\n";
            }
        }
    }

    std::cout << "\nInstalled " << installed << " new packages, "
              << (resolved.size() - installed) << " from cache.\n";
    return 0;
}

int PackageManager::login(const std::string& token) {
    auth_token_ = token;
    if (save_auth()) {
        std::cout << "Logged in successfully.\n";
        return 0;
    }
    std::cerr << "Failed to save authentication.\n";
    return 1;
}

int PackageManager::logout() {
    auth_token_.clear();
    std::string config_dir = get_cache_dir() + "/../config";
    std::string auth_file = config_dir + "/auth.json";
    std::remove(auth_file.c_str());
    std::cout << "Logged out.\n";
    return 0;
}

// ============================================================================
// Registry Operations
// ============================================================================

void PackageManager::set_registry_url(const std::string& url) {
    registry_url_ = url;
}

std::string PackageManager::get_cache_dir() const {
#ifdef _WIN32
    const char* appdata = getenv("LOCALAPPDATA");
    if (appdata) return std::string(appdata) + "\\mana\\cache";
    return ".mana_cache";
#else
    const char* home = getenv("HOME");
    if (home) return std::string(home) + "/.mana/cache";
    return ".mana_cache";
#endif
}

std::optional<RegistryPackage> PackageManager::fetch_package_info(const std::string& name) {
    std::string url = registry_url_ + "/api/v1/packages/" + url_encode(name);
    std::string response = http_get(url);

    if (response.empty()) {
        return std::nullopt;
    }

    // Simple JSON parsing (production would use a proper JSON library)
    RegistryPackage pkg;
    pkg.name = name;

    // Parse "version": "x.y.z"
    std::regex version_re("\"version\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch match;
    if (std::regex_search(response, match, version_re)) {
        pkg.latest_version = match[1];
    }

    // Parse "description": "..."
    std::regex desc_re("\"description\"\\s*:\\s*\"([^\"]+)\"");
    if (std::regex_search(response, match, desc_re)) {
        pkg.description = match[1];
    }

    // Parse "downloads": N
    std::regex dl_re("\"downloads\"\\s*:\\s*(\\d+)");
    if (std::regex_search(response, match, dl_re)) {
        pkg.downloads = std::stoi(match[1]);
    }

    // Parse "license": "..."
    std::regex lic_re("\"license\"\\s*:\\s*\"([^\"]+)\"");
    if (std::regex_search(response, match, lic_re)) {
        pkg.license = match[1];
    }

    // Parse versions array
    std::regex ver_re("\"versions\"\\s*:\\s*\\[([^\\]]+)\\]");
    if (std::regex_search(response, match, ver_re)) {
        std::string versions_str = match[1];
        std::regex ver_item_re("\"([^\"]+)\"");
        std::sregex_iterator it(versions_str.begin(), versions_str.end(), ver_item_re);
        std::sregex_iterator end;
        while (it != end) {
            pkg.versions.push_back((*it)[1]);
            ++it;
        }
    }

    return pkg;
}

bool PackageManager::download_package(const std::string& name, const std::string& version) {
    std::string url = registry_url_ + "/api/v1/packages/" + url_encode(name) +
                      "/versions/" + url_encode(version) + "/download";

    std::string response = http_get(url);
    if (response.empty()) {
        return false;
    }

    // Save to cache
    std::string cache_path = get_cached_package_path(name, version);

    // Create directory structure
    size_t last_slash = cache_path.rfind('/');
    if (last_slash != std::string::npos) {
        std::string dir = cache_path.substr(0, last_slash);
        create_directory(dir);
    }

    return write_file(cache_path + ".tar.gz", response);
}

std::vector<RegistryPackage> PackageManager::search_registry(const std::string& query) {
    std::vector<RegistryPackage> results;

    std::string url = registry_url_ + "/api/v1/search?q=" + url_encode(query);
    std::string response = http_get(url);

    if (response.empty()) {
        // Return some mock results for offline/demo mode
        RegistryPackage mock;
        mock.name = "mana-std";
        mock.description = "Mana standard library extensions";
        mock.latest_version = "1.0.0";
        mock.downloads = 1234;
        results.push_back(mock);

        mock.name = "mana-json";
        mock.description = "JSON parsing and serialization for Mana";
        mock.latest_version = "0.5.0";
        mock.downloads = 567;
        results.push_back(mock);

        mock.name = "mana-http";
        mock.description = "HTTP client library for Mana";
        mock.latest_version = "0.3.0";
        mock.downloads = 890;
        results.push_back(mock);
    }

    // In production, parse actual JSON response here
    return results;
}

bool PackageManager::upload_package(const Package& pkg) {
    // Create package archive
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << pkg.name << "\",\n";
    json << "  \"version\": \"" << pkg.version << "\",\n";
    json << "  \"description\": \"" << pkg.description << "\",\n";
    json << "  \"license\": \"" << pkg.license << "\",\n";
    json << "  \"authors\": [";
    for (size_t i = 0; i < pkg.authors.size(); ++i) {
        if (i > 0) json << ", ";
        json << "\"" << pkg.authors[i] << "\"";
    }
    json << "],\n";
    json << "  \"dependencies\": {";
    for (size_t i = 0; i < pkg.dependencies.size(); ++i) {
        if (i > 0) json << ", ";
        json << "\"" << pkg.dependencies[i].name << "\": \""
             << pkg.dependencies[i].version << "\"";
    }
    json << "}\n";
    json << "}\n";

    std::string url = registry_url_ + "/api/v1/packages/publish";
    std::string response = http_post(url, json.str());

    return !response.empty() && response.find("error") == std::string::npos;
}

// ============================================================================
// Dependency Resolution
// ============================================================================

std::vector<ResolvedDep> PackageManager::resolve_dependencies() {
    std::vector<ResolvedDep> resolved;
    std::set<std::string> visited;

    std::function<void(const std::vector<Dependency>&)> resolve_recursive;
    resolve_recursive = [&](const std::vector<Dependency>& deps) {
        for (const auto& dep : deps) {
            std::string key = dep.name + "@" + dep.version;
            if (visited.count(key)) continue;
            visited.insert(key);

            ResolvedDep rdep;
            rdep.name = dep.name;

            // Fetch info to get exact version
            auto pkg_info = fetch_package_info(dep.name);
            if (pkg_info.has_value()) {
                rdep.version = find_best_version(pkg_info->versions, dep.version);
                if (rdep.version.empty()) {
                    rdep.version = pkg_info->latest_version;
                }
                rdep.url = registry_url_ + "/api/v1/packages/" + dep.name +
                          "/versions/" + rdep.version + "/download";
            } else {
                rdep.version = dep.version;
            }

            resolved.push_back(rdep);

            // Resolve transitive dependencies
            // (In production, fetch package manifest and recurse)
        }
    };

    resolve_recursive(package_.dependencies);
    return resolved;
}

bool PackageManager::check_version_compatible(const std::string& required, const std::string& available) {
    if (required == "*" || required.empty()) return true;
    if (required == available) return true;

    // Handle ^x.y.z (compatible with)
    if (required[0] == '^') {
        std::string req = required.substr(1);
        // Parse major version
        size_t dot = req.find('.');
        std::string req_major = (dot != std::string::npos) ? req.substr(0, dot) : req;
        dot = available.find('.');
        std::string avail_major = (dot != std::string::npos) ? available.substr(0, dot) : available;
        return req_major == avail_major;
    }

    // Handle ~x.y.z (approximately)
    if (required[0] == '~') {
        std::string req = required.substr(1);
        // Match major.minor
        size_t dot1 = req.find('.');
        size_t dot2 = (dot1 != std::string::npos) ? req.find('.', dot1 + 1) : std::string::npos;
        std::string req_prefix = (dot2 != std::string::npos) ? req.substr(0, dot2) : req;

        dot1 = available.find('.');
        dot2 = (dot1 != std::string::npos) ? available.find('.', dot1 + 1) : std::string::npos;
        std::string avail_prefix = (dot2 != std::string::npos) ? available.substr(0, dot2) : available;

        return req_prefix == avail_prefix;
    }

    // Handle >=x.y.z
    if (required.substr(0, 2) == ">=") {
        // Simple numeric comparison (production would use semver library)
        return available >= required.substr(2);
    }

    return required == available;
}

std::string PackageManager::find_best_version(const std::vector<std::string>& versions,
                                              const std::string& constraint) {
    for (auto it = versions.rbegin(); it != versions.rend(); ++it) {
        if (check_version_compatible(constraint, *it)) {
            return *it;
        }
    }
    return "";
}

// ============================================================================
// Cache Management
// ============================================================================

std::string PackageManager::get_cached_package_path(const std::string& name, const std::string& version) {
    return get_cache_dir() + "/" + name + "/" + version;
}

bool PackageManager::is_package_cached(const std::string& name, const std::string& version) {
    std::string path = get_cached_package_path(name, version);
    return file_exists(path) || file_exists(path + ".tar.gz");
}

void PackageManager::clean_cache() {
    std::string cache_dir = get_cache_dir();
    remove_directory(cache_dir);
    std::cout << "Cache cleared.\n";
}

bool PackageManager::remove_directory(const std::string& path) {
#ifdef _WIN32
    std::string cmd = "rmdir /s /q \"" + path + "\" 2>nul";
#else
    std::string cmd = "rm -rf \"" + path + "\"";
#endif
    return run_command(cmd) == 0;
}

// ============================================================================
// Authentication
// ============================================================================

bool PackageManager::load_auth() {
#ifdef _WIN32
    const char* appdata = getenv("LOCALAPPDATA");
    std::string config_dir = appdata ? std::string(appdata) + "\\mana\\config" : ".mana";
#else
    const char* home = getenv("HOME");
    std::string config_dir = home ? std::string(home) + "/.mana/config" : ".mana";
#endif

    std::string auth_file = config_dir + "/auth.json";
    std::string content = read_file(auth_file);

    if (content.empty()) return false;

    // Simple token extraction
    std::regex token_re("\"token\"\\s*:\\s*\"([^\"]+)\"");
    std::smatch match;
    if (std::regex_search(content, match, token_re)) {
        auth_token_ = match[1];
        return true;
    }

    return false;
}

bool PackageManager::save_auth() {
#ifdef _WIN32
    const char* appdata = getenv("LOCALAPPDATA");
    std::string config_dir = appdata ? std::string(appdata) + "\\mana\\config" : ".mana";
#else
    const char* home = getenv("HOME");
    std::string config_dir = home ? std::string(home) + "/.mana/config" : ".mana";
#endif

    create_directory(config_dir);

    std::string auth_file = config_dir + "/auth.json";
    std::string content = "{\n  \"token\": \"" + auth_token_ + "\"\n}\n";

    return write_file(auth_file, content);
}

// ============================================================================
// HTTP Helpers
// ============================================================================

std::string PackageManager::url_encode(const std::string& str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }

    return escaped.str();
}

#ifdef _WIN32
std::string PackageManager::http_get(const std::string& url) {
    // Parse URL
    std::wstring wurl(url.begin(), url.end());

    URL_COMPONENTS urlComp = {};
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t hostName[256] = {};
    wchar_t urlPath[1024] = {};
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = 256;
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = 1024;

    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &urlComp)) {
        return "";
    }

    HINTERNET hSession = WinHttpOpen(L"Mana Package Manager/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, hostName,
                                        urlComp.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath,
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Add auth header if present
    if (!auth_token_.empty()) {
        std::wstring auth = L"Authorization: Bearer " +
                           std::wstring(auth_token_.begin(), auth_token_.end());
        WinHttpAddRequestHeaders(hRequest, auth.c_str(), -1,
                                WINHTTP_ADDREQ_FLAG_ADD);
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    std::string response;
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize > 0) {
            std::vector<char> buffer(dwSize + 1);
            DWORD dwDownloaded = 0;
            WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
            response.append(buffer.data(), dwDownloaded);
        }
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

std::string PackageManager::http_post(const std::string& url, const std::string& body) {
    std::wstring wurl(url.begin(), url.end());

    URL_COMPONENTS urlComp = {};
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t hostName[256] = {};
    wchar_t urlPath[1024] = {};
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = 256;
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = 1024;

    if (!WinHttpCrackUrl(wurl.c_str(), 0, 0, &urlComp)) {
        return "";
    }

    HINTERNET hSession = WinHttpOpen(L"Mana Package Manager/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return "";

    HINTERNET hConnect = WinHttpConnect(hSession, hostName,
                                        urlComp.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return "";
    }

    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", urlPath,
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    // Add headers
    WinHttpAddRequestHeaders(hRequest,
        L"Content-Type: application/json\r\n", -1,
        WINHTTP_ADDREQ_FLAG_ADD);

    if (!auth_token_.empty()) {
        std::wstring auth = L"Authorization: Bearer " +
                           std::wstring(auth_token_.begin(), auth_token_.end());
        WinHttpAddRequestHeaders(hRequest, auth.c_str(), -1,
                                WINHTTP_ADDREQ_FLAG_ADD);
    }

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            (LPVOID)body.c_str(), body.size(), body.size(), 0)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    std::string response;
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        WinHttpQueryDataAvailable(hRequest, &dwSize);
        if (dwSize > 0) {
            std::vector<char> buffer(dwSize + 1);
            DWORD dwDownloaded = 0;
            WinHttpReadData(hRequest, buffer.data(), dwSize, &dwDownloaded);
            response.append(buffer.data(), dwDownloaded);
        }
    } while (dwSize > 0);

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}
#else
// Unix implementation using sockets
std::string PackageManager::http_get(const std::string& url) {
    // Parse URL
    size_t proto_end = url.find("://");
    size_t host_start = (proto_end != std::string::npos) ? proto_end + 3 : 0;
    size_t path_start = url.find('/', host_start);

    std::string host = (path_start != std::string::npos)
        ? url.substr(host_start, path_start - host_start)
        : url.substr(host_start);
    std::string path = (path_start != std::string::npos)
        ? url.substr(path_start)
        : "/";

    int port = 80;
    size_t port_pos = host.find(':');
    if (port_pos != std::string::npos) {
        port = std::stoi(host.substr(port_pos + 1));
        host = host.substr(0, port_pos);
    }

    // Connect
    struct hostent* server = gethostbyname(host.c_str());
    if (!server) return "";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return "";
    }

    // Send request
    std::string request = "GET " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    if (!auth_token_.empty()) {
        request += "Authorization: Bearer " + auth_token_ + "\r\n";
    }
    request += "Connection: close\r\n\r\n";

    send(sock, request.c_str(), request.size(), 0);

    // Read response
    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        response += buffer;
    }

    close(sock);

    // Extract body
    size_t body_start = response.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        return response.substr(body_start + 4);
    }

    return response;
}

std::string PackageManager::http_post(const std::string& url, const std::string& body) {
    // Parse URL (same as http_get)
    size_t proto_end = url.find("://");
    size_t host_start = (proto_end != std::string::npos) ? proto_end + 3 : 0;
    size_t path_start = url.find('/', host_start);

    std::string host = (path_start != std::string::npos)
        ? url.substr(host_start, path_start - host_start)
        : url.substr(host_start);
    std::string path = (path_start != std::string::npos)
        ? url.substr(path_start)
        : "/";

    int port = 80;
    size_t port_pos = host.find(':');
    if (port_pos != std::string::npos) {
        port = std::stoi(host.substr(port_pos + 1));
        host = host.substr(0, port_pos);
    }

    struct hostent* server = gethostbyname(host.c_str());
    if (!server) return "";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return "";

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sock);
        return "";
    }

    std::string request = "POST " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    if (!auth_token_.empty()) {
        request += "Authorization: Bearer " + auth_token_ + "\r\n";
    }
    request += "Connection: close\r\n\r\n";
    request += body;

    send(sock, request.c_str(), request.size(), 0);

    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        response += buffer;
    }

    close(sock);

    size_t body_start = response.find("\r\n\r\n");
    if (body_start != std::string::npos) {
        return response.substr(body_start + 4);
    }

    return response;
}
#endif

} // namespace mana::pkg
