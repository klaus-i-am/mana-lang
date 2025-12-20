#include "E2ETestRunner.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    std::cout << "\n=== Mana End-to-End Test Suite ===\n";

    // Configure the test runner
    mana::e2e::E2ETestRunner::Config config;

#ifdef _WIN32
    // Default paths for Windows
    config.mana_compiler = "build\\Release\\mana.exe";
    config.cpp_compiler = "cl";
    config.runtime_header = "backend-cpp\\mana_runtime.h";
    config.output_dir = "build\\e2e_temp";
#else
    // Default paths for Unix
    config.mana_compiler = "build/mana";
    config.cpp_compiler = "g++";
    config.runtime_header = "backend-cpp/mana_runtime.h";
    config.output_dir = "build/e2e_temp";
#endif

    // Parse command line arguments
    std::string test_dir = "tests/e2e/tests";
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-v" || arg == "--verbose") {
            config.verbose = true;
        } else if (arg == "-k" || arg == "--keep") {
            config.keep_artifacts = true;
        } else if (arg == "--compiler" && i + 1 < argc) {
            config.mana_compiler = argv[++i];
        } else if (arg == "--cpp" && i + 1 < argc) {
            config.cpp_compiler = argv[++i];
        } else if (arg == "--dir" && i + 1 < argc) {
            test_dir = argv[++i];
        } else if (arg == "-h" || arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -v, --verbose       Show detailed output\n";
            std::cout << "  -k, --keep          Keep compiled artifacts\n";
            std::cout << "  --compiler <path>   Path to mana compiler\n";
            std::cout << "  --cpp <compiler>    C++ compiler to use\n";
            std::cout << "  --dir <path>        Test directory\n";
            std::cout << "  -h, --help          Show this help\n";
            return 0;
        } else if (arg[0] != '-') {
            test_dir = arg;
        }
    }

    // Create output directory if it doesn't exist
    std::filesystem::create_directories(config.output_dir);

    // Run tests
    mana::e2e::E2ETestRunner runner(config);
    auto summary = runner.run_all(test_dir);

    return summary.failed > 0 ? 1 : 0;
}
