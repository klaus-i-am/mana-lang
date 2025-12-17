#pragma once
#include "Token.h"
#include <iostream>

namespace mana::frontend {

    inline void print_token(const Token& tok) {
        std::cout << "Token(" << (int)tok.kind << ", \"" << tok.lexeme
            << "\" at " << tok.line << ":" << tok.column << ")\n";
    }

} // namespace mana::frontend
