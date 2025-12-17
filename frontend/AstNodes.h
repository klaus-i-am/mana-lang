#pragma once

#include <cstdint>

namespace mana::frontend {

    enum class NodeKind {
        Module,
        ImportDecl,
        UseDecl,

        // Decls
        FunctionDecl,
        GlobalVarDecl,
        StructDecl,
        EnumDecl,
        TraitDecl,
        ImplDecl,
        TypeAliasDecl,

        // Stmts
        BlockStmt,
        IfStmt,
        WhileStmt,
        ForStmt,
        ForInStmt,
        BreakStmt,
        ContinueStmt,
        DeferStmt,
        AssignStmt,
        VarDeclStmt,
        ScopeStmt,
        ReturnStmt,
        ExprStmt,
        LoopStmt,

        // Exprs
        IdentifierExpr,
        LiteralExpr,
        CallExpr,
        MethodCallExpr,
        BinaryExpr,
        UnaryExpr,
        IndexExpr,
        ArrayLiteralExpr,
        MemberAccessExpr,
        StructLiteralExpr,
        ScopeAccessExpr,
        SelfExpr,
        MatchExpr,
        ClosureExpr,
        TryExpr,    // expr? for error propagation
        OptionalChainExpr,  // expr?.field or expr?.method() for optional chaining
        NullCoalesceExpr,   // expr ?? default for null coalescing
        AwaitExpr,  // expr.await for async operations
        RangeExpr,  // start..end or start..=end
        TupleExpr,      // (expr1, expr2, ...)
        TupleIndexExpr, // tuple.0, tuple.1, etc.
        FStringExpr,    // f"Hello {name}!" interpolated string
        NoneExpr,       // None literal for Option types
        OptionPattern,  // Some(x), None, Ok(x), Err(e) patterns in match
        EnumPattern,    // Enum::Variant(x, y) or Enum::Variant { field: x } destructuring
        CastExpr,       // expr as Type
        IfExpr,         // if cond { expr } else { expr }
        SliceExpr,      // expr[start..end] slice syntax
    };

    struct AstNode {
        NodeKind kind;
        int line;
        int column;

        explicit AstNode(NodeKind k, int l = 0, int c = 0)
            : kind(k), line(l), column(c) {
        }

        virtual ~AstNode() = default;
    };

} // namespace mana::frontend
