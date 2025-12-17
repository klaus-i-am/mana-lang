#pragma once
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include "../frontend/AstModule.h"
#include "../frontend/AstExpressions.h"
#include "../frontend/AstStatements.h"
#include "../frontend/AstDeclarations.h"

namespace mana::backend {

    class CppEmitter {
    public:
        void emit(const mana::frontend::AstModule* m, std::ostream& out, bool test_mode = false);

    private:
        void emit_stmt(const mana::frontend::AstStmt* s, std::ostream& out, int ind);
        void emit_expr(const mana::frontend::AstExpr* e, std::ostream& out);
        void emit_capture_list(const mana::frontend::AstClosureExpr* cl, std::ostream& out);
        void indent(std::ostream& out, int n);
        void extract_try_exprs(const mana::frontend::AstExpr* e, std::ostream& out, int ind);
        std::unordered_map<const mana::frontend::AstTryExpr*, int> try_expr_ids_;
        std::unordered_map<std::string, const mana::frontend::AstStructDecl*> struct_types_;  // For default values
        std::unordered_set<std::string> adt_enums_;  // Enums with data variants (ADT)
        std::unordered_set<std::string> impl_methods_;  // TypeName_methodName for impl blocks
        bool test_mode_ = false;
        std::vector<std::string> test_function_names_;
    };

} // namespace mana::backend
