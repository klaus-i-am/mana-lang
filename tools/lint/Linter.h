#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "../frontend/AstModule.h"
#include "../frontend/Diagnostic.h"

namespace mana::lint {

// Lint warning categories
enum class LintCategory {
    Style,           // Naming conventions, formatting
    Unused,          // Unused variables, imports, functions
    Correctness,     // Potential bugs, unreachable code
    Performance,     // Inefficient patterns
    Complexity,      // Overly complex code
    Documentation,   // Missing docs
    All
};

// Lint severity
enum class LintSeverity {
    Allow,      // Ignore this lint
    Warn,       // Show as warning (default)
    Deny,       // Show as error
    Forbid      // Show as error, cannot be overridden
};

// Individual lint rule
struct LintRule {
    std::string id;              // e.g., "unused_variable"
    std::string name;            // Human-readable name
    std::string description;     // What the lint checks
    LintCategory category;
    LintSeverity default_severity;
    bool enabled = true;
};

// Lint warning/error
struct LintMessage {
    std::string rule_id;
    std::string message;
    std::string file;
    int line = 0;
    int column = 0;
    int span_length = 1;
    LintSeverity severity;
    std::string suggestion;      // Optional fix suggestion
    std::string help;            // Optional help text
};

// Linter configuration
struct LintConfig {
    std::unordered_map<std::string, LintSeverity> rule_severities;
    std::unordered_set<std::string> disabled_rules;
    std::unordered_set<LintCategory> enabled_categories;
    bool fix = false;            // Auto-fix issues when possible
    bool quiet = false;          // Only show errors
    bool verbose = false;        // Show more details
    std::string output_format = "pretty";  // pretty, json, compact
};

// Variable usage tracking
struct VarUsage {
    std::string name;
    std::string type;
    int decl_line = 0;
    int decl_column = 0;
    bool is_read = false;
    bool is_written = false;
    bool is_parameter = false;
    bool is_loop_var = false;
};

// Function tracking
struct FuncInfo {
    std::string name;
    int decl_line = 0;
    bool is_called = false;
    bool is_public = true;
    bool has_return = false;
    int complexity = 1;          // Cyclomatic complexity
};

// Main linter class
class Linter {
public:
    Linter(const LintConfig& config = LintConfig());

    // Lint a single file
    std::vector<LintMessage> lint_file(const std::string& file);
    std::vector<LintMessage> lint_module(const frontend::AstModule* module, const std::string& file);

    // Lint multiple files
    std::vector<LintMessage> lint_files(const std::vector<std::string>& files);

    // Get available rules
    static std::vector<LintRule> get_all_rules();
    static const LintRule* get_rule(const std::string& id);

    // Output results
    void print_results(const std::vector<LintMessage>& messages, std::ostream& out);
    void print_json(const std::vector<LintMessage>& messages, std::ostream& out);
    void print_compact(const std::vector<LintMessage>& messages, std::ostream& out);

    // Statistics
    int warning_count() const { return warning_count_; }
    int error_count() const { return error_count_; }

private:
    LintConfig config_;
    int warning_count_ = 0;
    int error_count_ = 0;

    // Variable and function tracking
    std::vector<std::unordered_map<std::string, VarUsage>> scopes_;
    std::unordered_map<std::string, FuncInfo> functions_;
    std::unordered_set<std::string> imports_;
    std::unordered_set<std::string> used_imports_;

    // Current context
    std::string current_file_;
    bool in_loop_ = false;
    bool after_return_ = false;
    int current_complexity_ = 1;

    // Scope management
    void push_scope();
    void pop_scope();
    void declare_var(const std::string& name, const std::string& type, int line, int col,
                    bool is_param = false, bool is_loop_var = false);
    void mark_var_read(const std::string& name);
    void mark_var_written(const std::string& name);
    VarUsage* find_var(const std::string& name);

    // AST visitors
    void visit_module(const frontend::AstModule* module, std::vector<LintMessage>& messages);
    void visit_function(const frontend::FuncDecl* func, std::vector<LintMessage>& messages);
    void visit_statement(const frontend::AstNode* stmt, std::vector<LintMessage>& messages);
    void visit_expression(const frontend::AstNode* expr, std::vector<LintMessage>& messages);

    // Individual lint checks
    void check_unused_variables(std::vector<LintMessage>& messages);
    void check_unused_functions(std::vector<LintMessage>& messages);
    void check_unused_imports(std::vector<LintMessage>& messages);
    void check_naming_conventions(const std::string& name, const std::string& kind,
                                  int line, int col, std::vector<LintMessage>& messages);
    void check_complexity(const frontend::FuncDecl* func, std::vector<LintMessage>& messages);
    void check_unreachable_code(const frontend::AstNode* stmt, std::vector<LintMessage>& messages);
    void check_shadowing(const std::string& name, int line, int col, std::vector<LintMessage>& messages);
    void check_assignment_in_condition(const frontend::AstNode* cond, std::vector<LintMessage>& messages);
    void check_empty_block(const frontend::BlockStmt* block, const std::string& context,
                          int line, std::vector<LintMessage>& messages);

    // Helpers
    LintSeverity get_severity(const std::string& rule_id) const;
    bool is_rule_enabled(const std::string& rule_id) const;
    void emit(const std::string& rule_id, const std::string& message,
             int line, int col, int span = 1,
             const std::string& suggestion = "", const std::string& help = "",
             std::vector<LintMessage>& messages);

    // Name validation
    bool is_snake_case(const std::string& name) const;
    bool is_upper_snake_case(const std::string& name) const;
    bool is_pascal_case(const std::string& name) const;
    std::string to_snake_case(const std::string& name) const;
};

// CLI interface
int run_lint(int argc, char* argv[]);

} // namespace mana::lint
