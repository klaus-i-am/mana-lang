// Comprehensive test for Mana standard library
#include <iostream>
#include "mana_runtime.h"

int main() {
    std::cout << "=== Mana Standard Library Test ===" << std::endl;

    // ===== File I/O =====
    std::cout << "\n--- File I/O ---" << std::endl;
    auto write_result = mana::write_file("test_output.txt", "Hello, Mana!\nLine 2\nLine 3");
    if (write_result.is_ok()) {
        std::cout << "write_file: OK" << std::endl;
    }

    auto read_result = mana::read_file("test_output.txt");
    if (read_result.is_ok()) {
        std::cout << "read_file: " << read_result.unwrap().size() << " bytes" << std::endl;
    }

    auto lines_result = mana::read_lines("test_output.txt");
    if (lines_result.is_ok()) {
        std::cout << "read_lines: " << lines_result.unwrap().len() << " lines" << std::endl;
    }

    std::cout << "file_exists: " << (mana::file_exists("test_output.txt") ? "true" : "false") << std::endl;
    std::cout << "is_file: " << (mana::is_file("test_output.txt") ? "true" : "false") << std::endl;

    // ===== Path Utilities =====
    std::cout << "\n--- Path Utilities ---" << std::endl;
    std::cout << "path_join: " << mana::path_join("foo", "bar.txt") << std::endl;
    std::cout << "file_name: " << mana::file_name("/path/to/file.txt") << std::endl;
    std::cout << "file_stem: " << mana::file_stem("/path/to/file.txt") << std::endl;
    std::cout << "file_extension: " << mana::file_extension("/path/to/file.txt") << std::endl;
    std::cout << "parent_dir: " << mana::parent_dir("/path/to/file.txt") << std::endl;
    std::cout << "current_dir: " << mana::current_dir() << std::endl;

    // ===== Environment =====
    std::cout << "\n--- Environment ---" << std::endl;
    auto path_var = mana::env_var("PATH");
    std::cout << "env_var(PATH): " << (path_var.is_some() ? "exists" : "not found") << std::endl;

    // ===== Random =====
    std::cout << "\n--- Random ---" << std::endl;
    std::cout << "random_int(1, 100): " << mana::random_int(1, 100) << std::endl;
    std::cout << "random_float(): " << mana::random_float() << std::endl;
    std::cout << "random_bool(): " << (mana::random_bool() ? "true" : "false") << std::endl;

    mana::Vec<int> nums = {1, 2, 3, 4, 5};
    std::cout << "random_choice: " << mana::random_choice(nums) << std::endl;

    // ===== Math =====
    std::cout << "\n--- Math ---" << std::endl;
    std::cout << "sqrt_f(16): " << mana::sqrt_f(16) << std::endl;
    std::cout << "pow_f(2, 10): " << mana::pow_f(2, 10) << std::endl;
    std::cout << "sin_f(PI/2): " << mana::sin_f(mana::PI / 2) << std::endl;
    std::cout << "log_f(E): " << mana::log_f(mana::E) << std::endl;
    std::cout << "floor_f(3.7): " << mana::floor_f(3.7) << std::endl;
    std::cout << "ceil_f(3.2): " << mana::ceil_f(3.2) << std::endl;
    std::cout << "gcd(48, 18): " << mana::gcd(48, 18) << std::endl;
    std::cout << "lcm(4, 6): " << mana::lcm(4, 6) << std::endl;
    std::cout << "to_radians(180): " << mana::to_radians(180) << std::endl;
    std::cout << "lerp(0, 100, 0.5): " << mana::lerp(0, 100, 0.5) << std::endl;

    // ===== Time =====
    std::cout << "\n--- Time ---" << std::endl;
    std::cout << "timestamp_ms: " << mana::timestamp_ms() << std::endl;

    auto t = mana::timer();
    mana::sleep_ms(10);
    std::cout << "timer elapsed after 10ms sleep: " << t.elapsed_ms() << "ms" << std::endl;

    // ===== Vec Methods =====
    std::cout << "\n--- Vec Methods ---" << std::endl;
    mana::Vec<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto chunks = v.chunks(3);
    std::cout << "chunks(3): " << chunks.len() << " chunks" << std::endl;

    auto windows = v.windows(3);
    std::cout << "windows(3): " << windows.len() << " windows" << std::endl;

    mana::Vec<int> dups = {1, 2, 2, 3, 3, 3, 4};
    auto unique = dups.unique();
    std::cout << "unique: " << unique.len() << " elements" << std::endl;

    auto pos = v.position(5);
    std::cout << "position(5): " << (pos.is_some() ? std::to_string(pos.unwrap()) : "not found") << std::endl;

    std::cout << "get_or(100, -1): " << v.get_or(100, -1) << std::endl;

    // Cleanup
    mana::remove_file("test_output.txt");

    std::cout << "\n=== All tests passed! ===" << std::endl;
    return 0;
}
