#include "TestRunner.h"
#include "../frontend/Lexer.h"
#include "../frontend/Parser.h"
#include "../frontend/Diagnostic.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <map>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace mana::test {

using namespace frontend;

// TestSuite methods
int TestSuite::passed_count() const {
    int count = 0;
    for (const auto& r : results) {
        if (r.status == TestStatus::Passed) count++;
    }
    return count;
}

int TestSuite::failed_count() const {
    int count = 0;
    for (const auto& r : results) {
        if (r.status == TestStatus::Failed) count++;
    }
    return count;
}

int TestSuite::skipped_count() const {
    int count = 0;
    for (const auto& r : results) {
        if (r.status == TestStatus::Skipped) count++;
    }
    return count;
}

int TestSuite::error_count() const {
    int count = 0;
    for (const auto& r : results) {
        if (r.status == TestStatus::Error) count++;
    }
    return count;
}

// PrettyReporter implementation
void PrettyReporter::on_run_start(int total_tests) {
    total_tests_ = total_tests;
    current_test_ = 0;
    std::cout << "\nRunning " << total_tests << " test" << (total_tests != 1 ? "s" : "") << "\n\n";
}

void PrettyReporter::on_suite_start(const std::string& name) {
    std::cout << "  " << name << "\n";
}

void PrettyReporter::on_test_start(const TestInfo& test) {
    current_test_++;
}

void PrettyReporter::on_test_end(const TestResult& result) {
    std::string status_str;
    std::string color_start, color_end;

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    WORD saved_attrs = consoleInfo.wAttributes;
#endif

    switch (result.status) {
        case TestStatus::Passed:
#ifdef _WIN32
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
#else
            color_start = "\033[32m";
            color_end = "\033[0m";
#endif
            status_str = "PASS";
            break;
        case TestStatus::Failed:
#ifdef _WIN32
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
#else
            color_start = "\033[31m";
            color_end = "\033[0m";
#endif
            status_str = "FAIL";
            break;
        case TestStatus::Skipped:
#ifdef _WIN32
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
#else
            color_start = "\033[33m";
            color_end = "\033[0m";
#endif
            status_str = "SKIP";
            break;
        case TestStatus::Error:
#ifdef _WIN32
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#else
            color_start = "\033[91m";
            color_end = "\033[0m";
#endif
            status_str = "ERR ";
            break;
    }

    std::cout << color_start << "    [" << status_str << "]" << color_end;

#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, saved_attrs);
#endif

    std::cout << " " << result.name;

    // Show duration for slower tests
    if (result.duration.count() > 1000) {
        std::cout << " (" << result.duration.count() / 1000 << "ms)";
    }
    std::cout << "\n";

    if (result.status == TestStatus::Failed || result.status == TestStatus::Error) {
        if (!result.message.empty()) {
            std::cout << "           " << result.message << "\n";
            std::cout << "           at " << result.file << ":" << result.line << "\n";
        }
    }
}

void PrettyReporter::on_suite_end(const TestSuite& suite) {
    // No output between suites
}

void PrettyReporter::on_run_end(const std::vector<TestSuite>& suites) {
    int total_passed = 0, total_failed = 0, total_skipped = 0, total_error = 0;
    std::chrono::microseconds total_time{0};

    for (const auto& s : suites) {
        total_passed += s.passed_count();
        total_failed += s.failed_count();
        total_skipped += s.skipped_count();
        total_error += s.error_count();
        total_time += s.total_duration;
    }

    std::cout << "\n";
    std::cout << "Test Results: ";

    if (total_failed == 0 && total_error == 0) {
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
        std::cout << "Some tests failed";
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
        std::cout << "\033[31;1mSome tests failed\033[0m";
#endif
    }
    std::cout << "\n";

    std::cout << "  " << total_passed << " passed";
    if (total_failed > 0) std::cout << ", " << total_failed << " failed";
    if (total_skipped > 0) std::cout << ", " << total_skipped << " skipped";
    if (total_error > 0) std::cout << ", " << total_error << " errors";
    std::cout << " (";
    if (total_time.count() > 1000000) {
        std::cout << total_time.count() / 1000000 << "s";
    } else if (total_time.count() > 1000) {
        std::cout << total_time.count() / 1000 << "ms";
    } else {
        std::cout << total_time.count() << "us";
    }
    std::cout << ")\n\n";
}

// JsonReporter implementation
JsonReporter::JsonReporter(std::ostream& out) : out_(out) {}

void JsonReporter::on_run_start(int total_tests) {
    out_ << "{\n  \"totalTests\": " << total_tests << ",\n  \"suites\": [\n";
}

void JsonReporter::on_suite_start(const std::string& name) {}
void JsonReporter::on_test_start(const TestInfo& test) {}
void JsonReporter::on_test_end(const TestResult& result) {}
void JsonReporter::on_suite_end(const TestSuite& suite) {}

void JsonReporter::on_run_end(const std::vector<TestSuite>& suites) {
    for (size_t i = 0; i < suites.size(); i++) {
        if (i > 0) out_ << ",\n";
        const auto& suite = suites[i];
        out_ << "    {\n";
        out_ << "      \"name\": \"" << suite.name << "\",\n";
        out_ << "      \"file\": \"" << suite.file << "\",\n";
        out_ << "      \"duration\": " << suite.total_duration.count() << ",\n";
        out_ << "      \"tests\": [\n";

        for (size_t j = 0; j < suite.results.size(); j++) {
            if (j > 0) out_ << ",\n";
            const auto& r = suite.results[j];
            out_ << "        {\n";
            out_ << "          \"name\": \"" << r.name << "\",\n";
            out_ << "          \"status\": \"";
            switch (r.status) {
                case TestStatus::Passed: out_ << "passed"; break;
                case TestStatus::Failed: out_ << "failed"; break;
                case TestStatus::Skipped: out_ << "skipped"; break;
                case TestStatus::Error: out_ << "error"; break;
            }
            out_ << "\",\n";
            out_ << "          \"duration\": " << r.duration.count();
            if (!r.message.empty()) {
                out_ << ",\n          \"message\": \"" << r.message << "\"";
            }
            out_ << "\n        }";
        }

        out_ << "\n      ]\n    }";
    }
    out_ << "\n  ]\n}\n";
}

// JUnitReporter implementation
JUnitReporter::JUnitReporter(std::ostream& out) : out_(out) {}

void JUnitReporter::on_run_start(int total_tests) {}
void JUnitReporter::on_suite_start(const std::string& name) {}
void JUnitReporter::on_test_start(const TestInfo& test) {}
void JUnitReporter::on_test_end(const TestResult& result) {}
void JUnitReporter::on_suite_end(const TestSuite& suite) {
    suites_.push_back(suite);
}

void JUnitReporter::on_run_end(const std::vector<TestSuite>& suites) {
    out_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    out_ << "<testsuites>\n";

    for (const auto& suite : suites) {
        out_ << "  <testsuite name=\"" << suite.name << "\" ";
        out_ << "tests=\"" << suite.results.size() << "\" ";
        out_ << "failures=\"" << suite.failed_count() << "\" ";
        out_ << "errors=\"" << suite.error_count() << "\" ";
        out_ << "skipped=\"" << suite.skipped_count() << "\" ";
        out_ << "time=\"" << suite.total_duration.count() / 1000000.0 << "\">\n";

        for (const auto& r : suite.results) {
            out_ << "    <testcase name=\"" << r.name << "\" ";
            out_ << "time=\"" << r.duration.count() / 1000000.0 << "\"";

            if (r.status == TestStatus::Passed) {
                out_ << "/>\n";
            } else {
                out_ << ">\n";
                if (r.status == TestStatus::Failed) {
                    out_ << "      <failure message=\"" << r.message << "\"/>\n";
                } else if (r.status == TestStatus::Error) {
                    out_ << "      <error message=\"" << r.message << "\"/>\n";
                } else if (r.status == TestStatus::Skipped) {
                    out_ << "      <skipped/>\n";
                }
                out_ << "    </testcase>\n";
            }
        }

        out_ << "  </testsuite>\n";
    }

    out_ << "</testsuites>\n";
}

// TestRunner implementation
TestRunner::TestRunner(const TestConfig& config) : config_(config) {
    reporter_ = std::make_unique<PrettyReporter>();
}

std::vector<TestInfo> TestRunner::discover_tests(const std::vector<std::string>& files) {
    std::vector<TestInfo> all_tests;

    for (const auto& file : files) {
        std::ifstream in(file);
        if (!in) {
            std::cerr << "Could not open: " << file << "\n";
            continue;
        }

        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string source = buffer.str();

        DiagnosticEngine diag;
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens, diag);
        auto module = parser.parse_module();

        if (!diag.has_errors()) {
            auto tests = discover_tests(module.get(), file);
            all_tests.insert(all_tests.end(), tests.begin(), tests.end());
        }
    }

    return all_tests;
}

std::vector<TestInfo> TestRunner::discover_tests(const AstModule* module, const std::string& file) {
    std::vector<TestInfo> tests;

    if (!module) return tests;

    for (const auto& decl : module->decls) {
        if (auto fn = dynamic_cast<const AstFuncDecl*>(decl.get())) {
            // Check for #[test] attribute (via is_test flag)
            if (fn->is_test) {
                bool should_panic = false;  // TODO: add attribute support
                std::string ignore_reason;
                std::vector<std::string> tags;

                TestInfo info;
                info.name = fn->name;
                info.file = file;
                info.line = fn->line;
                info.should_panic = should_panic;
                info.ignore_reason = ignore_reason;
                info.tags = tags;
                tests.push_back(info);
            }
        }
    }

    return tests;
}

bool TestRunner::should_run_test(const TestInfo& test) const {
    // Check ignore
    if (!test.ignore_reason.empty()) {
        return false;
    }

    // Check filter
    if (!config_.filter.empty()) {
        std::regex filter_re(config_.filter);
        if (!std::regex_search(test.name, filter_re)) {
            return false;
        }
    }

    // Check include tags
    if (!config_.include_tags.empty()) {
        bool has_tag = false;
        for (const auto& tag : test.tags) {
            if (std::find(config_.include_tags.begin(), config_.include_tags.end(), tag) != config_.include_tags.end()) {
                has_tag = true;
                break;
            }
        }
        if (!has_tag) return false;
    }

    // Check exclude tags
    for (const auto& tag : test.tags) {
        if (std::find(config_.exclude_tags.begin(), config_.exclude_tags.end(), tag) != config_.exclude_tags.end()) {
            return false;
        }
    }

    return true;
}

TestResult TestRunner::run_single_test(const TestInfo& test) {
    TestResult result;
    result.name = test.name;
    result.file = test.file;
    result.line = test.line;

    if (!test.ignore_reason.empty()) {
        result.status = TestStatus::Skipped;
        result.message = test.ignore_reason;
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // Compile and run the test
    // This is a simplified version - full implementation would compile to executable and run
    std::string test_exe = "build/test_" + test.name;
#ifdef _WIN32
    test_exe += ".exe";
#endif

    int exit_code = std::system((test_exe + " 2>&1").c_str());

    auto end = std::chrono::high_resolution_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    if (test.should_panic) {
        // Test should have panicked (non-zero exit)
        if (exit_code != 0) {
            result.status = TestStatus::Passed;
        } else {
            result.status = TestStatus::Failed;
            result.message = "Expected panic but test passed";
        }
    } else {
        if (exit_code == 0) {
            result.status = TestStatus::Passed;
        } else {
            result.status = TestStatus::Failed;
            result.message = "Test failed with exit code " + std::to_string(exit_code);
        }
    }

    return result;
}

void TestRunner::add_file(const std::string& file) {
    files_.push_back(file);
}

void TestRunner::add_directory(const std::string& dir, bool recursive) {
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA((dir + "\\*").c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        std::string name = findData.cFileName;
        if (name == "." || name == "..") continue;

        std::string path = dir + "\\" + name;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (recursive) {
                add_directory(path, true);
            }
        } else if (name.size() > 6 && name.substr(name.size() - 6) == ".mana") {
            files_.push_back(path);
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
#else
    DIR* d = opendir(dir.c_str());
    if (!d) return;

    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string path = dir + "/" + name;
        struct stat st;
        if (stat(path.c_str(), &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (recursive) {
                    add_directory(path, true);
                }
            } else if (name.size() > 6 && name.substr(name.size() - 6) == ".mana") {
                files_.push_back(path);
            }
        }
    }

    closedir(d);
#endif
}

void TestRunner::set_reporter(std::unique_ptr<TestReporter> reporter) {
    reporter_ = std::move(reporter);
}

int TestRunner::run() {
    return run(files_.empty() ? config_.files : files_);
}

int TestRunner::run(const std::vector<std::string>& files) {
    // Discover all tests
    auto all_tests = discover_tests(files);

    // Filter tests
    std::vector<TestInfo> tests_to_run;
    for (const auto& test : all_tests) {
        if (should_run_test(test)) {
            tests_to_run.push_back(test);
        }
    }

    reporter_->on_run_start(static_cast<int>(tests_to_run.size()));

    // Group by file
    std::map<std::string, std::vector<TestInfo>> tests_by_file;
    for (const auto& test : tests_to_run) {
        tests_by_file[test.file].push_back(test);
    }

    // Run tests
    for (auto it = tests_by_file.begin(); it != tests_by_file.end(); ++it) {
        const std::string& file = it->first;
        const std::vector<TestInfo>& file_tests = it->second;
        
        TestSuite suite;
        suite.file = file;
        // Extract filename without path
        size_t last_slash = file.find_last_of("/\\");
        suite.name = (last_slash != std::string::npos) ? file.substr(last_slash + 1) : file;

        reporter_->on_suite_start(suite.name);

        for (size_t i = 0; i < file_tests.size(); ++i) {
            const TestInfo& test = file_tests[i];
            reporter_->on_test_start(test);

            TestResult result = run_single_test(test);
            suite.results.push_back(result);
            suite.total_duration += result.duration;

            reporter_->on_test_end(result);

            if (config_.fail_fast && result.status == TestStatus::Failed) {
                break;
            }
        }

        reporter_->on_suite_end(suite);
        results_.push_back(suite);

        if (config_.fail_fast && suite.failed_count() > 0) {
            break;
        }
    }

    reporter_->on_run_end(results_);

    // Return exit code
    int total_failed = 0;
    for (const auto& s : results_) {
        total_failed += s.failed_count() + s.error_count();
    }
    return total_failed > 0 ? 1 : 0;
}

// Assertions implementation
namespace assertions {

void assert_true(bool condition, const char* expr, const char* file, int line) {
    if (!condition) {
        std::cerr << "Assertion failed: " << expr << "\n";
        std::cerr << "  at " << file << ":" << line << "\n";
        std::exit(1);
    }
}

void assert_false(bool condition, const char* expr, const char* file, int line) {
    if (condition) {
        std::cerr << "Assertion failed: !" << expr << "\n";
        std::cerr << "  at " << file << ":" << line << "\n";
        std::exit(1);
    }
}

void assert_eq(const char* left_str, const char* right_str,
               const std::string& left_val, const std::string& right_val,
               const char* file, int line) {
    if (left_val != right_val) {
        std::cerr << "Assertion failed: " << left_str << " == " << right_str << "\n";
        std::cerr << "  left:  " << left_val << "\n";
        std::cerr << "  right: " << right_val << "\n";
        std::cerr << "  at " << file << ":" << line << "\n";
        std::exit(1);
    }
}

void assert_ne(const char* left_str, const char* right_str,
               const std::string& left_val, const std::string& right_val,
               const char* file, int line) {
    if (left_val == right_val) {
        std::cerr << "Assertion failed: " << left_str << " != " << right_str << "\n";
        std::cerr << "  both equal: " << left_val << "\n";
        std::cerr << "  at " << file << ":" << line << "\n";
        std::exit(1);
    }
}

void assert_panic(std::function<void()> fn, const char* file, int line) {
    bool panicked = false;
    try {
        fn();
    } catch (...) {
        panicked = true;
    }

    if (!panicked) {
        std::cerr << "Assertion failed: expected panic\n";
        std::cerr << "  at " << file << ":" << line << "\n";
        std::exit(1);
    }
}

} // namespace assertions

} // namespace mana::test
