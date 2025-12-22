#pragma once
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
    inline void println(double v) { std::printf("%g\n", v); }
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
