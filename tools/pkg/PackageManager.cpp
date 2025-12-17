#include "PackageManager.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
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
    std::string main_content = R"(module main;

fn main() -> void {
    println("Hello, world!");
}
)";
    if (!write_file(name + "/src/main.mana", main_content)) {
        std::cerr << "Error: Could not create main.mana\n";
        return 1;
    }

    // Create .gitignore
    std::string gitignore = R"(# Build artifacts
/build/
/target/
*.o
*.exe

# IDE
.vscode/
.idea/
*.swp

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

    // Compile
    std::string cmd = "mana_lang " + entry + " -c -o build/" + package_.name;
    #ifdef _WIN32
    cmd += ".exe";
    #endif

    int result = run_command(cmd);
    if (result != 0) {
        std::cerr << "Build failed\n";
        return result;
    }

    std::cout << "Build successful: build/" << package_.name << "\n";
    return 0;
}

int PackageManager::run() {
    int result = build();
    if (result != 0) return result;

    std::string exe = "build/" + package_.name;
    #ifdef _WIN32
    exe += ".exe";
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
