#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <regex>
#include <chrono>
#include <cstdlib>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace mana::e2e {

// Test expectation types
enum class ExpectType {
    Output,         // expect: <output>
    OutputContains, // expect-contains: <substring>
    ExitCode,       // expect-exit: <code>
    Error,          // expect-error: <message>
    CompileError,   // expect-compile-error: <message>
    Timeout         // expect-timeout: <seconds>
};

struct Expectation {
    ExpectType type;
    std::string value;
    int line;
};

struct E2ETestResult {
    std::string name;
    std::string file;
    bool passed;
    std::string message;
    std::string actual_output;
    std::string expected_output;
    int exit_code;
    std::chrono::milliseconds duration;
};

struct E2ETestSummary {
    int total;
    int passed;
    int failed;
    int skipped;
    std::chrono::milliseconds total_duration;
    std::vector<E2ETestResult> results;
};

class E2ETestRunner {
public:
    struct Config {
        std::string mana_compiler;      // Path to mana.exe
        std::string cpp_compiler;       // Path to C++ compiler (cl, g++, clang++)
        std::string runtime_header;     // Path to mana_runtime.h
        std::string output_dir;         // Directory for compiled files
        int timeout_seconds = 30;       // Default timeout
        bool verbose = false;           // Verbose output
        bool keep_artifacts = false;    // Keep compiled files after test
    };

    explicit E2ETestRunner(const Config& config) : config_(config) {}

    // Parse expectations from a test file
    std::vector<Expectation> parse_expectations(const std::string& file_path) {
        std::vector<Expectation> expectations;
        std::ifstream in(file_path);
        if (!in) return expectations;

        std::string line;
        int line_num = 0;
        while (std::getline(in, line)) {
            line_num++;

            // Look for // expect: patterns
            size_t comment_pos = line.find("//");
            if (comment_pos == std::string::npos) continue;

            std::string comment = line.substr(comment_pos + 2);
            // Trim leading whitespace
            size_t start = comment.find_first_not_of(" \t");
            if (start == std::string::npos) continue;
            comment = comment.substr(start);

            Expectation exp;
            exp.line = line_num;

            if (comment.rfind("expect:", 0) == 0) {
                exp.type = ExpectType::Output;
                exp.value = comment.substr(7);
                // Trim leading space
                if (!exp.value.empty() && exp.value[0] == ' ') {
                    exp.value = exp.value.substr(1);
                }
                expectations.push_back(exp);
            }
            else if (comment.rfind("expect-contains:", 0) == 0) {
                exp.type = ExpectType::OutputContains;
                exp.value = comment.substr(16);
                if (!exp.value.empty() && exp.value[0] == ' ') {
                    exp.value = exp.value.substr(1);
                }
                expectations.push_back(exp);
            }
            else if (comment.rfind("expect-exit:", 0) == 0) {
                exp.type = ExpectType::ExitCode;
                exp.value = comment.substr(12);
                if (!exp.value.empty() && exp.value[0] == ' ') {
                    exp.value = exp.value.substr(1);
                }
                expectations.push_back(exp);
            }
            else if (comment.rfind("expect-error:", 0) == 0) {
                exp.type = ExpectType::Error;
                exp.value = comment.substr(13);
                if (!exp.value.empty() && exp.value[0] == ' ') {
                    exp.value = exp.value.substr(1);
                }
                expectations.push_back(exp);
            }
            else if (comment.rfind("expect-compile-error:", 0) == 0) {
                exp.type = ExpectType::CompileError;
                exp.value = comment.substr(21);
                if (!exp.value.empty() && exp.value[0] == ' ') {
                    exp.value = exp.value.substr(1);
                }
                expectations.push_back(exp);
            }
        }

        return expectations;
    }

    // Build expected output from Output expectations
    std::string build_expected_output(const std::vector<Expectation>& expectations) {
        std::string output;
        for (const auto& exp : expectations) {
            if (exp.type == ExpectType::Output) {
                if (!output.empty()) output += "\n";
                output += exp.value;
            }
        }
        return output;
    }

    // Run a command and capture output
    struct CommandResult {
        int exit_code;
        std::string stdout_output;
        std::string stderr_output;
        bool timed_out;
    };

    CommandResult run_command(const std::string& cmd, int timeout_sec = 30) {
        CommandResult result;
        result.exit_code = -1;
        result.timed_out = false;

#ifdef _WIN32
        // Create pipes for stdout and stderr
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        HANDLE stdout_read, stdout_write;
        HANDLE stderr_read, stderr_write;

        if (!CreatePipe(&stdout_read, &stdout_write, &sa, 0) ||
            !CreatePipe(&stderr_read, &stderr_write, &sa, 0)) {
            result.stderr_output = "Failed to create pipes";
            return result;
        }

        SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0);
        SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.hStdOutput = stdout_write;
        si.hStdError = stderr_write;
        si.dwFlags |= STARTF_USESTDHANDLES;
        ZeroMemory(&pi, sizeof(pi));

        std::string cmd_copy = cmd;
        if (!CreateProcessA(NULL, &cmd_copy[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            result.stderr_output = "Failed to create process";
            CloseHandle(stdout_read);
            CloseHandle(stdout_write);
            CloseHandle(stderr_read);
            CloseHandle(stderr_write);
            return result;
        }

        CloseHandle(stdout_write);
        CloseHandle(stderr_write);

        // Wait for process with timeout
        DWORD wait_result = WaitForSingleObject(pi.hProcess, timeout_sec * 1000);
        if (wait_result == WAIT_TIMEOUT) {
            TerminateProcess(pi.hProcess, 1);
            result.timed_out = true;
        }

        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        result.exit_code = static_cast<int>(exit_code);

        // Read stdout
        char buffer[4096];
        DWORD bytes_read;
        while (ReadFile(stdout_read, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {
            buffer[bytes_read] = '\0';
            result.stdout_output += buffer;
        }

        // Read stderr
        while (ReadFile(stderr_read, buffer, sizeof(buffer) - 1, &bytes_read, NULL) && bytes_read > 0) {
            buffer[bytes_read] = '\0';
            result.stderr_output += buffer;
        }

        CloseHandle(stdout_read);
        CloseHandle(stderr_read);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
#else
        // Unix implementation using popen
        FILE* pipe = popen((cmd + " 2>&1").c_str(), "r");
        if (!pipe) {
            result.stderr_output = "Failed to run command";
            return result;
        }

        char buffer[4096];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result.stdout_output += buffer;
        }

        result.exit_code = WEXITSTATUS(pclose(pipe));
#endif

        return result;
    }

    // Normalize line endings and trim whitespace
    std::string normalize_output(const std::string& output) {
        std::string result = output;
        // Replace \r\n with \n
        size_t pos = 0;
        while ((pos = result.find("\r\n", pos)) != std::string::npos) {
            result.replace(pos, 2, "\n");
        }
        // Remove trailing whitespace and newlines
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
            result.pop_back();
        }
        return result;
    }

    // Run a single E2E test
    E2ETestResult run_test(const std::string& test_file) {
        E2ETestResult result;
        result.passed = false;
        result.exit_code = -1;

        auto start = std::chrono::high_resolution_clock::now();

        // Extract test name from file path
        std::filesystem::path p(test_file);
        result.name = p.stem().string();
        result.file = test_file;

        // Parse expectations
        auto expectations = parse_expectations(test_file);
        if (expectations.empty()) {
            result.message = "No expectations found in test file";
            result.passed = false;
            auto end = std::chrono::high_resolution_clock::now();
            result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return result;
        }

        result.expected_output = build_expected_output(expectations);

        // Check for expected compile error
        bool expect_compile_error = false;
        std::string expected_compile_error_msg;
        for (const auto& exp : expectations) {
            if (exp.type == ExpectType::CompileError) {
                expect_compile_error = true;
                expected_compile_error_msg = exp.value;
                break;
            }
        }

        // Step 1: Compile Mana to C++
        std::string base_name = result.name;
        std::string cpp_file = config_.output_dir + "/" + base_name + ".cpp";
        std::string exe_file = config_.output_dir + "/" + base_name;
#ifdef _WIN32
        exe_file += ".exe";
#endif

        std::string mana_cmd = config_.mana_compiler + " \"" + test_file + "\" --emit-cpp -o \"" + cpp_file + "\"";
        if (config_.verbose) {
            std::cout << "  [MANA] " << mana_cmd << "\n";
        }

        auto mana_result = run_command(mana_cmd, config_.timeout_seconds);

        if (mana_result.exit_code != 0) {
            if (expect_compile_error) {
                // Check if the error message matches
                if (expected_compile_error_msg.empty() ||
                    mana_result.stderr_output.find(expected_compile_error_msg) != std::string::npos ||
                    mana_result.stdout_output.find(expected_compile_error_msg) != std::string::npos) {
                    result.passed = true;
                    result.message = "Compile error as expected";
                } else {
                    result.passed = false;
                    result.message = "Compile error but message didn't match";
                    result.actual_output = mana_result.stderr_output + mana_result.stdout_output;
                }
            } else {
                result.message = "Mana compilation failed: " + mana_result.stderr_output;
                result.actual_output = mana_result.stdout_output + mana_result.stderr_output;
            }
            auto end = std::chrono::high_resolution_clock::now();
            result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return result;
        }

        if (expect_compile_error) {
            result.passed = false;
            result.message = "Expected compile error but compilation succeeded";
            auto end = std::chrono::high_resolution_clock::now();
            result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return result;
        }

        // Step 2: Compile C++ to executable
        std::string cpp_cmd;
#ifdef _WIN32
        // Use MSVC cl.exe
        cpp_cmd = config_.cpp_compiler + " /nologo /EHsc /std:c++17 /Fe\"" + exe_file + "\" \"" + cpp_file + "\" /I\"" + std::filesystem::path(config_.runtime_header).parent_path().string() + "\"";
#else
        // Use g++ or clang++
        cpp_cmd = config_.cpp_compiler + " -std=c++17 -o \"" + exe_file + "\" \"" + cpp_file + "\" -I\"" + std::filesystem::path(config_.runtime_header).parent_path().string() + "\"";
#endif

        if (config_.verbose) {
            std::cout << "  [C++] " << cpp_cmd << "\n";
        }

        auto cpp_result = run_command(cpp_cmd, config_.timeout_seconds);
        if (cpp_result.exit_code != 0) {
            result.message = "C++ compilation failed: " + cpp_result.stderr_output;
            result.actual_output = cpp_result.stdout_output + cpp_result.stderr_output;
            auto end = std::chrono::high_resolution_clock::now();
            result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            return result;
        }

        // Step 3: Run the executable
        std::string run_cmd = "\"" + exe_file + "\"";
        if (config_.verbose) {
            std::cout << "  [RUN] " << run_cmd << "\n";
        }

        auto run_result = run_command(run_cmd, config_.timeout_seconds);
        result.exit_code = run_result.exit_code;
        result.actual_output = normalize_output(run_result.stdout_output);

        // Step 4: Verify expectations
        bool all_passed = true;
        std::vector<std::string> failures;

        for (const auto& exp : expectations) {
            switch (exp.type) {
                case ExpectType::Output: {
                    // Already handled via build_expected_output
                    break;
                }
                case ExpectType::OutputContains: {
                    if (result.actual_output.find(exp.value) == std::string::npos) {
                        all_passed = false;
                        failures.push_back("Output does not contain: " + exp.value);
                    }
                    break;
                }
                case ExpectType::ExitCode: {
                    int expected_exit = std::stoi(exp.value);
                    if (result.exit_code != expected_exit) {
                        all_passed = false;
                        failures.push_back("Exit code " + std::to_string(result.exit_code) + " != expected " + exp.value);
                    }
                    break;
                }
                case ExpectType::Error: {
                    if (result.actual_output.find(exp.value) == std::string::npos &&
                        run_result.stderr_output.find(exp.value) == std::string::npos) {
                        all_passed = false;
                        failures.push_back("Error message not found: " + exp.value);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        // Check full output match if we have Output expectations
        if (!result.expected_output.empty()) {
            std::string normalized_expected = normalize_output(result.expected_output);
            if (result.actual_output != normalized_expected) {
                all_passed = false;
                failures.push_back("Output mismatch");
            }
        }

        result.passed = all_passed;
        if (!failures.empty()) {
            result.message = failures[0];
            for (size_t i = 1; i < failures.size(); i++) {
                result.message += "; " + failures[i];
            }
        }

        // Cleanup
        if (!config_.keep_artifacts) {
            std::filesystem::remove(cpp_file);
            std::filesystem::remove(exe_file);
#ifdef _WIN32
            // Also remove .obj file
            std::filesystem::remove(config_.output_dir + "/" + base_name + ".obj");
#endif
        }

        auto end = std::chrono::high_resolution_clock::now();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        return result;
    }

    // Run all tests in a directory
    E2ETestSummary run_all(const std::string& test_dir) {
        E2ETestSummary summary;
        summary.total = 0;
        summary.passed = 0;
        summary.failed = 0;
        summary.skipped = 0;

        auto start = std::chrono::high_resolution_clock::now();

        // Find all .mana files in test directory
        std::vector<std::string> test_files;
        for (const auto& entry : std::filesystem::directory_iterator(test_dir)) {
            if (entry.path().extension() == ".mana") {
                test_files.push_back(entry.path().string());
            }
        }

        // Sort for consistent ordering
        std::sort(test_files.begin(), test_files.end());

        std::cout << "\nRunning " << test_files.size() << " end-to-end tests\n\n";

        for (const auto& test_file : test_files) {
            summary.total++;
            auto result = run_test(test_file);
            summary.results.push_back(result);

            // Print result
            std::string status;
#ifdef _WIN32
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            if (result.passed) {
                SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
                status = "PASS";
                summary.passed++;
            } else {
                SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
                status = "FAIL";
                summary.failed++;
            }
            std::cout << "[" << status << "]";
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
            if (result.passed) {
                status = "\033[32mPASS\033[0m";
                summary.passed++;
            } else {
                status = "\033[31mFAIL\033[0m";
                summary.failed++;
            }
            std::cout << "[" << status << "]";
#endif
            std::cout << " " << result.name << " (" << result.duration.count() << "ms)\n";

            if (!result.passed && !result.message.empty()) {
                std::cout << "       " << result.message << "\n";
                if (config_.verbose && !result.actual_output.empty()) {
                    std::cout << "       Actual output:\n";
                    std::istringstream iss(result.actual_output);
                    std::string line;
                    while (std::getline(iss, line)) {
                        std::cout << "         " << line << "\n";
                    }
                    if (!result.expected_output.empty()) {
                        std::cout << "       Expected output:\n";
                        std::istringstream ess(result.expected_output);
                        while (std::getline(ess, line)) {
                            std::cout << "         " << line << "\n";
                        }
                    }
                }
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        summary.total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // Print summary
        std::cout << "\n";
        std::cout << "E2E Test Results: ";
        if (summary.failed == 0) {
#ifdef _WIN32
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "All tests passed!";
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
            std::cout << "\033[32;1mAll tests passed!\033[0m";
#endif
        } else {
#ifdef _WIN32
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
            std::cout << summary.failed << " test(s) failed";
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
            std::cout << "\033[31;1m" << summary.failed << " test(s) failed\033[0m";
#endif
        }
        std::cout << "\n";
        std::cout << "  " << summary.passed << " passed, " << summary.failed << " failed";
        std::cout << " (" << summary.total_duration.count() << "ms)\n\n";

        return summary;
    }

private:
    Config config_;
};

} // namespace mana::e2e
