#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <memory>
#include "../frontend/AstModule.h"

namespace mana::test {

// Test result status
enum class TestStatus {
    Passed,
    Failed,
    Skipped,
    Error
};

// Single test result
struct TestResult {
    std::string name;
    std::string file;
    int line = 0;
    TestStatus status = TestStatus::Passed;
    std::string message;
    std::chrono::microseconds duration{0};
};

// Test suite (collection of tests from one file)
struct TestSuite {
    std::string name;
    std::string file;
    std::vector<TestResult> results;
    std::chrono::microseconds total_duration{0};

    int passed_count() const;
    int failed_count() const;
    int skipped_count() const;
    int error_count() const;
};

// Test discovery information
struct TestInfo {
    std::string name;
    std::string file;
    int line;
    bool should_panic = false;      // #[should_panic]
    std::string ignore_reason;       // #[ignore] or #[ignore = "reason"]
    std::vector<std::string> tags;   // #[tag("unit")] etc.
};

// Test configuration
struct TestConfig {
    std::vector<std::string> files;
    std::vector<std::string> include_tags;  // Only run tests with these tags
    std::vector<std::string> exclude_tags;  // Skip tests with these tags
    std::string filter;                     // Name filter pattern
    bool show_output = false;               // Show test output
    bool fail_fast = false;                 // Stop on first failure
    bool parallel = false;                  // Run tests in parallel
    int jobs = 1;                          // Number of parallel jobs
    bool no_capture = false;               // Don't capture stdout/stderr
    bool verbose = false;                  // Verbose output
    std::string output_format = "pretty";  // pretty, json, junit
};

// Test output formats
class TestReporter {
public:
    virtual ~TestReporter() = default;

    virtual void on_run_start(int total_tests) = 0;
    virtual void on_suite_start(const std::string& name) = 0;
    virtual void on_test_start(const TestInfo& test) = 0;
    virtual void on_test_end(const TestResult& result) = 0;
    virtual void on_suite_end(const TestSuite& suite) = 0;
    virtual void on_run_end(const std::vector<TestSuite>& suites) = 0;
};

// Pretty console reporter
class PrettyReporter : public TestReporter {
public:
    void on_run_start(int total_tests) override;
    void on_suite_start(const std::string& name) override;
    void on_test_start(const TestInfo& test) override;
    void on_test_end(const TestResult& result) override;
    void on_suite_end(const TestSuite& suite) override;
    void on_run_end(const std::vector<TestSuite>& suites) override;

private:
    int current_test_ = 0;
    int total_tests_ = 0;
};

// JSON reporter
class JsonReporter : public TestReporter {
public:
    explicit JsonReporter(std::ostream& out);

    void on_run_start(int total_tests) override;
    void on_suite_start(const std::string& name) override;
    void on_test_start(const TestInfo& test) override;
    void on_test_end(const TestResult& result) override;
    void on_suite_end(const TestSuite& suite) override;
    void on_run_end(const std::vector<TestSuite>& suites) override;

private:
    std::ostream& out_;
};

// JUnit XML reporter
class JUnitReporter : public TestReporter {
public:
    explicit JUnitReporter(std::ostream& out);

    void on_run_start(int total_tests) override;
    void on_suite_start(const std::string& name) override;
    void on_test_start(const TestInfo& test) override;
    void on_test_end(const TestResult& result) override;
    void on_suite_end(const TestSuite& suite) override;
    void on_run_end(const std::vector<TestSuite>& suites) override;

private:
    std::ostream& out_;
    std::vector<TestSuite> suites_;
};

// Main test runner
class TestRunner {
public:
    TestRunner(const TestConfig& config = TestConfig());

    // Discover tests in files
    std::vector<TestInfo> discover_tests(const std::vector<std::string>& files);
    std::vector<TestInfo> discover_tests(const frontend::AstModule* module, const std::string& file);

    // Run tests
    int run();
    int run(const std::vector<std::string>& files);

    // Add test files
    void add_file(const std::string& file);
    void add_directory(const std::string& dir, bool recursive = true);

    // Set reporter
    void set_reporter(std::unique_ptr<TestReporter> reporter);

    // Get results
    const std::vector<TestSuite>& get_results() const { return results_; }

private:
    TestConfig config_;
    std::vector<std::string> files_;
    std::vector<TestSuite> results_;
    std::unique_ptr<TestReporter> reporter_;

    bool should_run_test(const TestInfo& test) const;
    TestResult run_single_test(const TestInfo& test);
    void compile_test_file(const std::string& file);
};

// Test assertions (used in generated C++ code)
namespace assertions {

void assert_true(bool condition, const char* expr, const char* file, int line);
void assert_false(bool condition, const char* expr, const char* file, int line);
void assert_eq(const char* left_str, const char* right_str,
               const std::string& left_val, const std::string& right_val,
               const char* file, int line);
void assert_ne(const char* left_str, const char* right_str,
               const std::string& left_val, const std::string& right_val,
               const char* file, int line);
void assert_panic(std::function<void()> fn, const char* file, int line);

} // namespace assertions

} // namespace mana::test
