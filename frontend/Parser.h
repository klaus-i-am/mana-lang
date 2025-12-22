#pragma once
#include <memory>
#include <vector>
#include "Token.h"
#include "Diagnostic.h"
#include "AstModule.h"
#include "AstStatements.h"
#include "AstExpressions.h"
#include "Lexer.h"

namespace mana::frontend {

    class Parser {
    public:
        Parser(const std::vector<Token>& tokens, DiagnosticEngine& diag);

        std::unique_ptr<AstModule> parse_module();

    private:
        const std::vector<Token>& tokens_;
        DiagnosticEngine& diag_;
        size_t current_ = 0;

        const Token& peek() const;
        const Token& previous() const;
        bool is_at_end() const;

        bool check(TokenKind kind) const;
        bool check_next(TokenKind kind) const;  // Look ahead to next token
        const Token& advance();
        bool match(TokenKind kind);
        bool expect(TokenKind kind, const char* msg);
        void optional_semicolon();  // Consume semicolon if present (vNext: semicolons optional)
        void synchronize();
        void synchronize_statement();  // Synchronize within a block

        // Decls
        std::unique_ptr<AstDecl> parse_declaration();
        std::unique_ptr<AstDecl> parse_import_decl();
        std::unique_ptr<AstDecl> parse_use_decl(bool is_pub = false);
        std::unique_ptr<AstDecl> parse_function_decl(bool is_pub = false, bool is_async = false, bool is_test = false, bool is_extern = false);
        std::unique_ptr<AstDecl> parse_struct_decl(bool is_pub = false);
        std::unique_ptr<AstDecl> parse_enum_decl(bool is_pub = false, bool declared_as_variant = false);
        std::unique_ptr<AstDecl> parse_global_var_decl();
        std::unique_ptr<AstDecl> parse_trait_decl(bool is_pub = false);
        std::unique_ptr<AstDecl> parse_impl_decl();
        std::unique_ptr<AstDecl> parse_type_alias_decl(bool is_pub = false);

        // Statements
        std::unique_ptr<AstBlockStmt> parse_block();
        std::unique_ptr<AstStmt> parse_statement();

        std::unique_ptr<AstStmt> parse_var_decl_statement();
        std::unique_ptr<AstStmt> parse_destructure_statement(bool is_struct);
        std::unique_ptr<AstStmt> parse_let_statement();
        std::unique_ptr<AstStmt> parse_const_statement();
        std::unique_ptr<AstStmt> parse_assign_statement();
        std::unique_ptr<AstStmt> parse_scope_statement();
        std::unique_ptr<AstStmt> parse_defer_statement();
        std::unique_ptr<AstStmt> parse_return_statement();
        std::unique_ptr<AstStmt> parse_if_statement();
        std::unique_ptr<AstStmt> parse_while_statement();
        std::unique_ptr<AstStmt> parse_loop_statement();
        std::unique_ptr<AstStmt> parse_for_statement();
        std::unique_ptr<AstStmt> parse_break_statement();
        std::unique_ptr<AstStmt> parse_continue_statement();
        std::unique_ptr<AstStmt> parse_expression_statement();

        // Expressions (precedence)
        std::unique_ptr<AstExpr> parse_expression();
        std::unique_ptr<AstExpr> parse_or_control_flow();  // expr or return/break/block (vNext)
        std::unique_ptr<AstExpr> parse_null_coalesce();
        std::unique_ptr<AstExpr> parse_logical_or();
        std::unique_ptr<AstExpr> parse_logical_and();
        std::unique_ptr<AstExpr> parse_bitwise_or();
        std::unique_ptr<AstExpr> parse_bitwise_xor();
        std::unique_ptr<AstExpr> parse_bitwise_and();
        std::unique_ptr<AstExpr> parse_equality();
        std::unique_ptr<AstExpr> parse_relational();
        std::unique_ptr<AstExpr> parse_shift();
        std::unique_ptr<AstExpr> parse_additive();
        std::unique_ptr<AstExpr> parse_power();
        std::unique_ptr<AstExpr> parse_multiplicative();
        std::unique_ptr<AstExpr> parse_unary();
        std::unique_ptr<AstExpr> parse_postfix();
        std::unique_ptr<AstExpr> parse_primary();
        std::unique_ptr<AstExpr> parse_match_expression(bool is_when_style = false);
        std::unique_ptr<AstExpr> parse_closure_expression();
        std::unique_ptr<AstExpr> parse_closure_with_captures();

        std::string parse_type_name();
    };

} // namespace mana::frontend
