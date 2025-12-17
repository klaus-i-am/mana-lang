#pragma once
#include <ctime>
#include <cstdlib>
#include <regex>
#include <map>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>

namespace mana {

// ========== DateTime ==========

struct DateTime {
    int year, month, day, hour, minute, second;

    static DateTime now() {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        return { tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec };
    }

    static DateTime from_timestamp(int64_t ts) {
        auto t = static_cast<std::time_t>(ts);
        auto tm = *std::localtime(&t);
        return { tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                 tm.tm_hour, tm.tm_min, tm.tm_sec };
    }

    int64_t timestamp() const {
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        return static_cast<int64_t>(std::mktime(&tm));
    }

    std::string format(const std::string& fmt) const {
        char buf[256];
        std::tm tm = {};
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        std::strftime(buf, sizeof(buf), fmt.c_str(), &tm);
        return std::string(buf);
    }

    std::string to_string() const { return format("%Y-%m-%d %H:%M:%S"); }
    std::string to_iso8601() const { return format("%Y-%m-%dT%H:%M:%S"); }
    std::string to_date_string() const { return format("%Y-%m-%d"); }
    std::string to_time_string() const { return format("%H:%M:%S"); }
};

inline DateTime datetime_now() { return DateTime::now(); }
inline DateTime datetime_from_timestamp(int64_t ts) { return DateTime::from_timestamp(ts); }

// ========== Regex ==========

class Regex {
    std::regex pattern_;
    std::string pattern_str_;
public:
    explicit Regex(const std::string& pattern) : pattern_(pattern), pattern_str_(pattern) {}

    bool matches(const std::string& text) const {
        return std::regex_match(text, pattern_);
    }

    bool contains(const std::string& text) const {
        return std::regex_search(text, pattern_);
    }

    Option<std::string> find(const std::string& text) const {
        std::smatch match;
        if (std::regex_search(text, match, pattern_)) {
            return Option<std::string>(match.str());
        }
        return Option<std::string>();
    }

    Vec<std::string> find_all(const std::string& text) const {
        Vec<std::string> results;
        std::sregex_iterator it(text.begin(), text.end(), pattern_);
        std::sregex_iterator end;
        for (; it != end; ++it) {
            results.push(it->str());
        }
        return results;
    }

    std::string replace_first(const std::string& text, const std::string& replacement) const {
        return std::regex_replace(text, pattern_, replacement, std::regex_constants::format_first_only);
    }

    std::string replace_all(const std::string& text, const std::string& replacement) const {
        return std::regex_replace(text, pattern_, replacement);
    }

    Vec<std::string> split(const std::string& text) const {
        Vec<std::string> results;
        std::sregex_token_iterator it(text.begin(), text.end(), pattern_, -1);
        std::sregex_token_iterator end;
        for (; it != end; ++it) {
            results.push(*it);
        }
        return results;
    }
};

inline Regex regex(const std::string& pattern) { return Regex(pattern); }

// ========== JSON ==========

class JsonValue;
using JsonObject = std::map<std::string, JsonValue>;
using JsonArray = std::vector<JsonValue>;

class JsonValue {
public:
    enum Type { Null, Bool, Number, String, Array, Object };
private:
    Type type_ = Null;
    bool bool_val_ = false;
    double num_val_ = 0.0;
    std::string str_val_;
    JsonArray arr_val_;
    JsonObject obj_val_;
public:
    JsonValue() : type_(Null) {}
    JsonValue(std::nullptr_t) : type_(Null) {}
    JsonValue(bool v) : type_(Bool), bool_val_(v) {}
    JsonValue(int v) : type_(Number), num_val_(v) {}
    JsonValue(int64_t v) : type_(Number), num_val_(static_cast<double>(v)) {}
    JsonValue(double v) : type_(Number), num_val_(v) {}
    JsonValue(const char* v) : type_(String), str_val_(v) {}
    JsonValue(const std::string& v) : type_(String), str_val_(v) {}
    JsonValue(const JsonArray& v) : type_(Array), arr_val_(v) {}
    JsonValue(const JsonObject& v) : type_(Object), obj_val_(v) {}

    Type type() const { return type_; }
    bool is_null() const { return type_ == Null; }
    bool is_bool() const { return type_ == Bool; }
    bool is_number() const { return type_ == Number; }
    bool is_string() const { return type_ == String; }
    bool is_array() const { return type_ == Array; }
    bool is_object() const { return type_ == Object; }

    bool as_bool() const { return bool_val_; }
    double as_number() const { return num_val_; }
    int32_t as_int() const { return static_cast<int32_t>(num_val_); }
    const std::string& as_string() const { return str_val_; }
    const JsonArray& as_array() const { return arr_val_; }
    const JsonObject& as_object() const { return obj_val_; }
    JsonArray& as_array_mut() { return arr_val_; }
    JsonObject& as_object_mut() { return obj_val_; }

    JsonValue& operator[](size_t idx) { return arr_val_[idx]; }
    const JsonValue& operator[](size_t idx) const { return arr_val_[idx]; }
    JsonValue& operator[](const std::string& key) { return obj_val_[key]; }

    bool has(const std::string& key) const {
        return type_ == Object && obj_val_.find(key) != obj_val_.end();
    }

    Option<JsonValue> get(const std::string& key) const {
        if (type_ != Object) return Option<JsonValue>();
        auto it = obj_val_.find(key);
        if (it == obj_val_.end()) return Option<JsonValue>();
        return Option<JsonValue>(it->second);
    }

    size_t size() const {
        if (type_ == Array) return arr_val_.size();
        if (type_ == Object) return obj_val_.size();
        return 0;
    }

    std::string to_string() const {
        std::ostringstream oss;
        write_json(oss);
        return oss.str();
    }

private:
    void write_json(std::ostream& os) const {
        switch (type_) {
            case Null: os << "null"; break;
            case Bool: os << (bool_val_ ? "true" : "false"); break;
            case Number:
                if (num_val_ == static_cast<int64_t>(num_val_))
                    os << static_cast<int64_t>(num_val_);
                else
                    os << num_val_;
                break;
            case String:
                os << '"';
                for (char c : str_val_) {
                    switch (c) {
                        case '"': os << "\\\""; break;
                        case '\\': os << "\\\\"; break;
                        case '\n': os << "\\n"; break;
                        case '\r': os << "\\r"; break;
                        case '\t': os << "\\t"; break;
                        default: os << c;
                    }
                }
                os << '"';
                break;
            case Array:
                os << '[';
                for (size_t i = 0; i < arr_val_.size(); ++i) {
                    if (i > 0) os << ',';
                    arr_val_[i].write_json(os);
                }
                os << ']';
                break;
            case Object: {
                os << '{';
                bool first = true;
                for (const auto& kv : obj_val_) {
                    if (!first) os << ',';
                    first = false;
                    os << '"' << kv.first << "\":";
                    kv.second.write_json(os);
                }
                os << '}';
                break;
            }
        }
    }
};

class JsonParser {
    const std::string& src_;
    size_t pos_ = 0;

    void skip_ws() { while (pos_ < src_.size() && std::isspace(src_[pos_])) pos_++; }
    char peek() const { return pos_ < src_.size() ? src_[pos_] : '\0'; }
    char advance() { return src_[pos_++]; }

    JsonValue parse_value() {
        skip_ws();
        char c = peek();
        if (c == 'n') return parse_null();
        if (c == 't' || c == 'f') return parse_bool();
        if (c == '"') return parse_string();
        if (c == '[') return parse_array();
        if (c == '{') return parse_object();
        if (c == '-' || std::isdigit(c)) return parse_number();
        throw std::runtime_error("Invalid JSON");
    }

    JsonValue parse_null() {
        if (src_.substr(pos_, 4) == "null") { pos_ += 4; return JsonValue(); }
        throw std::runtime_error("Invalid JSON null");
    }

    JsonValue parse_bool() {
        if (src_.substr(pos_, 4) == "true") { pos_ += 4; return JsonValue(true); }
        if (src_.substr(pos_, 5) == "false") { pos_ += 5; return JsonValue(false); }
        throw std::runtime_error("Invalid JSON bool");
    }

    JsonValue parse_number() {
        size_t start = pos_;
        if (peek() == '-') pos_++;
        while (std::isdigit(peek())) pos_++;
        if (peek() == '.') { pos_++; while (std::isdigit(peek())) pos_++; }
        if (peek() == 'e' || peek() == 'E') {
            pos_++;
            if (peek() == '+' || peek() == '-') pos_++;
            while (std::isdigit(peek())) pos_++;
        }
        return JsonValue(std::stod(src_.substr(start, pos_ - start)));
    }

    JsonValue parse_string() {
        advance();
        std::string result;
        while (peek() != '"') {
            if (peek() == '\\') {
                advance();
                switch (advance()) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: throw std::runtime_error("Invalid escape");
                }
            } else {
                result += advance();
            }
        }
        advance();
        return JsonValue(result);
    }

    JsonValue parse_array() {
        advance();
        JsonArray arr;
        skip_ws();
        if (peek() != ']') {
            arr.push_back(parse_value());
            while (skip_ws(), peek() == ',') { advance(); arr.push_back(parse_value()); }
        }
        skip_ws(); advance();
        return JsonValue(arr);
    }

    JsonValue parse_object() {
        advance();
        JsonObject obj;
        skip_ws();
        if (peek() != '}') {
            auto key = parse_string().as_string();
            skip_ws(); advance();
            obj[key] = parse_value();
            while (skip_ws(), peek() == ',') {
                advance(); skip_ws();
                key = parse_string().as_string();
                skip_ws(); advance();
                obj[key] = parse_value();
            }
        }
        skip_ws(); advance();
        return JsonValue(obj);
    }

public:
    explicit JsonParser(const std::string& src) : src_(src) {}
    JsonValue parse() { return parse_value(); }
};

inline Result<JsonValue, std::string> json_parse(const std::string& src) {
    try {
        JsonParser parser(src);
        return Result<JsonValue, std::string>::Ok(parser.parse());
    } catch (const std::exception& e) {
        return Result<JsonValue, std::string>::Err(std::string(e.what()));
    }
}

inline std::string json_stringify(const JsonValue& val) { return val.to_string(); }
inline JsonValue json_object() { return JsonValue(JsonObject{}); }
inline JsonValue json_array() { return JsonValue(JsonArray{}); }

// ========== Environment ==========

inline Option<std::string> env_get(const std::string& name) {
    const char* val = std::getenv(name.c_str());
    if (val) return Option<std::string>(std::string(val));
    return Option<std::string>();
}

// ========== File System ==========

inline bool file_exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

inline Result<std::string, std::string> file_read(const std::string& path) {
    std::ifstream f(path);
    if (!f) return Result<std::string, std::string>::Err("Cannot open file: " + path);
    std::stringstream buf;
    buf << f.rdbuf();
    return Result<std::string, std::string>::Ok(buf.str());
}

inline Result<bool, std::string> file_write(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    if (!f) return Result<bool, std::string>::Err("Cannot write file: " + path);
    f << content;
    return Result<bool, std::string>::Ok(true);
}

inline Result<bool, std::string> file_append(const std::string& path, const std::string& content) {
    std::ofstream f(path, std::ios::app);
    if (!f) return Result<bool, std::string>::Err("Cannot append to file: " + path);
    f << content;
    return Result<bool, std::string>::Ok(true);
}

inline Result<Vec<std::string>, std::string> file_read_lines(const std::string& path) {
    std::ifstream f(path);
    if (!f) return Result<Vec<std::string>, std::string>::Err("Cannot open file: " + path);
    Vec<std::string> lines;
    std::string line;
    while (std::getline(f, line)) lines.push(line);
    return Result<Vec<std::string>, std::string>::Ok(lines);
}

// ========== Random ==========

inline int32_t random_int(int32_t min_val, int32_t max_val) {
    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); seeded = true; }
    return min_val + std::rand() % (max_val - min_val + 1);
}

inline float random_float() {
    static bool seeded = false;
    if (!seeded) { std::srand(static_cast<unsigned>(std::time(nullptr))); seeded = true; }
    return static_cast<float>(std::rand()) / RAND_MAX;
}

// ========== Sleep ==========

inline void sleep_ms(int64_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void sleep_secs(int64_t secs) {
    std::this_thread::sleep_for(std::chrono::seconds(secs));
}

} // namespace mana
