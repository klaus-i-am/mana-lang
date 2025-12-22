#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace mana::pkg {

    struct Dependency {
        std::string name;
        std::string version;
        std::string source;  // "registry", "git", "path"
        std::string url;     // For git/path sources
    };

    struct Package {
        std::string name;
        std::string version;
        std::string description;
        std::vector<std::string> authors;
        std::string license;
        std::string entry_point = "src/main.mana";
        std::vector<Dependency> dependencies;
    };

    class PackageManager {
    public:
        PackageManager();

        // Commands
        int init(const std::string& name, bool graphics = false);
        int build();
        int run();
        int test();
        int add(const std::string& dep_spec);
        int remove(const std::string& name);
        int update();
        int publish();

        // Load/save package file
        bool load_package(const std::string& path = "package.toml");
        bool save_package(const std::string& path = "package.toml");

    private:
        Package package_;
        std::string package_dir_;

        bool create_directory(const std::string& path);
        bool write_file(const std::string& path, const std::string& content);
        std::string read_file(const std::string& path);
        bool file_exists(const std::string& path);
        int run_command(const std::string& cmd);
        std::string get_runtime_header();
        std::string get_graphics_header();
        std::string get_graphics_mana();
        std::string get_executable_path();
    };

} // namespace mana::pkg
