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

    // Create mana_runtime.h
    if (!write_file(name + "/mana_runtime.h", get_runtime_header())) {
        std::cerr << "Error: Could not create mana_runtime.h\n";
        return 1;
    }

    // Graphics-specific files
    if (graphics) {
        // Create graphics.mana
        if (!write_file(name + "/src/graphics.mana", get_graphics_mana())) {
            std::cerr << "Error: Could not create graphics.mana\n";
            return 1;
        }

        // Create mana_graphics.h
        if (!write_file(name + "/mana_graphics.h", get_graphics_header())) {
            std::cerr << "Error: Could not create mana_graphics.h\n";
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
target_include_directories()" + name + R"( PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries()" + name + R"( PRIVATE glfw glad::glad)

if(WIN32)
    target_link_libraries()" + name + R"( PRIVATE opengl32)
endif()
)";
        write_file(name + "/CMakeLists.txt", cmake);
    }

    // Create .gitignore
    std::string gitignore = R"(# Build artifacts
/build/
*.o
*.obj
*.exe

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
