#pragma once

#include <string>

#include "Type.h"
#include "AstDeclarations.h"

namespace mana::frontend {

    enum class SymbolKind {
        Var,
        Func
    };

    struct Symbol {
        SymbolKind kind;
        std::string name;
        Type type;
        int line;
        int column;

        // 🔑 only valid if kind == Func
        const AstFuncDecl* func_decl = nullptr;

        Symbol(
            SymbolKind k,
            const std::string& n,
            const Type& t,
            int l,
            int c,
            const AstFuncDecl* fn = nullptr
        )
            : kind(k), name(n), type(t), line(l), column(c), func_decl(fn) {
        }
    };

} // namespace mana::frontend
