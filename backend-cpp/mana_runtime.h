#pragma once
#include <utility>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <string>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <cctype>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <atomic>
#include <chrono>

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

    // Variadic print/println - uses fold expressions (C++17)
    template <typename T, typename... Args>
    void print(T&& first, Args&&... rest) {
        print(std::forward<T>(first));
        (print(std::forward<Args>(rest)), ...);
    }

    template <typename T, typename... Args>
    void println(T&& first, Args&&... rest) {
        print(std::forward<T>(first));
        (print(std::forward<Args>(rest)), ...);
        std::printf("\n");
    }

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
    // HashMap<K, V> - Key-value collection
    template <typename K, typename V>
    class HashMap {
        std::unordered_map<K, V> data_;
    public:
        HashMap() = default;

        void insert(const K& key, const V& value) { data_[key] = value; }

        Option<V> get(const K& key) const {
            auto it = data_.find(key);
            if (it != data_.end()) return Option<V>(it->second);
            return Option<V>();
        }

        V& operator[](const K& key) { return data_[key]; }

        bool contains(const K& key) const { return data_.count(key) > 0; }

        bool remove(const K& key) {
            auto it = data_.find(key);
            if (it != data_.end()) {
                data_.erase(it);
                return true;
            }
            return false;
        }

        size_t len() const { return data_.size(); }
        bool is_empty() const { return data_.empty(); }
        void clear() { data_.clear(); }

        Vec<K> keys() const {
            Vec<K> result;
            for (const auto& pair : data_) {
                result.push(pair.first);
            }
            return result;
        }

        Vec<V> values() const {
            Vec<V> result;
            for (const auto& pair : data_) {
                result.push(pair.second);
            }
            return result;
        }

        // Iterator support
        auto begin() { return data_.begin(); }
        auto end() { return data_.end(); }
        auto begin() const { return data_.begin(); }
        auto end() const { return data_.end(); }
    };


    // String functions
    inline size_t len(const std::string& s) { return s.size(); }
    inline bool is_empty(const std::string& s) { return s.empty(); }

    inline std::string to_string(int32_t v) { return std::to_string(v); }
    inline std::string to_string(float v) { return std::to_string(v); }
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

    inline std::string replace(const std::string& s, const std::string& from, const std::string& to) {
        std::string result = s;
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
        return result;
    }

    inline std::string to_uppercase(const std::string& s) {
        std::string result = s;
        for (auto& c : result) c = std::toupper(c);
        return result;
    }

    inline std::string to_lowercase(const std::string& s) {
        std::string result = s;
        for (auto& c : result) c = std::tolower(c);
        return result;
    }

    inline Option<size_t> find(const std::string& s, const std::string& sub) {
        size_t pos = s.find(sub);
        if (pos != std::string::npos) return Option<size_t>(pos);
        return Option<size_t>();
    }

    inline Vec<std::string> split(const std::string& s, const std::string& delim) {
        Vec<std::string> result;
        size_t start = 0, end = 0;
        while ((end = s.find(delim, start)) != std::string::npos) {
            result.push(s.substr(start, end - start));
            start = end + delim.length();
        }
        result.push(s.substr(start));
        return result;
    }

    inline std::string repeat(const std::string& s, size_t count) {
        std::string result;
        result.reserve(s.size() * count);
        for (size_t i = 0; i < count; ++i) result += s;
        return result;
    }

    inline std::string reverse(const std::string& s) {
        return std::string(s.rbegin(), s.rend());
    }

    inline std::string join(const Vec<std::string>& v, const std::string& sep) {
        std::string result;
        for (size_t i = 0; i < v.len(); ++i) {
            if (i > 0) result += sep;
            result += v[i];
        }
        return result;
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

    // Advanced math functions
    inline float sqrt(float x) { return std::sqrt(x); }
    inline double sqrt(double x) { return std::sqrt(x); }

    inline float sin(float x) { return std::sin(x); }
    inline double sin(double x) { return std::sin(x); }

    inline float cos(float x) { return std::cos(x); }
    inline double cos(double x) { return std::cos(x); }

    inline float tan(float x) { return std::tan(x); }
    inline double tan(double x) { return std::tan(x); }

    inline float asin(float x) { return std::asin(x); }
    inline double asin(double x) { return std::asin(x); }

    inline float acos(float x) { return std::acos(x); }
    inline double acos(double x) { return std::acos(x); }

    inline float atan(float x) { return std::atan(x); }
    inline double atan(double x) { return std::atan(x); }

    inline float atan2(float y, float x) { return std::atan2(y, x); }
    inline double atan2(double y, double x) { return std::atan2(y, x); }

    inline float pow(float base, float exp) { return std::pow(base, exp); }
    inline double pow(double base, double exp) { return std::pow(base, exp); }

    inline float exp(float x) { return std::exp(x); }
    inline double exp(double x) { return std::exp(x); }

    inline float log(float x) { return std::log(x); }
    inline double log(double x) { return std::log(x); }

    inline float log10(float x) { return std::log10(x); }
    inline double log10(double x) { return std::log10(x); }

    inline float log2(float x) { return std::log2(x); }
    inline double log2(double x) { return std::log2(x); }

    inline float floor(float x) { return std::floor(x); }
    inline double floor(double x) { return std::floor(x); }

    inline float ceil(float x) { return std::ceil(x); }
    inline double ceil(double x) { return std::ceil(x); }

    inline float round(float x) { return std::round(x); }
    inline double round(double x) { return std::round(x); }

    inline float trunc(float x) { return std::trunc(x); }
    inline double trunc(double x) { return std::trunc(x); }

    inline float fmod(float x, float y) { return std::fmod(x, y); }
    inline double fmod(double x, double y) { return std::fmod(x, y); }

    inline float hypot(float x, float y) { return std::hypot(x, y); }
    inline double hypot(double x, double y) { return std::hypot(x, y); }

    // Math constants
    constexpr float PI_F = 3.14159265358979323846f;
    constexpr double PI = 3.14159265358979323846;
    constexpr float E_F = 2.71828182845904523536f;
    constexpr double E = 2.71828182845904523536;

    // Angle conversion
    inline float to_radians(float degrees) { return degrees * PI_F / 180.0f; }
    inline double to_radians(double degrees) { return degrees * PI / 180.0; }
    inline float to_degrees(float radians) { return radians * 180.0f / PI_F; }
    inline double to_degrees(double radians) { return radians * 180.0 / PI; }

    // Random number generation
    inline void seed_random(int32_t seed) {
        std::srand(static_cast<unsigned int>(seed));
    }

    inline int32_t random_int(int32_t min_val, int32_t max_val) {
        static bool seeded = false;
        if (!seeded) { std::srand(static_cast<unsigned int>(std::time(nullptr))); seeded = true; }
        return min_val + std::rand() % (max_val - min_val + 1);
    }

    inline float random_float() {
        static bool seeded = false;
        if (!seeded) { std::srand(static_cast<unsigned int>(std::time(nullptr))); seeded = true; }
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    inline float random_range(float min_val, float max_val) {
        return min_val + random_float() * (max_val - min_val);
    }

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


    // File I/O functions
    inline Result<std::string, std::string> read_file(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return Result<std::string, std::string>::Err("Failed to open file: " + path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return Result<std::string, std::string>::Ok(buffer.str());
    }

    inline Result<bool, std::string> write_file(const std::string& path, const std::string& content) {
        std::ofstream file(path);
        if (!file.is_open()) {
            return Result<bool, std::string>::Err("Failed to open file for writing: " + path);
        }
        file << content;
        return Result<bool, std::string>::Ok(true);
    }

    inline Result<bool, std::string> append_file(const std::string& path, const std::string& content) {
        std::ofstream file(path, std::ios::app);
        if (!file.is_open()) {
            return Result<bool, std::string>::Err("Failed to open file for appending: " + path);
        }
        file << content;
        return Result<bool, std::string>::Ok(true);
    }

    inline bool file_exists(const std::string& path) {
        return std::filesystem::exists(path);
    }

    inline Result<bool, std::string> delete_file(const std::string& path) {
        try {
            if (std::filesystem::remove(path)) {
                return Result<bool, std::string>::Ok(true);
            }
            return Result<bool, std::string>::Err("File not found: " + path);
        } catch (const std::exception& e) {
            return Result<bool, std::string>::Err(std::string("Error deleting file: ") + e.what());
        }
    }

    inline Result<Vec<std::string>, std::string> read_lines(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            return Result<Vec<std::string>, std::string>::Err("Failed to open file: " + path);
        }
        Vec<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push(line);
        }
        return Result<Vec<std::string>, std::string>::Ok(std::move(lines));
    }

    // Assert function
    inline void assert_true(bool condition, const char* msg = "assertion failed") {
        if (!condition) throw std::runtime_error(msg);
    }

    // ============================================================================
    // Async Runtime Support
    // ============================================================================

    // Thread pool for efficient task scheduling
    class ThreadPool {
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::mutex queue_mutex_;
        std::condition_variable condition_;
        std::atomic<bool> stop_{false};

    public:
        explicit ThreadPool(size_t num_threads = 0) {
            if (num_threads == 0) {
                num_threads = std::thread::hardware_concurrency();
                if (num_threads == 0) num_threads = 4;
            }

            for (size_t i = 0; i < num_threads; ++i) {
                workers_.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(queue_mutex_);
                            condition_.wait(lock, [this] {
                                return stop_.load() || !tasks_.empty();
                            });
                            if (stop_.load() && tasks_.empty()) return;
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                        task();
                    }
                });
            }
        }

        ~ThreadPool() {
            stop_.store(true);
            condition_.notify_all();
            for (auto& worker : workers_) {
                if (worker.joinable()) worker.join();
            }
        }

        template<typename F, typename... Args>
        auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
            using return_type = decltype(f(args...));
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );
            std::future<return_type> result = task->get_future();
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                tasks_.emplace([task]() { (*task)(); });
            }
            condition_.notify_one();
            return result;
        }

        size_t size() const { return workers_.size(); }
    };

    // Global thread pool singleton
    inline ThreadPool& global_pool() {
        static ThreadPool pool;
        return pool;
    }

    // Task<T> - ergonomic async task wrapper
    template<typename T>
    class Task {
        std::future<T> future_;
        bool valid_ = false;

    public:
        Task() = default;
        explicit Task(std::future<T>&& f) : future_(std::move(f)), valid_(true) {}

        Task(Task&& other) noexcept : future_(std::move(other.future_)), valid_(other.valid_) {
            other.valid_ = false;
        }

        Task& operator=(Task&& other) noexcept {
            future_ = std::move(other.future_);
            valid_ = other.valid_;
            other.valid_ = false;
            return *this;
        }

        // Get the result (blocks if not ready)
        T get() {
            if (!valid_) throw std::runtime_error("Task already consumed or invalid");
            valid_ = false;
            return future_.get();
        }

        // Check if result is ready
        bool is_ready() const {
            if (!valid_) return false;
            return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }

        // Wait for completion with timeout (returns true if ready)
        bool wait_for(int64_t millis) {
            if (!valid_) return true;
            return future_.wait_for(std::chrono::milliseconds(millis)) == std::future_status::ready;
        }

        // Wait for completion
        void wait() {
            if (valid_) future_.wait();
        }

        bool is_valid() const { return valid_; }
    };

    // Specialization for void
    template<>
    class Task<void> {
        std::future<void> future_;
        bool valid_ = false;

    public:
        Task() = default;
        explicit Task(std::future<void>&& f) : future_(std::move(f)), valid_(true) {}

        Task(Task&& other) noexcept : future_(std::move(other.future_)), valid_(other.valid_) {
            other.valid_ = false;
        }

        void get() {
            if (!valid_) throw std::runtime_error("Task already consumed or invalid");
            valid_ = false;
            future_.get();
        }

        bool is_ready() const {
            if (!valid_) return false;
            return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
        }

        bool wait_for(int64_t millis) {
            if (!valid_) return true;
            return future_.wait_for(std::chrono::milliseconds(millis)) == std::future_status::ready;
        }

        void wait() {
            if (valid_) future_.wait();
        }

        bool is_valid() const { return valid_; }
    };

    // spawn - launch a task on the thread pool
    template<typename F, typename... Args>
    auto spawn(F&& f, Args&&... args) -> Task<decltype(f(args...))> {
        return Task<decltype(f(args...))>(
            global_pool().submit(std::forward<F>(f), std::forward<Args>(args)...)
        );
    }

    // spawn_async - launch task using std::async (for comparison/fallback)
    template<typename F, typename... Args>
    auto spawn_async(F&& f, Args&&... args) -> Task<decltype(f(args...))> {
        return Task<decltype(f(args...))>(
            std::async(std::launch::async, std::forward<F>(f), std::forward<Args>(args)...)
        );
    }

    // sleep - pause current thread for given milliseconds
    inline void sleep(int64_t millis) {
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }

    // sleep_seconds - pause for seconds
    inline void sleep_seconds(double seconds) {
        auto duration = std::chrono::duration<double>(seconds);
        std::this_thread::sleep_for(duration);
    }

    // yield - give up current time slice
    inline void yield() {
        std::this_thread::yield();
    }

    // Channel<T> - async message passing between tasks
    template<typename T>
    class Channel {
        std::queue<T> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
        std::atomic<bool> closed_{false};
        size_t capacity_;

    public:
        explicit Channel(size_t capacity = 0) : capacity_(capacity) {}

        // Send a value (blocks if channel is full and bounded)
        bool send(T value) {
            std::unique_lock<std::mutex> lock(mutex_);
            if (capacity_ > 0) {
                cond_.wait(lock, [this] {
                    return closed_.load() || queue_.size() < capacity_;
                });
            }
            if (closed_.load()) return false;
            queue_.push(std::move(value));
            lock.unlock();
            cond_.notify_one();
            return true;
        }

        // Receive a value (blocks until available or channel is closed)
        Option<T> recv() {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] {
                return closed_.load() || !queue_.empty();
            });
            if (queue_.empty()) return Option<T>();
            T value = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            cond_.notify_one();
            return Option<T>(std::move(value));
        }

        // Try to receive without blocking
        Option<T> try_recv() {
            std::lock_guard<std::mutex> lock(mutex_);
            if (queue_.empty()) return Option<T>();
            T value = std::move(queue_.front());
            queue_.pop();
            cond_.notify_one();
            return Option<T>(std::move(value));
        }

        // Try to send without blocking
        bool try_send(T value) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (closed_.load()) return false;
            if (capacity_ > 0 && queue_.size() >= capacity_) return false;
            queue_.push(std::move(value));
            cond_.notify_one();
            return true;
        }

        // Close the channel
        void close() {
            closed_.store(true);
            cond_.notify_all();
        }

        bool is_closed() const { return closed_.load(); }
        bool is_empty() const {
            std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
            return queue_.empty();
        }
    };

    // Timer - schedule delayed execution
    class Timer {
        std::atomic<bool> cancelled_{false};
        std::thread thread_;

    public:
        Timer() = default;
        ~Timer() { cancel(); }

        template<typename F>
        void set(int64_t delay_ms, F&& callback) {
            cancel();
            cancelled_.store(false);
            thread_ = std::thread([this, delay_ms, cb = std::forward<F>(callback)]() {
                auto end_time = std::chrono::steady_clock::now() +
                               std::chrono::milliseconds(delay_ms);
                while (!cancelled_.load()) {
                    if (std::chrono::steady_clock::now() >= end_time) {
                        if (!cancelled_.load()) cb();
                        return;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            });
        }

        void cancel() {
            cancelled_.store(true);
            if (thread_.joinable()) thread_.join();
        }
    };

    // delay - create a task that completes after given milliseconds
    inline Task<void> delay(int64_t millis) {
        return spawn([millis]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(millis));
        });
    }

    // WaitGroup - wait for multiple tasks to complete
    class WaitGroup {
        std::atomic<int32_t> count_{0};
        std::mutex mutex_;
        std::condition_variable cond_;

    public:
        void add(int32_t delta = 1) {
            count_.fetch_add(delta);
        }

        void done() {
            if (count_.fetch_sub(1) == 1) {
                cond_.notify_all();
            }
        }

        void wait() {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [this] { return count_.load() <= 0; });
        }

        bool wait_for(int64_t millis) {
            std::unique_lock<std::mutex> lock(mutex_);
            return cond_.wait_for(lock, std::chrono::milliseconds(millis),
                                  [this] { return count_.load() <= 0; });
        }
    };

    // Mutex - mutual exclusion lock
    class Mutex {
        std::mutex mutex_;
    public:
        void lock() { mutex_.lock(); }
        void unlock() { mutex_.unlock(); }
        bool try_lock() { return mutex_.try_lock(); }

        // RAII guard
        class Guard {
            Mutex& mutex_;
        public:
            explicit Guard(Mutex& m) : mutex_(m) { mutex_.lock(); }
            ~Guard() { mutex_.unlock(); }
        };

        Guard guard() { return Guard(*this); }
    };

    // Atomic wrapper for common types
    template<typename T>
    class Atomic {
        std::atomic<T> value_;
    public:
        Atomic() : value_(T{}) {}
        explicit Atomic(T value) : value_(value) {}

        T load() const { return value_.load(); }
        void store(T value) { value_.store(value); }
        T exchange(T value) { return value_.exchange(value); }

        // For numeric types
        T fetch_add(T delta) { return value_.fetch_add(delta); }
        T fetch_sub(T delta) { return value_.fetch_sub(delta); }

        T operator++() { return ++value_; }
        T operator++(int) { return value_++; }
        T operator--() { return --value_; }
        T operator--(int) { return value_--; }
    };

    // Once - ensure code runs exactly once
    class Once {
        std::once_flag flag_;
    public:
        template<typename F>
        void call(F&& f) {
            std::call_once(flag_, std::forward<F>(f));
        }
    };

    // ============================================================================
    // Networking Support
    // ============================================================================

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define INVALID_SOCK INVALID_SOCKET
    #define SOCK_ERROR SOCKET_ERROR
    inline int close_socket(socket_t s) { return closesocket(s); }
    inline int get_socket_error() { return WSAGetLastError(); }
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    using socket_t = int;
    #define INVALID_SOCK (-1)
    #define SOCK_ERROR (-1)
    inline int close_socket(socket_t s) { return close(s); }
    inline int get_socket_error() { return errno; }
#endif

    // Socket initialization (required on Windows)
    inline bool net_init() {
#ifdef _WIN32
        WSADATA wsa;
        return WSAStartup(MAKEWORD(2, 2), &wsa) == 0;
#else
        return true;
#endif
    }

    inline void net_cleanup() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    // TCP Socket wrapper
    class TcpSocket {
        socket_t sock_ = INVALID_SOCK;
        bool connected_ = false;

    public:
        TcpSocket() = default;
        ~TcpSocket() { close(); }

        // Non-copyable
        TcpSocket(const TcpSocket&) = delete;
        TcpSocket& operator=(const TcpSocket&) = delete;

        // Movable
        TcpSocket(TcpSocket&& other) noexcept : sock_(other.sock_), connected_(other.connected_) {
            other.sock_ = INVALID_SOCK;
            other.connected_ = false;
        }

        Result<bool, std::string> connect(const std::string& host, int port) {
            sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock_ == INVALID_SOCK) {
                return Result<bool, std::string>::Err("Failed to create socket");
            }

            struct addrinfo hints = {}, *result = nullptr;
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0) {
                close_socket(sock_);
                sock_ = INVALID_SOCK;
                return Result<bool, std::string>::Err("Failed to resolve host: " + host);
            }

            if (::connect(sock_, result->ai_addr, static_cast<int>(result->ai_addrlen)) == SOCK_ERROR) {
                freeaddrinfo(result);
                close_socket(sock_);
                sock_ = INVALID_SOCK;
                return Result<bool, std::string>::Err("Failed to connect to " + host + ":" + std::to_string(port));
            }

            freeaddrinfo(result);
            connected_ = true;
            return Result<bool, std::string>::Ok(true);
        }

        Result<int32_t, std::string> send(const std::string& data) {
            if (!connected_) return Result<int32_t, std::string>::Err("Not connected");
            int sent = ::send(sock_, data.c_str(), static_cast<int>(data.size()), 0);
            if (sent == SOCK_ERROR) {
                return Result<int32_t, std::string>::Err("Send failed");
            }
            return Result<int32_t, std::string>::Ok(sent);
        }

        Result<std::string, std::string> recv(int32_t max_bytes = 4096) {
            if (!connected_) return Result<std::string, std::string>::Err("Not connected");
            std::vector<char> buffer(max_bytes);
            int received = ::recv(sock_, buffer.data(), max_bytes, 0);
            if (received == SOCK_ERROR) {
                return Result<std::string, std::string>::Err("Receive failed");
            }
            if (received == 0) {
                connected_ = false;
                return Result<std::string, std::string>::Err("Connection closed");
            }
            return Result<std::string, std::string>::Ok(std::string(buffer.data(), received));
        }

        void close() {
            if (sock_ != INVALID_SOCK) {
                close_socket(sock_);
                sock_ = INVALID_SOCK;
                connected_ = false;
            }
        }

        bool is_connected() const { return connected_; }
    };

    // TCP Server wrapper
    class TcpServer {
        socket_t sock_ = INVALID_SOCK;
        bool listening_ = false;

    public:
        TcpServer() = default;
        ~TcpServer() { close(); }

        Result<bool, std::string> bind(int port) {
            sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock_ == INVALID_SOCK) {
                return Result<bool, std::string>::Err("Failed to create socket");
            }

            int opt = 1;
            setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

            struct sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(static_cast<uint16_t>(port));

            if (::bind(sock_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == SOCK_ERROR) {
                close_socket(sock_);
                sock_ = INVALID_SOCK;
                return Result<bool, std::string>::Err("Failed to bind to port " + std::to_string(port));
            }

            return Result<bool, std::string>::Ok(true);
        }

        Result<bool, std::string> listen(int backlog = 10) {
            if (::listen(sock_, backlog) == SOCK_ERROR) {
                return Result<bool, std::string>::Err("Failed to listen");
            }
            listening_ = true;
            return Result<bool, std::string>::Ok(true);
        }

        Result<TcpSocket, std::string> accept() {
            if (!listening_) return Result<TcpSocket, std::string>::Err("Not listening");

            struct sockaddr_in client_addr = {};
            socklen_t addr_len = sizeof(client_addr);
            socket_t client_sock = ::accept(sock_, reinterpret_cast<struct sockaddr*>(&client_addr), &addr_len);

            if (client_sock == INVALID_SOCK) {
                return Result<TcpSocket, std::string>::Err("Accept failed");
            }

            TcpSocket client;
            client.sock_ = client_sock;
            client.connected_ = true;
            return Result<TcpSocket, std::string>::Ok(std::move(client));
        }

        void close() {
            if (sock_ != INVALID_SOCK) {
                close_socket(sock_);
                sock_ = INVALID_SOCK;
                listening_ = false;
            }
        }

        bool is_listening() const { return listening_; }

        friend class TcpSocket;
    };

    // Simple HTTP client
    inline Result<std::string, std::string> http_get(const std::string& url) {
        // Parse URL: http://host:port/path
        std::string host, path = "/";
        int port = 80;

        size_t proto_end = url.find("://");
        size_t host_start = (proto_end != std::string::npos) ? proto_end + 3 : 0;
        size_t path_start = url.find('/', host_start);

        std::string host_port;
        if (path_start != std::string::npos) {
            host_port = url.substr(host_start, path_start - host_start);
            path = url.substr(path_start);
        } else {
            host_port = url.substr(host_start);
        }

        size_t port_pos = host_port.find(':');
        if (port_pos != std::string::npos) {
            host = host_port.substr(0, port_pos);
            port = std::stoi(host_port.substr(port_pos + 1));
        } else {
            host = host_port;
        }

        TcpSocket sock;
        auto conn_result = sock.connect(host, port);
        if (conn_result.is_err()) {
            return Result<std::string, std::string>::Err(conn_result.unwrap_err());
        }

        std::string request = "GET " + path + " HTTP/1.1\r\n";
        request += "Host: " + host + "\r\n";
        request += "Connection: close\r\n";
        request += "\r\n";

        auto send_result = sock.send(request);
        if (send_result.is_err()) {
            return Result<std::string, std::string>::Err(send_result.unwrap_err());
        }

        std::string response;
        while (sock.is_connected()) {
            auto recv_result = sock.recv(4096);
            if (recv_result.is_err()) break;
            response += recv_result.unwrap();
        }

        // Extract body (after \r\n\r\n)
        size_t body_start = response.find("\r\n\r\n");
        if (body_start != std::string::npos) {
            return Result<std::string, std::string>::Ok(response.substr(body_start + 4));
        }

        return Result<std::string, std::string>::Ok(response);
    }

    // UDP Socket wrapper
    class UdpSocket {
        socket_t sock_ = INVALID_SOCK;

    public:
        UdpSocket() = default;
        ~UdpSocket() { close(); }

        Result<bool, std::string> bind(int port) {
            sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sock_ == INVALID_SOCK) {
                return Result<bool, std::string>::Err("Failed to create UDP socket");
            }

            struct sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(static_cast<uint16_t>(port));

            if (::bind(sock_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == SOCK_ERROR) {
                close_socket(sock_);
                sock_ = INVALID_SOCK;
                return Result<bool, std::string>::Err("Failed to bind UDP socket");
            }

            return Result<bool, std::string>::Ok(true);
        }

        Result<bool, std::string> open() {
            sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (sock_ == INVALID_SOCK) {
                return Result<bool, std::string>::Err("Failed to create UDP socket");
            }
            return Result<bool, std::string>::Ok(true);
        }

        Result<int32_t, std::string> send_to(const std::string& host, int port, const std::string& data) {
            struct sockaddr_in addr = {};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(static_cast<uint16_t>(port));
            inet_pton(AF_INET, host.c_str(), &addr.sin_addr);

            int sent = sendto(sock_, data.c_str(), static_cast<int>(data.size()), 0,
                             reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr));
            if (sent == SOCK_ERROR) {
                return Result<int32_t, std::string>::Err("UDP send failed");
            }
            return Result<int32_t, std::string>::Ok(sent);
        }

        Result<std::string, std::string> recv_from(int32_t max_bytes = 4096) {
            std::vector<char> buffer(max_bytes);
            struct sockaddr_in sender = {};
            socklen_t sender_len = sizeof(sender);

            int received = recvfrom(sock_, buffer.data(), max_bytes, 0,
                                   reinterpret_cast<struct sockaddr*>(&sender), &sender_len);
            if (received == SOCK_ERROR) {
                return Result<std::string, std::string>::Err("UDP receive failed");
            }
            return Result<std::string, std::string>::Ok(std::string(buffer.data(), received));
        }

        void close() {
            if (sock_ != INVALID_SOCK) {
                close_socket(sock_);
                sock_ = INVALID_SOCK;
            }
        }

        bool is_open() const { return sock_ != INVALID_SOCK; }
    };

}
