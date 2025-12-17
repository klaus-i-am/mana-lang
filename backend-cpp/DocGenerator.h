#pragma once
#include <string>
#include <sstream>
#include "../frontend/AstModule.h"
#include "../frontend/AstDecl.h"
#include "../frontend/AstStatements.h"
#include "../frontend/AstExpressions.h"

namespace mana::backend {

    // Documentation generator that produces Markdown documentation from AST
    class DocGenerator {
    public:
        std::string generate(const mana::frontend::AstModule& mod);

    private:
        std::stringstream out_;

        void emit_module(const mana::frontend::AstModule& mod);
        void emit_function(const mana::frontend::AstFuncDecl& fn);
        void emit_struct(const mana::frontend::AstStructDecl& s);
        void emit_enum(const mana::frontend::AstEnumDecl& e);
        void emit_trait(const mana::frontend::AstTraitDecl& t);
        void emit_type_alias(const mana::frontend::AstTypeAliasDecl& t);

        std::string escape_markdown(const std::string& s);
    };

} // namespace mana::backend
