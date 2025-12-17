#pragma once
#include <ostream>
#include "AstModule.h"
#include "AstExpressions.h"
#include "AstStatements.h"

namespace mana::frontend {

    class AstPrinter {
    public:
        void print(const AstModule* m, std::ostream& out);

    private:
        void indent(std::ostream& out, int n);
        void print_decl(const AstDecl* d, std::ostream& out, int ind);
        void print_stmt(const AstStmt* s, std::ostream& out, int ind);
        void print_expr(const AstExpr* e, std::ostream& out, int ind);
    };

} // namespace mana::frontend
