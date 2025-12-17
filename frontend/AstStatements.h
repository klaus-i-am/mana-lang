#pragma once

#include <memory>
#include <vector>
#include <string>

#include "AstNodes.h"

namespace mana::frontend {

    struct AstVarDeclStmt;
    struct AstBlockStmt;
    struct AstExpr;  // Forward declare for AstBreakStmt

    struct AstStmt : AstNode {
        explicit AstStmt(NodeKind k, int line = 0, int column = 0) : AstNode(k, line, column) {}
        virtual ~AstStmt() = default;
    };

    struct AstBlockStmt : AstStmt {
        std::vector<std::unique_ptr<AstStmt>> statements;
        AstBlockStmt(int line = 0, int column = 0) : AstStmt(NodeKind::BlockStmt, line, column) {}
    };

    struct AstVarDeclStmt : AstStmt {
        std::string name;
        std::string type_name;
        std::unique_ptr<AstNode> init_expr;
        bool is_mutable = true;
        AstVarDeclStmt(int line = 0, int column = 0) : AstStmt(NodeKind::VarDeclStmt, line, column) {}
        AstVarDeclStmt(std::string n, std::string t, std::unique_ptr<AstNode> init, bool mut = true, int line = 0, int column = 0)
            : AstStmt(NodeKind::VarDeclStmt, line, column), name(std::move(n)), type_name(std::move(t)), init_expr(std::move(init)), is_mutable(mut) {}
    };

    struct AstAssignStmt : AstStmt {
        std::string target_name;
        std::unique_ptr<AstNode> target_expr;
        std::unique_ptr<AstNode> value;
        std::string op = "=";
        AstAssignStmt(int line = 0, int column = 0) : AstStmt(NodeKind::AssignStmt, line, column) {}
        AstAssignStmt(std::string name, std::unique_ptr<AstNode> v, int line = 0, int column = 0)
            : AstStmt(NodeKind::AssignStmt, line, column), target_name(std::move(name)), value(std::move(v)) {}
        bool is_complex_target() const { return target_expr != nullptr; }
        bool is_compound() const { return op != "="; }
    };

    struct AstIfStmt : AstStmt {
        std::unique_ptr<AstNode> condition;
        std::unique_ptr<AstStmt> then_block;
        std::unique_ptr<AstStmt> else_block;
        bool is_if_let = false;
        std::string pattern_kind;
        std::string pattern_var;
        std::unique_ptr<AstNode> pattern_expr;
        AstIfStmt(int line = 0, int column = 0) : AstStmt(NodeKind::IfStmt, line, column) {}
        AstIfStmt(std::unique_ptr<AstNode> cond, std::unique_ptr<AstStmt> then_b, std::unique_ptr<AstStmt> else_b, int line = 0, int column = 0)
            : AstStmt(NodeKind::IfStmt, line, column), condition(std::move(cond)), then_block(std::move(then_b)), else_block(std::move(else_b)) {}
    };

    struct AstWhileStmt : AstStmt {
        std::unique_ptr<AstNode> condition;
        std::unique_ptr<AstStmt> body;
        bool is_while_let = false;
        std::string pattern_kind;
        std::string pattern_var;
        std::unique_ptr<AstNode> pattern_expr;
        AstWhileStmt(int line = 0, int column = 0) : AstStmt(NodeKind::WhileStmt, line, column) {}
        AstWhileStmt(std::unique_ptr<AstNode> cond, std::unique_ptr<AstStmt> b, int line = 0, int column = 0)
            : AstStmt(NodeKind::WhileStmt, line, column), condition(std::move(cond)), body(std::move(b)) {}
    };

    struct AstForStmt : AstStmt {
        std::unique_ptr<AstStmt> init;
        std::unique_ptr<AstNode> condition;
        std::unique_ptr<AstStmt> increment;
        std::unique_ptr<AstStmt> body;
        AstForStmt(std::unique_ptr<AstStmt> i, std::unique_ptr<AstNode> cond, std::unique_ptr<AstStmt> inc, std::unique_ptr<AstStmt> b, int line = 0, int column = 0)
            : AstStmt(NodeKind::ForStmt, line, column), init(std::move(i)), condition(std::move(cond)), increment(std::move(inc)), body(std::move(b)) {}
    };

    struct AstLoopStmt : AstStmt {
        std::unique_ptr<AstStmt> body;
        AstLoopStmt(int line = 0, int column = 0) : AstStmt(NodeKind::LoopStmt, line, column) {}
        explicit AstLoopStmt(std::unique_ptr<AstStmt> b, int line = 0, int column = 0) : AstStmt(NodeKind::LoopStmt, line, column), body(std::move(b)) {}
    };

    struct AstBreakStmt : AstStmt {
        std::unique_ptr<AstExpr> value;  // Optional break value
        AstBreakStmt(int line = 0, int column = 0) : AstStmt(NodeKind::BreakStmt, line, column) {}
    };

    struct AstContinueStmt : AstStmt {
        AstContinueStmt(int line = 0, int column = 0) : AstStmt(NodeKind::ContinueStmt, line, column) {}
    };

    struct AstReturnStmt : AstStmt {
        std::unique_ptr<AstNode> value;
        AstReturnStmt(int line = 0, int column = 0) : AstStmt(NodeKind::ReturnStmt, line, column) {}
        explicit AstReturnStmt(std::unique_ptr<AstNode> v, int line = 0, int column = 0) : AstStmt(NodeKind::ReturnStmt, line, column), value(std::move(v)) {}
    };

    struct AstDeferStmt : AstStmt {
        std::unique_ptr<AstStmt> body;
        AstDeferStmt(int line = 0, int column = 0) : AstStmt(NodeKind::DeferStmt, line, column) {}
        explicit AstDeferStmt(std::unique_ptr<AstStmt> b, int line = 0, int column = 0) : AstStmt(NodeKind::DeferStmt, line, column), body(std::move(b)) {}
    };

    struct AstScopeStmt : AstStmt {
        std::string name;
        std::unique_ptr<AstNode> init_expr;
        std::unique_ptr<AstStmt> body;
        AstScopeStmt(int line = 0, int column = 0) : AstStmt(NodeKind::ScopeStmt, line, column) {}
        explicit AstScopeStmt(std::unique_ptr<AstStmt> b, int line = 0, int column = 0) : AstStmt(NodeKind::ScopeStmt, line, column), body(std::move(b)) {}
    };

    struct AstExprStmt : AstStmt {
        std::unique_ptr<AstNode> expr;
        AstExprStmt(int line = 0, int column = 0) : AstStmt(NodeKind::ExprStmt, line, column) {}
        explicit AstExprStmt(std::unique_ptr<AstNode> e, int line = 0, int column = 0) : AstStmt(NodeKind::ExprStmt, line, column), expr(std::move(e)) {}
    };

    struct DestructureBinding {
        std::string name;
        std::string field_name;
        int line = 0;
        int column = 0;
    };

    struct AstDestructureStmt : AstStmt {
        std::vector<DestructureBinding> bindings;
        std::string type_name;
        std::unique_ptr<AstNode> init_expr;
        bool is_struct = true;
        bool is_tuple = false;
        AstDestructureStmt(int line = 0, int column = 0) : AstStmt(NodeKind::VarDeclStmt, line, column) {}
    };

    struct AstForInStmt : AstStmt {
        std::string var_name;                    // Single variable name for simple iteration
        std::vector<std::string> var_names;      // Multiple names for destructuring: (key, value)
        bool is_destructure = false;             // True if using destructuring pattern
        std::unique_ptr<AstNode> iterable;
        std::unique_ptr<AstStmt> body;
        AstForInStmt(int line = 0, int column = 0) : AstStmt(NodeKind::ForInStmt, line, column) {}
    };

} // namespace mana::frontend
