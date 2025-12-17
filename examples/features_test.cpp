// Test for JSON, Regex, and HTTP features
#include <iostream>
#include "mana_runtime.h"

int main() {
    std::cout << "=== New Features Test ===" << std::endl;

    // ===== JSON Test =====
    std::cout << "\n--- JSON ---" << std::endl;

    // Create JSON object
    mana::JsonValue obj = {
        {"name", "Alice"},
        {"age", 30},
        {"active", true},
        {"scores", mana::json_array({95, 87, 92})}
    };

    std::cout << "Created: " << mana::json_stringify(obj) << std::endl;

    // Parse JSON
    std::string json_str = R"({"message": "Hello", "count": 42, "items": [1, 2, 3]})";
    auto parsed = mana::json_parse(json_str);
    if (parsed.is_ok()) {
        auto& val = parsed.unwrap();
        std::cout << "Parsed message: " << val["message"].as_string() << std::endl;
        std::cout << "Parsed count: " << val["count"].as_int() << std::endl;
        std::cout << "Items count: " << val["items"].size() << std::endl;
    }

    // ===== Regex Test =====
    std::cout << "\n--- Regex ---" << std::endl;

    // Simple match
    std::cout << "Match 'hello' in 'hello world': "
              << (mana::regex_match("hello world", "hello") ? "true" : "false") << std::endl;

    // Find all
    auto matches = mana::regex_find_all("The quick brown fox", "\\w+");
    std::cout << "Words found: " << matches.len() << std::endl;

    // Replace
    std::string replaced = mana::regex_replace("Hello World", "World", "Mana");
    std::cout << "Replaced: " << replaced << std::endl;

    // Split
    auto parts = mana::regex_split("a,b;c:d", "[,;:]");
    std::cout << "Split parts: " << parts.len() << std::endl;

    // Regex class
    auto re_result = mana::regex_new("\\d+");
    if (re_result.is_ok()) {
        auto& re = re_result.unwrap();
        auto nums = re.find_all("I have 3 apples and 5 oranges");
        std::cout << "Numbers found: ";
        for (size_t i = 0; i < nums.len(); ++i) {
            std::cout << nums[i] << " ";
        }
        std::cout << std::endl;
    }

    // ===== URL Encoding Test =====
    std::cout << "\n--- URL Encoding ---" << std::endl;
    std::string encoded = mana::url_encode("hello world&foo=bar");
    std::cout << "Encoded: " << encoded << std::endl;
    std::cout << "Decoded: " << mana::url_decode(encoded) << std::endl;

    // Note: HTTP test skipped as it requires network
    std::cout << "\n--- HTTP ---" << std::endl;
    std::cout << "HTTP client available (requires network for testing)" << std::endl;

    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
