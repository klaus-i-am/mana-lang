#pragma once
#include <string_view>
#include "Token.h"

namespace mana::frontend {

    inline bool keyword_kind(std::string_view s, TokenKind& out) {
        if (s == "module") { out = TokenKind::KwModule; return true; }
        if (s == "import") { out = TokenKind::KwImport; return true; }
        if (s == "fn") { out = TokenKind::KwFn; return true; }
        if (s == "struct") { out = TokenKind::KwStruct; return true; }
        if (s == "enum") { out = TokenKind::KwEnum; return true; }
        if (s == "let") { out = TokenKind::KwLet; return true; }
        if (s == "return") { out = TokenKind::KwReturn; return true; }
        if (s == "if") { out = TokenKind::KwIf; return true; }
        if (s == "else") { out = TokenKind::KwElse; return true; }
        if (s == "while") { out = TokenKind::KwWhile; return true; }
        if (s == "for") { out = TokenKind::KwFor; return true; }
        if (s == "break") { out = TokenKind::KwBreak; return true; }
        if (s == "continue") { out = TokenKind::KwContinue; return true; }
        if (s == "defer") { out = TokenKind::KwDefer; return true; }
        if (s == "scope") { out = TokenKind::KwScope; return true; }
        if (s == "true") { out = TokenKind::KwTrue; return true; }
        if (s == "false") { out = TokenKind::KwFalse; return true; }
        if (s == "self") { out = TokenKind::KwSelf; return true; }
        if (s == "match") { out = TokenKind::KwMatch; return true; }
        if (s == "trait") { out = TokenKind::KwTrait; return true; }
        if (s == "impl") { out = TokenKind::KwImpl; return true; }
        if (s == "dyn") { out = TokenKind::KwDyn; return true; }
        if (s == "in") { out = TokenKind::KwIn; return true; }
        if (s == "mut") { out = TokenKind::KwMut; return true; }
        if (s == "move") { out = TokenKind::KwMove; return true; }
        if (s == "const") { out = TokenKind::KwConst; return true; }
        if (s == "None") { out = TokenKind::KwNone; return true; }
        if (s == "type") { out = TokenKind::KwType; return true; }
        if (s == "loop") { out = TokenKind::KwLoop; return true; }
        if (s == "as") { out = TokenKind::KwAs; return true; }
        if (s == "use") { out = TokenKind::KwUse; return true; }
        if (s == "pub") { out = TokenKind::KwPub; return true; }
        if (s == "from") { out = TokenKind::KwFrom; return true; }
        if (s == "async") { out = TokenKind::KwAsync; return true; }
        if (s == "await") { out = TokenKind::KwAwait; return true; }
        if (s == "where") { out = TokenKind::KwWhere; return true; }
        if (s == "static") { out = TokenKind::KwStatic; return true; }
        return false;
    }

} // namespace mana::frontend
