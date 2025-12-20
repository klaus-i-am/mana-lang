#pragma once
#include <unordered_map>
#include <vector>
#include <set>
#include "AstModule.h"
#include "AstExpressions.h"
#include "AstStatements.h"
#include "AstDeclarations.h"
#include "Diagnostic.h"
#include "Type.h"
#include "Symbol.h"

namespace mana::frontend {

    class SemanticAnalyzer {
    public:
        explicit SemanticAnalyzer(DiagnosticEngine& diag);

        void analyze(AstModule* module);

    private:
        DiagnosticEngine& diag_;

        std::vector<std::unordered_map<std::string, Symbol>> scopes_;
        std::unordered_map<std::string, AstStructDecl*> struct_types_;
        std::unordered_map<std::string, AstEnumDecl*> enum_types_;
        std::unordered_map<std::string, AstTraitDecl*> trait_types_;
        std::unordered_map<std::string, AstFuncDecl*> func_decls_;  // For named argument validation
        std::vector<AstFuncDecl*> test_functions_;  // Functions marked with #[test]
        std::unordered_map<std::string, std::string> type_aliases_;  // alias -> target type
        std::vector<std::string> imported_modules_;  // Modules imported via 'use'
        Type current_return_type_ = Type::unknown();
        Type current_receiver_type_ = Type::unknown();  // For methods
        int loop_depth_ = 0;
        std::string current_module_;  // Current module being analyzed

        // Generic type tracking
        std::unordered_map<std::string, Type> type_param_bindings_;  // T -> i32 during instantiation
        std::vector<std::string> current_type_params_;  // Type params in scope

        // Trait implementation tracking: type_name -> set of implemented traits
        std::unordered_map<std::string, std::set<std::string>> type_trait_impls_;

        // scope
        void push_scope();
        void pop_scope();
        bool declare(const std::string& name, const Symbol& sym);
        Symbol* lookup(const std::string& name);
        bool check_visibility(const Symbol* sym, int line, int col);
        bool type_implements_trait(const std::string& type_name, const std::string& trait_name);
        Type instantiate_generic(const std::string& generic_type, const std::vector<Type>& type_args);
        bool check_trait_bounds(const std::string& type_param, const Type& concrete_type,
                               const std::vector<std::string>& required_traits, int line, int col);

        // built-ins
        void register_builtins();
        std::unordered_map<std::string, bool> builtin_functions_;

        // visitors
        void register_declaration(AstDecl* d);  // First pass: register declarations
        void visit_decl(AstDecl* d);            // Second pass: analyze bodies
        void visit_stmt(AstStmt* s);
        Type visit_expr(AstExpr* e);

        // helpers
        Type parse_type_name(const std::string& name);
        std::string infer_type_name(const Type& t);  // Convert Type back to string for AST
        bool is_numeric(const Type& t);

        // "Did you mean?" suggestions
        static size_t levenshtein_distance(const std::string& a, const std::string& b);
        std::string find_similar_name(const std::string& name, int max_distance = 3);
        std::vector<std::string> get_all_known_names();

        // Unused variable tracking
        std::unordered_map<std::string, bool> variable_used_;
        std::unordered_map<std::string, std::pair<int, int>> variable_location_;  // name -> (line, column)
        void mark_variable_used(const std::string& name);
        void check_unused_variables();

        // Constant folding
        struct ConstValue {
            enum { Int, Float, Bool, String, None } kind = None;
            int64_t int_val = 0;
            double float_val = 0.0;
            bool bool_val = false;
            std::string string_val;
        };
        bool try_fold_constant(AstExpr* e, ConstValue& result);
        void fold_constants_in_module(AstModule* module);
        void fold_constants_in_stmt(AstStmt* s);
        void fold_constants_in_expr(std::unique_ptr<AstNode>& e);
    };

} // namespace mana::frontend
