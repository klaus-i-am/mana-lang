// Test for new string methods
#include <iostream>
#include "mana_runtime.h"

int main() {
    std::cout << "=== String Methods Test ===" << std::endl;

    // trim_start / trim_end
    std::string s1 = "   hello   ";
    std::cout << "trim_start: [" << mana::trim_start(s1) << "]" << std::endl;
    std::cout << "trim_end: [" << mana::trim_end(s1) << "]" << std::endl;
    std::cout << "trim: [" << mana::trim(s1) << "]" << std::endl;

    // pad_start / pad_end
    std::string s2 = "42";
    std::cout << "pad_start(5): [" << mana::pad_start(s2, 5) << "]" << std::endl;
    std::cout << "pad_start(5, '0'): [" << mana::pad_start(s2, 5, '0') << "]" << std::endl;
    std::cout << "pad_end(5): [" << mana::pad_end(s2, 5) << "]" << std::endl;

    // char_at
    std::string s3 = "hello";
    auto ch = mana::char_at(s3, 1);
    if (ch.is_some()) {
        std::cout << "char_at(1): " << ch.unwrap() << std::endl;
    }
    auto ch_out = mana::char_at(s3, 10);
    std::cout << "char_at(10) is_none: " << (ch_out.is_none() ? "true" : "false") << std::endl;

    // find_index / rfind_index
    std::string s4 = "hello world hello";
    auto idx1 = mana::find_index(s4, "hello");
    auto idx2 = mana::rfind_index(s4, "hello");
    if (idx1.is_some()) std::cout << "find_index('hello'): " << idx1.unwrap() << std::endl;
    if (idx2.is_some()) std::cout << "rfind_index('hello'): " << idx2.unwrap() << std::endl;

    // replace_first
    std::cout << "replace_first: " << mana::replace_first(s4, "hello", "hi") << std::endl;

    // is_digit / is_alpha / is_alphanumeric / is_whitespace / is_numeric
    std::cout << "is_digit('123'): " << (mana::is_digit("123") ? "true" : "false") << std::endl;
    std::cout << "is_digit('12a'): " << (mana::is_digit("12a") ? "true" : "false") << std::endl;
    std::cout << "is_alpha('abc'): " << (mana::is_alpha("abc") ? "true" : "false") << std::endl;
    std::cout << "is_alphanumeric('abc123'): " << (mana::is_alphanumeric("abc123") ? "true" : "false") << std::endl;
    std::cout << "is_whitespace('   '): " << (mana::is_whitespace("   ") ? "true" : "false") << std::endl;
    std::cout << "is_numeric('-3.14'): " << (mana::is_numeric("-3.14") ? "true" : "false") << std::endl;
    std::cout << "is_numeric('1.2.3'): " << (mana::is_numeric("1.2.3") ? "true" : "false") << std::endl;

    // chars
    auto ch_vec = mana::chars("hi");
    std::cout << "chars('hi'): ";
    for (size_t i = 0; i < ch_vec.len(); ++i) {
        std::cout << ch_vec[i] << " ";
    }
    std::cout << std::endl;

    // lines
    std::string multiline = "line1\nline2\nline3";
    auto line_vec = mana::lines(multiline);
    std::cout << "lines (count): " << line_vec.len() << std::endl;
    for (size_t i = 0; i < line_vec.len(); ++i) {
        std::cout << "  line " << i << ": " << line_vec[i] << std::endl;
    }

    // capitalize
    std::cout << "capitalize('hello'): " << mana::capitalize("hello") << std::endl;

    std::cout << std::endl << "All tests passed!" << std::endl;
    return 0;
}
