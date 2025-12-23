#pragma once
#include <string>
#include <ostream>
#include "../../frontend/AstModule.h"
#include "../../frontend/AstExpressions.h"
#include "../../frontend/AstStatements.h"
#include "../../frontend/AstDeclarations.h"

namespace mana::fmt {

    struct FormatConfig {
        int indent_width = 4;
        bool use_tabs = false;
        int max_line_length = 100;
        bool trailing_commas = true;
        bool space_after_colon = true;
        bool space_around_operators = true;
    };

    class Formatter {
    public:
        explicit Formatter(const FormatConfig& config = {});

        std::string format(const frontend::AstModule* module);
        void format(const frontend::AstModule* module, std::ostream& out);

    private:
        void emit_decl(const frontend::AstDecl* d, std::ostream& out);
        void emit_stmt(const frontend::AstStmt* s, std::ostream& out, int indent);
        void emit_stmt_inline(const frontend::AstStmt* s, std::ostream& out);  // For for-loop parts
        void emit_expr(const frontend::AstExpr* e, std::ostream& out);
        void emit_indent(std::ostream& out, int level);
        void emit_type(const std::string& type, std::ostream& out);

        FormatConfig config_;
        int current_indent_ = 0;
    };

} // namespace mana::fmt
