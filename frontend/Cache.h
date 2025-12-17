#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <optional>

namespace mana::frontend {

    // Simple file hash using contents
    inline std::string compute_file_hash(const std::string& content) {
        // Simple hash - sum of characters with position weighting
        size_t hash = 0;
        size_t pos = 1;
        for (char c : content) {
            hash = hash * 31 + static_cast<unsigned char>(c) * pos;
            pos++;
        }

        // Convert to hex string
        std::ostringstream ss;
        ss << std::hex << hash;
        return ss.str();
    }

    // Cache entry for a single file
    struct CacheEntry {
        std::string file_path;
        std::string content_hash;
        std::string cpp_output;      // Generated C++ code
        int64_t timestamp;           // Last modification time
    };

    // Simple incremental compilation cache
    class CompilationCache {
    public:
        CompilationCache() = default;

        // Set the cache directory
        void set_cache_dir(const std::filesystem::path& dir) {
            cache_dir_ = dir;
            if (!std::filesystem::exists(cache_dir_)) {
                std::filesystem::create_directories(cache_dir_);
            }
            load_cache_index();
        }

        // Check if a file is cached and unchanged
        bool is_cached(const std::string& file_path, const std::string& content) const {
            auto it = entries_.find(file_path);
            if (it == entries_.end()) return false;

            std::string current_hash = compute_file_hash(content);
            return it->second.content_hash == current_hash;
        }

        // Get cached C++ output
        std::optional<std::string> get_cached_cpp(const std::string& file_path) const {
            auto it = entries_.find(file_path);
            if (it == entries_.end()) return std::nullopt;

            // Read from cache file
            std::filesystem::path cache_file = cache_dir_ / (it->second.content_hash + ".cpp");
            if (!std::filesystem::exists(cache_file)) return std::nullopt;

            std::ifstream in(cache_file);
            if (!in) return std::nullopt;

            std::ostringstream ss;
            ss << in.rdbuf();
            return ss.str();
        }

        // Store cache entry
        void store(const std::string& file_path, const std::string& content, const std::string& cpp_output) {
            std::string hash = compute_file_hash(content);

            CacheEntry entry;
            entry.file_path = file_path;
            entry.content_hash = hash;
            entry.cpp_output = cpp_output;
            entry.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();

            entries_[file_path] = entry;

            // Write C++ to cache file
            std::filesystem::path cache_file = cache_dir_ / (hash + ".cpp");
            std::ofstream out(cache_file);
            if (out) {
                out << cpp_output;
            }

            save_cache_index();
        }

        // Invalidate cache for a file
        void invalidate(const std::string& file_path) {
            auto it = entries_.find(file_path);
            if (it != entries_.end()) {
                // Remove cached cpp file
                std::filesystem::path cache_file = cache_dir_ / (it->second.content_hash + ".cpp");
                std::filesystem::remove(cache_file);
                entries_.erase(it);
                save_cache_index();
            }
        }

        // Clear all cache
        void clear() {
            for (const auto& [path, entry] : entries_) {
                std::filesystem::path cache_file = cache_dir_ / (entry.content_hash + ".cpp");
                std::filesystem::remove(cache_file);
            }
            entries_.clear();
            save_cache_index();
        }

        // Get cache statistics
        size_t size() const { return entries_.size(); }
        bool empty() const { return entries_.empty(); }

    private:
        std::filesystem::path cache_dir_;
        std::unordered_map<std::string, CacheEntry> entries_;

        void load_cache_index() {
            std::filesystem::path index_file = cache_dir_ / "cache_index.txt";
            if (!std::filesystem::exists(index_file)) return;

            std::ifstream in(index_file);
            if (!in) return;

            entries_.clear();
            std::string line;
            while (std::getline(in, line)) {
                // Format: file_path|hash|timestamp
                size_t pos1 = line.find('|');
                size_t pos2 = line.find('|', pos1 + 1);
                if (pos1 == std::string::npos || pos2 == std::string::npos) continue;

                CacheEntry entry;
                entry.file_path = line.substr(0, pos1);
                entry.content_hash = line.substr(pos1 + 1, pos2 - pos1 - 1);
                entry.timestamp = std::stoll(line.substr(pos2 + 1));
                entries_[entry.file_path] = entry;
            }
        }

        void save_cache_index() {
            std::filesystem::path index_file = cache_dir_ / "cache_index.txt";
            std::ofstream out(index_file);
            if (!out) return;

            for (const auto& [path, entry] : entries_) {
                out << entry.file_path << "|" << entry.content_hash << "|" << entry.timestamp << "\n";
            }
        }
    };

} // namespace mana::frontend
