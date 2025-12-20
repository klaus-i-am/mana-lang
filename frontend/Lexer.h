#pragma once
#include <string>
#include <vector>
#include "Token.h"

namespace mana::frontend {

    class Lexer {
    public:
        explicit Lexer(std::string src);

        std::vector<Token> tokenize();

    private:
        std::string src_;
        std::vector<Token> tokens_;

        size_t current_ = 0;
        int line_ = 1;
        int col_ = 1;

        bool is_at_end() const;
        char peek_char() const;
        char peek_next() const;
        char advance_char();
        bool match_char(char expected);

        void skip_whitespace_and_comments();

        void lex_number(int start_line, int start_col);
        void lex_identifier_or_keyword(int start_line, int start_col);
        void lex_string(int start_line, int start_col);
        void lex_char(int start_line, int start_col);
        void lex_raw_string(int start_line, int start_col);
        void lex_multiline_string(int start_line, int start_col);

        void add(TokenKind kind, const std::string& lexeme, int line, int col);

        static bool is_digit(char c);
        static bool is_alpha(char c);
        static bool is_alnum(char c);
    };

} // namespace mana::frontend
