#include "Linter.h"
#include "../frontend/Lexer.h"
#include "../frontend/Parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>

namespace mana::lint {

// Static rule definitions
static std::vector<LintRule> s_all_rules = {
    {"unused_variable", "Unused Variable", "Warns when a variable is declared but never used",
     LintCategory::Unused, LintSeverity::Warn, true},
    {"unused_function", "Unused Function", "Warns when a private function is never called",
     LintCategory::Unused, LintSeverity::Warn, true},
    {"unused_import", "Unused Import", "Warns when an import is never used",
     LintCategory::Unused, LintSeverity::Warn, true},
    {"unused_parameter", "Unused Parameter", "Warns when a function parameter is never used",
     LintCategory::Unused, LintSeverity::Warn, true},

    {"snake_case_variable", "Snake Case Variable", "Variables should use snake_case naming",
     LintCategory::Style, LintSeverity::Warn, true},
    {"snake_case_function", "Snake Case Function", "Functions should use snake_case naming",
     LintCategory::Style, LintSeverity::Warn, true},
    {"upper_snake_case_constant", "Upper Snake Case Constant", "Constants should use UPPER_SNAKE_CASE",
     LintCategory::Style, LintSeverity::Warn, true},
    {"pascal_case_type", "Pascal Case Type", "Types should use PascalCase naming",
     LintCategory::Style, LintSeverity::Warn, true},

    {"unreachable_code", "Unreachable Code", "Code after return/break/continue is unreachable",
     LintCategory::Correctness, LintSeverity::Warn, true},
    {"shadowed_variable", "Shadowed Variable", "Variable shadows a variable from an outer scope",
     LintCategory::Correctness, LintSeverity::Warn, true},
    {"assignment_in_condition", "Assignment in Condition", "Assignment used where comparison expected",
     LintCategory::Correctness, LintSeverity::Warn, true},
    {"empty_block", "Empty Block", "Empty block statements may indicate missing code",
     LintCategory::Correctness, LintSeverity::Warn, true},
    {"break_outside_loop", "Break Outside Loop", "Break statement outside of loop",
     LintCategory::Correctness, LintSeverity::Deny, true},
    {"continue_outside_loop", "Continue Outside Loop", "Continue statement outside of loop",
     LintCategory::Correctness, LintSeverity::Deny, true},

    {"high_complexity", "High Complexity", "Function has high cyclomatic complexity",
     LintCategory::Complexity, LintSeverity::Warn, true},
    {"too_many_parameters", "Too Many Parameters", "Function has too many parameters",
     LintCategory::Complexity, LintSeverity::Warn, true},
    {"deep_nesting", "Deep Nesting", "Code has deep nesting levels",
     LintCategory::Complexity, LintSeverity::Warn, true},

    {"missing_return", "Missing Return", "Non-void function may not return a value",
     LintCategory::Correctness, LintSeverity::Warn, true},
    {"redundant_return", "Redundant Return", "Void function has unnecessary return statement",
     LintCategory::Style, LintSeverity::Warn, false},  // Disabled by default
};

// Terminal colors
static bool s_colors_enabled = true;

static const char* color_reset() { return s_colors_enabled ? "\033[0m" : ""; }
static const char* color_bold() { return s_colors_enabled ? "\033[1m" : ""; }
static const char* color_red() { return s_colors_enabled ? "\033[31m" : ""; }
static const char* color_yellow() { return s_colors_enabled ? "\033[33m" : ""; }
static const char* color_blue() { return s_colors_enabled ? "\033[34m" : ""; }
static const char* color_cyan() { return s_colors_enabled ? "\033[36m" : ""; }
static const char* color_green() { return s_colors_enabled ? "\033[32m" : ""; }
static const char* color_dim() { return s_colors_enabled ? "\033[2m" : ""; }

// Linter implementation
Linter::Linter(const LintConfig& config) : config_(config) {}

std::vector<LintRule> Linter::get_all_rules() {
    return s_all_rules;
}

const LintRule* Linter::get_rule(const std::string& id) {
    for (const auto& rule : s_all_rules) {
        if (rule.id == id) {
            return &rule;
        }
    }
    return nullptr;
}

void Linter::push_scope() {
    scopes_.push_back({});
}

void Linter::pop_scope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

void Linter::declare_var(const std::string& name, const std::string& type,
                         int line, int col, bool is_param, bool is_loop_var) {
    if (scopes_.empty()) {
        push_scope();
    }

    VarUsage usage;
    usage.name = name;
    usage.type = type;
    usage.decl_line = line;
    usage.decl_column = col;
    usage.is_parameter = is_param;
    usage.is_loop_var = is_loop_var;
    usage.is_read = false;
    usage.is_written = is_loop_var;  // Loop vars are implicitly written

    scopes_.back()[name] = usage;
}

void Linter::mark_var_read(const std::string& name) {
    if (VarUsage* var = find_var(name)) {
        var->is_read = true;
    }
}

void Linter::mark_var_written(const std::string& name) {
    if (VarUsage* var = find_var(name)) {
        var->is_written = true;
    }
}

VarUsage* Linter::find_var(const std::string& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

std::vector<LintMessage> Linter::lint_file(const std::string& file) {
    std::vector<LintMessage> messages;

    std::ifstream in(file);
    if (!in) {
        LintMessage msg;
        msg.rule_id = "file_error";
        msg.message = "Cannot open file: " + file;
        msg.file = file;
        msg.severity = LintSeverity::Deny;
        messages.push_back(msg);
        return messages;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    // Lex and parse
    frontend::Lexer lexer(source);
    auto tokens = lexer.tokenize();

    frontend::DiagnosticEngine diag;
    diag.set_source(file, source);

    frontend::Parser parser(tokens, diag);
    auto module = parser.parse();

    if (!module) {
        LintMessage msg;
        msg.rule_id = "parse_error";
        msg.message = "Failed to parse file: " + file;
        msg.file = file;
        msg.severity = LintSeverity::Deny;
        messages.push_back(msg);
        return messages;
    }

    return lint_module(module.get(), file);
}

std::vector<LintMessage> Linter::lint_module(const frontend::AstModule* module,
                                              const std::string& file) {
    std::vector<LintMessage> messages;
    current_file_ = file;

    // Reset state
    scopes_.clear();
    functions_.clear();
    imports_.clear();
    used_imports_.clear();
    warning_count_ = 0;
    error_count_ = 0;

    visit_module(module, messages);

    // Check for unused functions
    check_unused_functions(messages);

    // Check for unused imports
    check_unused_imports(messages);

    // Count warnings and errors
    for (const auto& msg : messages) {
        if (msg.severity == LintSeverity::Warn) {
            warning_count_++;
        } else if (msg.severity == LintSeverity::Deny || msg.severity == LintSeverity::Forbid) {
            error_count_++;
        }
    }

    return messages;
}

std::vector<LintMessage> Linter::lint_files(const std::vector<std::string>& files) {
    std::vector<LintMessage> all_messages;

    for (const auto& file : files) {
        auto messages = lint_file(file);
        all_messages.insert(all_messages.end(), messages.begin(), messages.end());
    }

    return all_messages;
}

void Linter::visit_module(const frontend::AstModule* module, std::vector<LintMessage>& messages) {
    push_scope();  // Global scope

    // Process imports
    for (const auto& import : module->imports) {
        if (import && import->kind == frontend::NodeKind::Import) {
            auto* imp = static_cast<const frontend::Import*>(import.get());
            imports_.insert(imp->module_path);
        }
    }

    // Process functions
    for (const auto& decl : module->declarations) {
        if (decl && decl->kind == frontend::NodeKind::FuncDecl) {
            auto* func = static_cast<const frontend::FuncDecl*>(decl.get());

            FuncInfo info;
            info.name = func->name;
            info.decl_line = func->line;
            info.is_public = !func->is_private;
            info.is_called = (func->name == "main");  // main is always "called"
            functions_[func->name] = info;

            // Check function naming
            check_naming_conventions(func->name, "function", func->line, 1, messages);
        }
    }

    // Visit function bodies
    for (const auto& decl : module->declarations) {
        if (decl && decl->kind == frontend::NodeKind::FuncDecl) {
            auto* func = static_cast<const frontend::FuncDecl*>(decl.get());
            visit_function(func, messages);
        }
    }

    // Check unused variables in global scope
    check_unused_variables(messages);
    pop_scope();
}

void Linter::visit_function(const frontend::FuncDecl* func, std::vector<LintMessage>& messages) {
    push_scope();

    in_loop_ = false;
    after_return_ = false;
    current_complexity_ = 1;

    // Declare parameters
    for (const auto& param : func->params) {
        if (param.name.empty()) continue;

        // Check parameter naming
        check_naming_conventions(param.name, "parameter", func->line, 1, messages);

        // Check for shadowing
        check_shadowing(param.name, func->line, 1, messages);

        declare_var(param.name, param.type, func->line, 1, true, false);
    }

    // Check for too many parameters
    if (func->params.size() > 5 && is_rule_enabled("too_many_parameters")) {
        emit("too_many_parameters",
             "Function '" + func->name + "' has " + std::to_string(func->params.size()) +
             " parameters (consider using a struct)",
             func->line, 1, static_cast<int>(func->name.length()),
             "", "Functions with many parameters are harder to use correctly",
             messages);
    }

    // Visit body
    if (func->body) {
        auto* block = static_cast<const frontend::BlockStmt*>(func->body.get());
        for (const auto& stmt : block->statements) {
            visit_statement(stmt.get(), messages);
        }

        // Check for empty function body
        if (block->statements.empty()) {
            check_empty_block(block, "function body", func->line, messages);
        }
    }

    // Check cyclomatic complexity
    check_complexity(func, messages);

    // Check unused variables
    check_unused_variables(messages);

    pop_scope();
}

void Linter::visit_statement(const frontend::AstNode* stmt, std::vector<LintMessage>& messages) {
    if (!stmt) return;

    // Check for unreachable code
    if (after_return_ && is_rule_enabled("unreachable_code")) {
        emit("unreachable_code", "Unreachable code after return/break/continue",
             stmt->line, 1, 1, "Remove this code or the preceding control flow statement", "",
             messages);
    }

    switch (stmt->kind) {
    case frontend::NodeKind::VarDeclStmt: {
        auto* var = static_cast<const frontend::VarDeclStmt*>(stmt);

        // Check naming
        check_naming_conventions(var->name, var->is_const ? "constant" : "variable",
                                stmt->line, 1, messages);

        // Check shadowing
        check_shadowing(var->name, stmt->line, 1, messages);

        declare_var(var->name, var->type_name, stmt->line, 1, false, false);

        if (var->initializer) {
            visit_expression(var->initializer.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::AssignStmt: {
        auto* assign = static_cast<const frontend::AssignStmt*>(stmt);
        mark_var_written(assign->target);

        // Check for assignment as condition (unlikely here but check anyway)
        if (assign->value) {
            visit_expression(assign->value.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::ExprStmt: {
        auto* expr = static_cast<const frontend::ExprStmt*>(stmt);
        if (expr->expr) {
            visit_expression(expr->expr.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::BlockStmt: {
        auto* block = static_cast<const frontend::BlockStmt*>(stmt);
        push_scope();
        for (const auto& s : block->statements) {
            visit_statement(s.get(), messages);
        }
        check_unused_variables(messages);
        pop_scope();
        break;
    }

    case frontend::NodeKind::IfStmt: {
        auto* if_stmt = static_cast<const frontend::IfStmt*>(stmt);
        current_complexity_++;  // Branching increases complexity

        // Check condition for assignment
        if (if_stmt->condition) {
            check_assignment_in_condition(if_stmt->condition.get(), messages);
            visit_expression(if_stmt->condition.get(), messages);
        }

        bool then_returns = false;
        bool else_returns = false;

        if (if_stmt->then_branch) {
            bool saved_after_return = after_return_;
            after_return_ = false;
            visit_statement(if_stmt->then_branch.get(), messages);
            then_returns = after_return_;
            after_return_ = saved_after_return;

            // Check for empty then block
            if (if_stmt->then_branch->kind == frontend::NodeKind::BlockStmt) {
                auto* block = static_cast<const frontend::BlockStmt*>(if_stmt->then_branch.get());
                check_empty_block(block, "if body", stmt->line, messages);
            }
        }

        if (if_stmt->else_branch) {
            bool saved_after_return = after_return_;
            after_return_ = false;
            visit_statement(if_stmt->else_branch.get(), messages);
            else_returns = after_return_;
            after_return_ = saved_after_return;
        }

        // Both branches return means code after this is unreachable
        after_return_ = then_returns && else_returns;
        break;
    }

    case frontend::NodeKind::WhileStmt: {
        auto* while_stmt = static_cast<const frontend::WhileStmt*>(stmt);
        current_complexity_++;  // Loop increases complexity

        // Check condition
        if (while_stmt->condition) {
            check_assignment_in_condition(while_stmt->condition.get(), messages);
            visit_expression(while_stmt->condition.get(), messages);
        }

        bool saved_in_loop = in_loop_;
        in_loop_ = true;

        if (while_stmt->body) {
            visit_statement(while_stmt->body.get(), messages);

            // Check for empty loop body
            if (while_stmt->body->kind == frontend::NodeKind::BlockStmt) {
                auto* block = static_cast<const frontend::BlockStmt*>(while_stmt->body.get());
                check_empty_block(block, "while body", stmt->line, messages);
            }
        }

        in_loop_ = saved_in_loop;
        break;
    }

    case frontend::NodeKind::ForStmt: {
        auto* for_stmt = static_cast<const frontend::ForStmt*>(stmt);
        current_complexity_++;

        push_scope();

        if (for_stmt->init) {
            visit_statement(for_stmt->init.get(), messages);
        }
        if (for_stmt->condition) {
            visit_expression(for_stmt->condition.get(), messages);
        }
        if (for_stmt->update) {
            visit_statement(for_stmt->update.get(), messages);
        }

        bool saved_in_loop = in_loop_;
        in_loop_ = true;

        if (for_stmt->body) {
            visit_statement(for_stmt->body.get(), messages);
        }

        in_loop_ = saved_in_loop;
        check_unused_variables(messages);
        pop_scope();
        break;
    }

    case frontend::NodeKind::ReturnStmt: {
        auto* ret = static_cast<const frontend::ReturnStmt*>(stmt);
        if (ret->value) {
            visit_expression(ret->value.get(), messages);
        }
        after_return_ = true;
        break;
    }

    case frontend::NodeKind::BreakStmt:
        if (!in_loop_ && is_rule_enabled("break_outside_loop")) {
            emit("break_outside_loop", "Break statement outside of loop",
                 stmt->line, 1, 5, "Remove this break or move it inside a loop", "",
                 messages);
        }
        after_return_ = true;  // Unreachable after break (within loop)
        break;

    case frontend::NodeKind::ContinueStmt:
        if (!in_loop_ && is_rule_enabled("continue_outside_loop")) {
            emit("continue_outside_loop", "Continue statement outside of loop",
                 stmt->line, 1, 8, "Remove this continue or move it inside a loop", "",
                 messages);
        }
        after_return_ = true;  // Unreachable after continue (within loop)
        break;

    case frontend::NodeKind::DeferStmt: {
        auto* defer = static_cast<const frontend::DeferStmt*>(stmt);
        if (defer->body) {
            visit_statement(defer->body.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::MatchStmt: {
        auto* match = static_cast<const frontend::MatchStmt*>(stmt);
        if (match->value) {
            visit_expression(match->value.get(), messages);
        }
        for (const auto& arm : match->arms) {
            current_complexity_++;  // Each match arm increases complexity
            if (arm.body) {
                visit_statement(arm.body.get(), messages);
            }
        }
        break;
    }

    default:
        break;
    }
}

void Linter::visit_expression(const frontend::AstNode* expr, std::vector<LintMessage>& messages) {
    if (!expr) return;

    switch (expr->kind) {
    case frontend::NodeKind::Identifier: {
        auto* id = static_cast<const frontend::AstIdentifierExpr*>(expr);
        mark_var_read(id->name);

        // Check if this is using an imported module
        auto dot_pos = id->name.find('.');
        if (dot_pos != std::string::npos) {
            std::string module = id->name.substr(0, dot_pos);
            used_imports_.insert(module);
        }
        break;
    }

    case frontend::NodeKind::CallExpr: {
        auto* call = static_cast<const frontend::AstCallExpr*>(expr);

        // Mark function as called
        auto it = functions_.find(call->func_name);
        if (it != functions_.end()) {
            it->second.is_called = true;
        }

        // Check for imported function usage
        auto dot_pos = call->func_name.find('.');
        if (dot_pos != std::string::npos) {
            std::string module = call->func_name.substr(0, dot_pos);
            used_imports_.insert(module);
        }

        // Visit arguments
        for (const auto& arg : call->args) {
            visit_expression(arg.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::BinaryExpr: {
        auto* bin = static_cast<const frontend::AstBinaryExpr*>(expr);
        current_complexity_++;  // && and || increase complexity

        if (bin->left) {
            visit_expression(bin->left.get(), messages);
        }
        if (bin->right) {
            visit_expression(bin->right.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::UnaryExpr: {
        auto* unary = static_cast<const frontend::AstUnaryExpr*>(expr);
        if (unary->operand) {
            visit_expression(unary->operand.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::MemberExpr: {
        auto* member = static_cast<const frontend::AstMemberExpr*>(expr);
        if (member->object) {
            visit_expression(member->object.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::IndexExpr: {
        auto* index = static_cast<const frontend::AstIndexExpr*>(expr);
        if (index->array) {
            visit_expression(index->array.get(), messages);
        }
        if (index->index) {
            visit_expression(index->index.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::ArrayExpr: {
        auto* arr = static_cast<const frontend::AstArrayExpr*>(expr);
        for (const auto& elem : arr->elements) {
            visit_expression(elem.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::CastExpr: {
        auto* cast = static_cast<const frontend::AstCastExpr*>(expr);
        if (cast->expr) {
            visit_expression(cast->expr.get(), messages);
        }
        break;
    }

    case frontend::NodeKind::TernaryExpr: {
        auto* tern = static_cast<const frontend::AstTernaryExpr*>(expr);
        current_complexity_++;

        if (tern->condition) {
            visit_expression(tern->condition.get(), messages);
        }
        if (tern->then_expr) {
            visit_expression(tern->then_expr.get(), messages);
        }
        if (tern->else_expr) {
            visit_expression(tern->else_expr.get(), messages);
        }
        break;
    }

    default:
        break;
    }
}

void Linter::check_unused_variables(std::vector<LintMessage>& messages) {
    if (scopes_.empty()) return;

    const auto& scope = scopes_.back();
    for (const auto& [name, var] : scope) {
        if (!var.is_read) {
            if (var.is_parameter) {
                if (is_rule_enabled("unused_parameter")) {
                    emit("unused_parameter",
                         "Parameter '" + name + "' is never used",
                         var.decl_line, var.decl_column, static_cast<int>(name.length()),
                         "Prefix with underscore to silence: _" + name, "",
                         messages);
                }
            } else if (!var.is_loop_var) {
                if (is_rule_enabled("unused_variable")) {
                    emit("unused_variable",
                         "Variable '" + name + "' is declared but never used",
                         var.decl_line, var.decl_column, static_cast<int>(name.length()),
                         "Remove this variable or use it", "",
                         messages);
                }
            }
        }
    }
}

void Linter::check_unused_functions(std::vector<LintMessage>& messages) {
    if (!is_rule_enabled("unused_function")) return;

    for (const auto& [name, func] : functions_) {
        if (!func.is_called && !func.is_public) {
            emit("unused_function",
                 "Private function '" + name + "' is never called",
                 func.decl_line, 1, static_cast<int>(name.length()),
                 "Remove this function or call it", "",
                 messages);
        }
    }
}

void Linter::check_unused_imports(std::vector<LintMessage>& messages) {
    if (!is_rule_enabled("unused_import")) return;

    for (const auto& import : imports_) {
        if (used_imports_.find(import) == used_imports_.end()) {
            emit("unused_import",
                 "Import '" + import + "' is never used",
                 1, 1, static_cast<int>(import.length()),
                 "Remove this import", "",
                 messages);
        }
    }
}

void Linter::check_naming_conventions(const std::string& name, const std::string& kind,
                                       int line, int col, std::vector<LintMessage>& messages) {
    // Skip names starting with underscore (intentionally ignored)
    if (!name.empty() && name[0] == '_') return;

    if (kind == "variable" || kind == "parameter") {
        if (!is_snake_case(name) && is_rule_enabled("snake_case_variable")) {
            emit("snake_case_variable",
                 "Variable '" + name + "' should use snake_case",
                 line, col, static_cast<int>(name.length()),
                 "Rename to: " + to_snake_case(name), "",
                 messages);
        }
    } else if (kind == "function") {
        if (!is_snake_case(name) && is_rule_enabled("snake_case_function")) {
            emit("snake_case_function",
                 "Function '" + name + "' should use snake_case",
                 line, col, static_cast<int>(name.length()),
                 "Rename to: " + to_snake_case(name), "",
                 messages);
        }
    } else if (kind == "constant") {
        if (!is_upper_snake_case(name) && is_rule_enabled("upper_snake_case_constant")) {
            emit("upper_snake_case_constant",
                 "Constant '" + name + "' should use UPPER_SNAKE_CASE",
                 line, col, static_cast<int>(name.length()),
                 "", "",
                 messages);
        }
    }
}

void Linter::check_complexity(const frontend::FuncDecl* func, std::vector<LintMessage>& messages) {
    if (!is_rule_enabled("high_complexity")) return;

    const int MAX_COMPLEXITY = 10;
    if (current_complexity_ > MAX_COMPLEXITY) {
        emit("high_complexity",
             "Function '" + func->name + "' has cyclomatic complexity of " +
             std::to_string(current_complexity_) + " (max: " + std::to_string(MAX_COMPLEXITY) + ")",
             func->line, 1, static_cast<int>(func->name.length()),
             "Consider breaking this function into smaller pieces", "",
             messages);
    }
}

void Linter::check_unreachable_code(const frontend::AstNode* stmt, std::vector<LintMessage>& messages) {
    // Already handled in visit_statement
}

void Linter::check_shadowing(const std::string& name, int line, int col,
                              std::vector<LintMessage>& messages) {
    if (!is_rule_enabled("shadowed_variable")) return;

    // Check if variable exists in outer scopes (skip the current scope)
    for (size_t i = 0; i + 1 < scopes_.size(); ++i) {
        auto it = scopes_[i].find(name);
        if (it != scopes_[i].end()) {
            emit("shadowed_variable",
                 "Variable '" + name + "' shadows a variable in outer scope (line " +
                 std::to_string(it->second.decl_line) + ")",
                 line, col, static_cast<int>(name.length()),
                 "Use a different name to avoid confusion", "",
                 messages);
            break;
        }
    }
}

void Linter::check_assignment_in_condition(const frontend::AstNode* cond,
                                           std::vector<LintMessage>& messages) {
    if (!cond || !is_rule_enabled("assignment_in_condition")) return;

    // This is a simplified check - would need more sophisticated analysis
    // to detect assignment expressions in conditions
}

void Linter::check_empty_block(const frontend::BlockStmt* block, const std::string& context,
                               int line, std::vector<LintMessage>& messages) {
    if (!is_rule_enabled("empty_block")) return;

    if (block && block->statements.empty()) {
        emit("empty_block",
             "Empty " + context,
             line, 1, 1,
             "Add code or a comment explaining why this is empty", "",
             messages);
    }
}

LintSeverity Linter::get_severity(const std::string& rule_id) const {
    auto it = config_.rule_severities.find(rule_id);
    if (it != config_.rule_severities.end()) {
        return it->second;
    }

    const LintRule* rule = get_rule(rule_id);
    if (rule) {
        return rule->default_severity;
    }
    return LintSeverity::Warn;
}

bool Linter::is_rule_enabled(const std::string& rule_id) const {
    if (config_.disabled_rules.count(rule_id)) {
        return false;
    }

    const LintRule* rule = get_rule(rule_id);
    if (rule) {
        return rule->enabled;
    }
    return true;
}

void Linter::emit(const std::string& rule_id, const std::string& message,
                  int line, int col, int span,
                  const std::string& suggestion, const std::string& help,
                  std::vector<LintMessage>& messages) {
    LintMessage msg;
    msg.rule_id = rule_id;
    msg.message = message;
    msg.file = current_file_;
    msg.line = line;
    msg.column = col;
    msg.span_length = span;
    msg.severity = get_severity(rule_id);
    msg.suggestion = suggestion;
    msg.help = help;
    messages.push_back(msg);
}

bool Linter::is_snake_case(const std::string& name) const {
    if (name.empty()) return true;

    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (std::isupper(c)) return false;
        if (c == '_' && i > 0 && name[i-1] == '_') return false;  // No double underscores
    }
    return true;
}

bool Linter::is_upper_snake_case(const std::string& name) const {
    if (name.empty()) return true;

    for (char c : name) {
        if (std::islower(c)) return false;
    }
    return true;
}

bool Linter::is_pascal_case(const std::string& name) const {
    if (name.empty()) return true;
    if (!std::isupper(name[0])) return false;

    for (char c : name) {
        if (c == '_') return false;
    }
    return true;
}

std::string Linter::to_snake_case(const std::string& name) const {
    if (name.empty()) return name;

    std::string result;
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (std::isupper(c)) {
            if (i > 0) result += '_';
            result += static_cast<char>(std::tolower(c));
        } else {
            result += c;
        }
    }
    return result;
}

void Linter::print_results(const std::vector<LintMessage>& messages, std::ostream& out) {
    if (config_.output_format == "json") {
        print_json(messages, out);
        return;
    }
    if (config_.output_format == "compact") {
        print_compact(messages, out);
        return;
    }

    // Pretty format (default)
    std::string last_file;

    for (const auto& msg : messages) {
        if (config_.quiet && msg.severity == LintSeverity::Warn) {
            continue;
        }

        // Print file header if changed
        if (msg.file != last_file) {
            if (!last_file.empty()) {
                out << "\n";
            }
            last_file = msg.file;
        }

        // Print severity
        switch (msg.severity) {
        case LintSeverity::Forbid:
        case LintSeverity::Deny:
            out << color_bold() << color_red() << "error" << color_reset();
            break;
        case LintSeverity::Warn:
            out << color_bold() << color_yellow() << "warning" << color_reset();
            break;
        default:
            out << color_bold() << "note" << color_reset();
            break;
        }

        out << "[" << msg.rule_id << "]" << color_reset()
            << ": " << color_bold() << msg.message << color_reset() << "\n";

        // Print location
        out << color_blue() << "  --> " << color_reset()
            << msg.file << ":" << msg.line << ":" << msg.column << "\n";

        // Print suggestion if present
        if (!msg.suggestion.empty()) {
            out << color_green() << "   = suggestion: " << color_reset()
                << msg.suggestion << "\n";
        }

        // Print help if present
        if (!msg.help.empty()) {
            out << color_cyan() << "   = help: " << color_reset()
                << msg.help << "\n";
        }

        out << "\n";
    }

    // Print summary
    int warns = 0, errors = 0;
    for (const auto& msg : messages) {
        if (msg.severity == LintSeverity::Warn) warns++;
        else if (msg.severity == LintSeverity::Deny || msg.severity == LintSeverity::Forbid) errors++;
    }

    if (warns > 0 || errors > 0) {
        out << color_bold();
        if (errors > 0) {
            out << color_red() << errors << " error" << (errors > 1 ? "s" : "") << color_reset();
            if (warns > 0) out << ", ";
        }
        if (warns > 0) {
            out << color_yellow() << warns << " warning" << (warns > 1 ? "s" : "") << color_reset();
        }
        out << " generated\n";
    } else {
        out << color_green() << color_bold() << "No lint issues found" << color_reset() << "\n";
    }
}

void Linter::print_json(const std::vector<LintMessage>& messages, std::ostream& out) {
    out << "[\n";
    bool first = true;
    for (const auto& msg : messages) {
        if (!first) out << ",\n";
        first = false;

        out << "  {\n";
        out << "    \"rule\": \"" << msg.rule_id << "\",\n";
        out << "    \"severity\": \"";
        switch (msg.severity) {
            case LintSeverity::Allow: out << "allow"; break;
            case LintSeverity::Warn: out << "warning"; break;
            case LintSeverity::Deny: out << "error"; break;
            case LintSeverity::Forbid: out << "error"; break;
        }
        out << "\",\n";
        out << "    \"message\": \"" << msg.message << "\",\n";
        out << "    \"file\": \"" << msg.file << "\",\n";
        out << "    \"line\": " << msg.line << ",\n";
        out << "    \"column\": " << msg.column;
        if (!msg.suggestion.empty()) {
            out << ",\n    \"suggestion\": \"" << msg.suggestion << "\"";
        }
        if (!msg.help.empty()) {
            out << ",\n    \"help\": \"" << msg.help << "\"";
        }
        out << "\n  }";
    }
    out << "\n]\n";
}

void Linter::print_compact(const std::vector<LintMessage>& messages, std::ostream& out) {
    for (const auto& msg : messages) {
        out << msg.file << ":" << msg.line << ":" << msg.column << ": ";
        switch (msg.severity) {
            case LintSeverity::Allow: out << "note"; break;
            case LintSeverity::Warn: out << "warning"; break;
            case LintSeverity::Deny:
            case LintSeverity::Forbid: out << "error"; break;
        }
        out << " [" << msg.rule_id << "]: " << msg.message << "\n";
    }
}

// CLI entry point
int run_lint(int argc, char* argv[]) {
    LintConfig config;
    std::vector<std::string> files;
    bool show_help = false;
    bool show_rules = false;

    // Parse arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            show_help = true;
        } else if (arg == "--rules") {
            show_rules = true;
        } else if (arg == "-q" || arg == "--quiet") {
            config.quiet = true;
        } else if (arg == "-v" || arg == "--verbose") {
            config.verbose = true;
        } else if (arg == "--fix") {
            config.fix = true;
        } else if (arg == "--json") {
            config.output_format = "json";
        } else if (arg == "--compact") {
            config.output_format = "compact";
        } else if (arg.substr(0, 2) == "-W") {
            std::string rule = arg.substr(2);
            config.rule_severities[rule] = LintSeverity::Warn;
        } else if (arg.substr(0, 2) == "-D") {
            std::string rule = arg.substr(2);
            config.rule_severities[rule] = LintSeverity::Deny;
        } else if (arg.substr(0, 2) == "-A") {
            std::string rule = arg.substr(2);
            config.disabled_rules.insert(rule);
        } else if (arg[0] != '-') {
            files.push_back(arg);
        }
    }

    if (show_help) {
        std::cout << "mana lint - Mana code linter\n\n";
        std::cout << "USAGE:\n";
        std::cout << "    mana lint [OPTIONS] <FILES>...\n\n";
        std::cout << "OPTIONS:\n";
        std::cout << "    -h, --help     Show this help message\n";
        std::cout << "    --rules        List all available lint rules\n";
        std::cout << "    -q, --quiet    Only show errors, not warnings\n";
        std::cout << "    -v, --verbose  Show more details\n";
        std::cout << "    --fix          Auto-fix issues when possible\n";
        std::cout << "    --json         Output in JSON format\n";
        std::cout << "    --compact      Output in compact format\n";
        std::cout << "    -W<rule>       Set rule severity to warning\n";
        std::cout << "    -D<rule>       Set rule severity to error\n";
        std::cout << "    -A<rule>       Allow (disable) a rule\n\n";
        std::cout << "EXAMPLES:\n";
        std::cout << "    mana lint src/*.mana\n";
        std::cout << "    mana lint -Dunused_variable -Aempty_block main.mana\n";
        std::cout << "    mana lint --json src/ > lint_report.json\n";
        return 0;
    }

    if (show_rules) {
        std::cout << "Available lint rules:\n\n";
        auto rules = Linter::get_all_rules();
        for (const auto& rule : rules) {
            std::cout << "  " << color_bold() << std::setw(28) << std::left << rule.id << color_reset();

            switch (rule.default_severity) {
                case LintSeverity::Warn:
                    std::cout << color_yellow() << " warn  " << color_reset();
                    break;
                case LintSeverity::Deny:
                    std::cout << color_red() << " deny  " << color_reset();
                    break;
                case LintSeverity::Allow:
                    std::cout << color_dim() << " allow " << color_reset();
                    break;
                default:
                    std::cout << "       ";
                    break;
            }

            std::cout << rule.description;
            if (!rule.enabled) {
                std::cout << color_dim() << " (disabled by default)" << color_reset();
            }
            std::cout << "\n";
        }
        return 0;
    }

    if (files.empty()) {
        std::cerr << "Error: No input files specified\n";
        std::cerr << "Usage: mana lint <FILES>...\n";
        std::cerr << "Try 'mana lint --help' for more information.\n";
        return 1;
    }

    Linter linter(config);
    auto messages = linter.lint_files(files);
    linter.print_results(messages, std::cout);

    // Return error code if there were any errors
    return linter.error_count() > 0 ? 1 : 0;
}

} // namespace mana::lint
