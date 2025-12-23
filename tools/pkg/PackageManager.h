#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <set>

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
        std::vector<std::string> keywords;
        std::string repository;
    };

    // Registry package metadata
    struct RegistryPackage {
        std::string name;
        std::string description;
        std::vector<std::string> versions;
        std::string latest_version;
        std::vector<std::string> authors;
        std::string license;
        std::string repository;
        std::vector<std::string> keywords;
        int downloads;
    };

    // Resolved dependency with full metadata
    struct ResolvedDep {
        std::string name;
        std::string version;
        std::string url;
        std::vector<Dependency> transitive_deps;
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

        // Registry commands
        int search(const std::string& query);
        int info(const std::string& package_name);
        int install();  // Install all dependencies
        int login(const std::string& token);
        int logout();

        // Load/save package file
        bool load_package(const std::string& path = "package.toml");
        bool save_package(const std::string& path = "package.toml");

        // Configuration
        void set_registry_url(const std::string& url);
        std::string get_cache_dir() const;

    private:
        Package package_;
        std::string package_dir_;
        std::string registry_url_ = "https://registry.mana-lang.org";
        std::string auth_token_;

        // File operations
        bool create_directory(const std::string& path);
        bool write_file(const std::string& path, const std::string& content);
        std::string read_file(const std::string& path);
        bool file_exists(const std::string& path);
        int run_command(const std::string& cmd);
        bool remove_directory(const std::string& path);

        // Template files
        std::string get_runtime_header();
        std::string get_graphics_header();
        std::string get_graphics_mana();
        std::string get_executable_path();

        // Registry operations
        std::optional<RegistryPackage> fetch_package_info(const std::string& name);
        bool download_package(const std::string& name, const std::string& version);
        std::vector<RegistryPackage> search_registry(const std::string& query);
        bool upload_package(const Package& pkg);

        // Dependency resolution
        std::vector<ResolvedDep> resolve_dependencies();
        bool check_version_compatible(const std::string& required, const std::string& available);
        std::string find_best_version(const std::vector<std::string>& versions, const std::string& constraint);

        // Cache management
        std::string get_cached_package_path(const std::string& name, const std::string& version);
        bool is_package_cached(const std::string& name, const std::string& version);
        void clean_cache();

        // Auth
        bool load_auth();
        bool save_auth();

        // HTTP helpers (platform-independent)
        std::string http_get(const std::string& url);
        std::string http_post(const std::string& url, const std::string& body);
        std::string url_encode(const std::string& str);
    };

} // namespace mana::pkg
