#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AstNodes.h"

namespace mana::frontend {

    struct AstExpr : AstNode {
        explicit AstExpr(NodeKind k, int line = 0, int column = 0) : AstNode(k, line, column) {}
        virtual ~AstExpr() = default;
    };

    struct AstIdentifierExpr : AstExpr {
        std::string name;

        explicit AstIdentifierExpr(std::string n, int line = 0, int column = 0)
            : AstExpr(NodeKind::IdentifierExpr, line, column),
            name(std::move(n)) {
        }
    };

    struct AstLiteralExpr : AstExpr {
        std::string value;
        bool is_string = false;
        bool is_char = false;

        explicit AstLiteralExpr(std::string v, bool is_str = false, bool is_ch = false, int line = 0, int column = 0)
            : AstExpr(NodeKind::LiteralExpr, line, column),
            value(std::move(v)), is_string(is_str), is_char(is_ch) {
        }
    };

    struct AstCallExpr : AstExpr {
        std::string func_name;
        std::vector<std::unique_ptr<AstNode>> args;
        std::vector<std::string> arg_names;  // Named arguments: empty string for positional args

        explicit AstCallExpr(std::string name, int line = 0, int column = 0)
            : AstExpr(NodeKind::CallExpr, line, column),
            func_name(std::move(name)) {
        }
    };

    // Method call: object.method(args)
    struct AstMethodCallExpr : AstExpr {
        std::unique_ptr<AstExpr> object;
        std::string method_name;
        std::vector<std::unique_ptr<AstNode>> args;
        std::vector<std::string> arg_names;  // Named arguments: empty string for positional args
        std::string object_type;  // Filled by semantic analyzer for code generation

        AstMethodCallExpr(std::string method, int line = 0, int column = 0)
            : AstExpr(NodeKind::MethodCallExpr, line, column),
            method_name(std::move(method)) {
        }
    };

    struct AstBinaryExpr : AstExpr {
        std::string op;
        std::unique_ptr<AstNode> left;
        std::unique_ptr<AstNode> right;

        AstBinaryExpr(std::string o, int line = 0, int column = 0)
            : AstExpr(NodeKind::BinaryExpr, line, column),
            op(std::move(o)) {
        }

        AstBinaryExpr(
            std::string o,
            std::unique_ptr<AstNode> l,
            std::unique_ptr<AstNode> r,
            int line = 0,
            int column = 0)
            : AstExpr(NodeKind::BinaryExpr, line, column),
            op(std::move(o)),
            left(std::move(l)),
            right(std::move(r)) {
        }
    };

    struct AstUnaryExpr : AstExpr {
        std::string op;
        std::unique_ptr<AstNode> right;

        AstUnaryExpr(std::string o, int line = 0, int column = 0)
            : AstExpr(NodeKind::UnaryExpr, line, column),
            op(std::move(o)) {
        }

        AstUnaryExpr(std::string o, std::unique_ptr<AstNode> r, int line = 0, int column = 0)
            : AstExpr(NodeKind::UnaryExpr, line, column),
            op(std::move(o)),
            right(std::move(r)) {
        }
    };

    struct AstIndexExpr : AstExpr {
        std::unique_ptr<AstExpr> base;
        std::unique_ptr<AstExpr> index;

        AstIndexExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::IndexExpr, line, column) {
        }
    };

    // Slice expression: expr[start..end] or expr[start..=end]
    struct AstSliceExpr : AstExpr {
        std::unique_ptr<AstExpr> base;      // The collection being sliced
        std::unique_ptr<AstExpr> start;     // Start index (optional)
        std::unique_ptr<AstExpr> end;       // End index (optional)
        bool inclusive;                      // true for ..=, false for ..

        AstSliceExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::SliceExpr, line, column), inclusive(false) {
        }
    };

    struct AstArrayLiteralExpr : AstExpr {
        std::vector<std::unique_ptr<AstExpr>> elements;
        // For fill syntax: [value; count]
        std::unique_ptr<AstExpr> fill_value;
        std::unique_ptr<AstExpr> fill_count;

        AstArrayLiteralExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::ArrayLiteralExpr, line, column) {
        }

        bool is_fill() const { return fill_value != nullptr; }
    };

    struct AstMemberAccessExpr : AstExpr {
        std::unique_ptr<AstExpr> object;
        std::string member_name;

        AstMemberAccessExpr(std::string member, int line = 0, int column = 0)
            : AstExpr(NodeKind::MemberAccessExpr, line, column),
            member_name(std::move(member)) {
        }
    };

    struct AstStructFieldInit {
        std::string field_name;  // empty for positional init
        std::unique_ptr<AstExpr> value;
        int line = 0;
        int column = 0;
    };

    struct AstStructLiteralExpr : AstExpr {
        std::string type_name;
        std::vector<AstStructFieldInit> fields;
        bool is_named;  // true if using named fields (Vec3{x: 1}), false for positional

        AstStructLiteralExpr(std::string name, int line = 0, int column = 0)
            : AstExpr(NodeKind::StructLiteralExpr, line, column),
            type_name(std::move(name)),
            is_named(false) {
        }
    };

    // For Enum::Variant or Module::Type access
    struct AstScopeAccessExpr : AstExpr {
        std::string scope_name;   // e.g., "Color" in Color::Red
        std::string member_name;  // e.g., "Red" in Color::Red

        AstScopeAccessExpr(std::string scope, std::string member, int line = 0, int column = 0)
            : AstExpr(NodeKind::ScopeAccessExpr, line, column),
            scope_name(std::move(scope)),
            member_name(std::move(member)) {
        }
    };

    // Self reference in methods
    struct AstSelfExpr : AstExpr {
        AstSelfExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::SelfExpr, line, column) {
        }
    };

    // Match arm: pattern => result (supports or-patterns like 1 | 2 | 3)
    struct AstMatchArm {
        std::vector<std::unique_ptr<AstExpr>> patterns;  // Multiple patterns (or-patterns)
        std::unique_ptr<AstExpr> guard;    // Optional guard clause (if condition)
        std::unique_ptr<AstExpr> result;   // The result expression (for expression arms)
        std::unique_ptr<AstBlockStmt> result_block;  // Block body (for statement arms)
        std::string binding;               // Binding variable name (for patterns like n if n < 5)
        int line = 0;
        int column = 0;

        bool has_block() const { return result_block != nullptr; }
        bool has_binding() const { return !binding.empty(); }
    };

    // Match expression: match value { pattern => expr, ... }
    struct AstMatchExpr : AstExpr {
        std::unique_ptr<AstExpr> value;       // The value being matched
        std::vector<AstMatchArm> arms;        // Match arms
        bool has_default = false;             // True if _ wildcard is present

        AstMatchExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::MatchExpr, line, column) {
        }
    };

    // Closure parameter
    struct AstClosureParam {
        std::string name;
        std::string type_name;  // Empty if type is inferred
        int line = 0;
        int column = 0;
    };

    // Capture specification for closures
    enum class CaptureMode {
        ByRef,      // &x - capture by reference
        ByValue,    // =x or x - capture by value (copy)
        ByMove      // move x - capture by move
    };

    struct CaptureSpec {
        std::string name;       // Variable name to capture (empty for default capture [&] or [=])
        CaptureMode mode = CaptureMode::ByRef;
    };

    // Closure expression: |params| expr or |params| { block }
    // Can have explicit captures: [x, &y, move z]|params| expr
    // Or move semantics: move |params| expr
    struct AstClosureExpr : AstExpr {
        std::vector<AstClosureParam> params;
        std::string return_type;              // Empty if type is inferred
        std::unique_ptr<AstExpr> body_expr;   // For single-expression closures
        std::unique_ptr<AstBlockStmt> body_block;  // For block closures
        bool captures_by_ref = true;          // [&] vs [=] in C++ (used when no explicit captures)
        std::vector<CaptureSpec> captures;    // Explicit capture list (empty = default capture)
        bool has_explicit_captures = false;   // True if [...] capture list was specified

        AstClosureExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::ClosureExpr, line, column) {
        }

        bool has_block() const { return body_block != nullptr; }
    };

    // Try expression: expr? for error propagation
    // Unwraps Result/Option or propagates the error/None to caller
    struct AstTryExpr : AstExpr {
        std::unique_ptr<AstExpr> operand;  // The expression being tried

        AstTryExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::TryExpr, line, column) {
        }
    };

    // Optional chaining expression: expr?.field or expr?.method()
    // Returns None if expr is None, otherwise accesses field/method on unwrapped value
    struct AstOptionalChainExpr : AstExpr {
        std::unique_ptr<AstExpr> object;       // The Option being accessed
        std::string member_name;               // Field or method name
        bool is_method_call = false;           // true if expr?.method()
        std::vector<std::unique_ptr<AstNode>> args;  // Method arguments (if is_method_call)
        std::vector<std::string> arg_names;    // Named argument names

        AstOptionalChainExpr(std::string member, int line = 0, int column = 0)
            : AstExpr(NodeKind::OptionalChainExpr, line, column),
              member_name(std::move(member)) {
        }
    };

    // Null coalescing expression: opt ?? default
    struct AstNullCoalesceExpr : AstExpr {
        std::unique_ptr<AstExpr> option_expr;   // The Option being checked
        std::unique_ptr<AstExpr> default_expr;  // Default value if None

        AstNullCoalesceExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::NullCoalesceExpr, line, column) {
        }
    };

    // Await expression: expr.await for async operations
    struct AstAwaitExpr : AstExpr {
        std::unique_ptr<AstExpr> operand;  // The future being awaited

        AstAwaitExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::AwaitExpr, line, column) {
        }
    };

    // Range expression: start..end or start..=end
    struct AstRangeExpr : AstExpr {
        std::unique_ptr<AstExpr> start;
        std::unique_ptr<AstExpr> end;
        bool inclusive = false;  // true for ..=, false for ..

        AstRangeExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::RangeExpr, line, column) {
        }
    };

    // Tuple literal: (expr1, expr2, ...)
    struct AstTupleExpr : AstExpr {
        std::vector<std::unique_ptr<AstExpr>> elements;

        AstTupleExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::TupleExpr, line, column) {
        }
    };

    // Tuple index access: tuple.0, tuple.1, etc.
    struct AstTupleIndexExpr : AstExpr {
        std::unique_ptr<AstExpr> tuple;
        int index;

        AstTupleIndexExpr(int idx, int line = 0, int column = 0)
            : AstExpr(NodeKind::TupleIndexExpr, line, column),
            index(idx) {
        }
    };

    // F-string part: either a literal string segment or an expression
    struct AstFStringPart {
        bool is_expr = false;
        std::string literal;                 // For literal parts
        std::unique_ptr<AstExpr> expr;       // For expression parts
        std::string format_spec;             // Format specifier (e.g., ".2f", "04d")
    };

    // Interpolated string: f"Hello {name}, you are {age} years old!"
    struct AstFStringExpr : AstExpr {
        std::vector<AstFStringPart> parts;

        AstFStringExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::FStringExpr, line, column) {
        }
    };

    // None literal for Option types
    struct AstNoneExpr : AstExpr {
        AstNoneExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::NoneExpr, line, column) {
        }
    };

    // Pattern for Option/Result matching: Some(x), None, Ok(x), Err(e)
    struct AstOptionPattern : AstExpr {
        std::string pattern_kind;  // "Some", "Ok", "Err", "None"
        std::string binding;       // Variable to bind (empty for None)

        AstOptionPattern(std::string kind, std::string bind, int line = 0, int column = 0)
            : AstExpr(NodeKind::OptionPattern, line, column),
              pattern_kind(std::move(kind)),
              binding(std::move(bind)) {
        }
    };

    // Pattern for destructuring ADT enum variants: Enum::Variant(x, y) or Enum::Variant { field: x }
    struct AstEnumPattern : AstExpr {
        std::string enum_name;                     // The enum type name (e.g., "Message")
        std::string variant_name;                  // The variant name (e.g., "Move")
        std::vector<std::string> bindings;         // Tuple bindings: ["x", "y"] for Variant(x, y)
        std::vector<std::pair<std::string, std::string>> field_bindings;  // Struct bindings: [("field", "x")]
        bool is_tuple_pattern = true;              // true for (x, y), false for { field: x }

        AstEnumPattern(std::string ename, std::string vname, int line = 0, int column = 0)
            : AstExpr(NodeKind::EnumPattern, line, column),
              enum_name(std::move(ename)),
              variant_name(std::move(vname)) {
        }

        bool is_unit_pattern() const { return bindings.empty() && field_bindings.empty(); }
    };

    // Cast expression: expr as Type
    struct AstCastExpr : AstExpr {
        std::unique_ptr<AstExpr> operand;  // The expression being cast
        std::string target_type;           // The type to cast to

        AstCastExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::CastExpr, line, column) {
        }
    };

    // If expression: if cond { then_expr } else { else_expr }
    struct AstIfExpr : AstExpr {
        std::unique_ptr<AstExpr> condition;
        std::unique_ptr<AstExpr> then_expr;
        std::unique_ptr<AstExpr> else_expr;

        AstIfExpr(int line = 0, int column = 0)
            : AstExpr(NodeKind::IfExpr, line, column) {}
    };

} // namespace mana::frontend