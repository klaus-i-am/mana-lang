#include "Lexer.h"
#include "Keywords.h"
#include <cctype>
#include <algorithm>

namespace mana::frontend {

    Lexer::Lexer(std::string src) : src_(std::move(src)) {}

    bool Lexer::is_at_end() const { return current_ >= src_.size(); }
    char Lexer::peek_char() const { return is_at_end() ? '\0' : src_[current_]; }
    char Lexer::peek_next() const { return (current_ + 1 >= src_.size()) ? '\0' : src_[current_ + 1]; }

    char Lexer::advance_char() {
        char c = src_[current_++];
        if (c == '\n') { ++line_; col_ = 1; }
        else { ++col_; }
        return c;
    }

    bool Lexer::match_char(char expected) {
        if (is_at_end() || src_[current_] != expected) return false;
        advance_char();
        return true;
    }

    void Lexer::add(TokenKind kind, const std::string& lexeme, int line, int col) {
        tokens_.emplace_back(kind, lexeme, line, col);
    }

    bool Lexer::is_digit(char c) { return c >= '0' && c <= '9'; }
    bool Lexer::is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool Lexer::is_alnum(char c) { return is_alpha(c) || is_digit(c); }

    void Lexer::skip_whitespace_and_comments() {
        while (!is_at_end()) {
            char c = peek_char();
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                advance_char();
            }
            else if (c == '/' && peek_next() == '/') {
                // Check for doc comment (///)
                if (current_ + 2 < src_.size() && src_[current_ + 2] == '/') {
                    // This is a doc comment - don't skip it, let tokenize handle it
                    break;
                }
                // Regular comment - skip
                while (!is_at_end() && peek_char() != '\n') advance_char();
            }
            else if (c == '/' && peek_next() == '*') {
                advance_char();
                advance_char();
                while (!is_at_end()) {
                    if (peek_char() == '*' && peek_next() == '/') {
                        advance_char(); advance_char();
                        break;
                    }
                    advance_char();
                }
            }
            else {
                break;
            }
        }
    }

    void Lexer::lex_string(int start_line, int start_col) {
        std::string s;
        while (!is_at_end() && peek_char() != '"') {
            if (peek_char() == '\\') {
                advance_char();
                if (!is_at_end()) {
                    char escaped = advance_char();
                    switch (escaped) {
                        case 'n': s.push_back('\n'); break;
                        case 't': s.push_back('\t'); break;
                        case 'r': s.push_back('\r'); break;
                        case '\\': s.push_back('\\'); break;
                        case '"': s.push_back('"'); break;
                        default: s.push_back(escaped); break;
                    }
                }
            } else {
                s.push_back(advance_char());
            }
        }
        if (!is_at_end()) advance_char();
        add(TokenKind::StringLiteral, s, start_line, start_col);
    }

    void Lexer::lex_fstring(int start_line, int start_col) {
        std::string s;
        while (!is_at_end() && peek_char() != '"') {
            if (peek_char() == '\\') {
                advance_char();
                if (!is_at_end()) {
                    char escaped = advance_char();
                    switch (escaped) {
                        case 'n': s.push_back('\n'); break;
                        case 't': s.push_back('\t'); break;
                        case 'r': s.push_back('\r'); break;
                        case '\\': s.push_back('\\'); break;
                        case '"': s.push_back('"'); break;
                        case '{': s.push_back('{'); break;
                        case '}': s.push_back('}'); break;
                        default: s.push_back(escaped); break;
                    }
                }
            } else {
                s.push_back(advance_char());
            }
        }
        if (!is_at_end()) advance_char();
        add(TokenKind::FStringLiteral, s, start_line, start_col);
    }

    void Lexer::lex_char(int start_line, int start_col) {
        char ch;
        if (is_at_end()) {
            add(TokenKind::CharLiteral, "", start_line, start_col);
            return;
        }
        if (peek_char() == '\\') {
            advance_char();
            if (!is_at_end()) {
                char escaped = advance_char();
                switch (escaped) {
                    case 'n': ch = '\n'; break;
                    case 't': ch = '\t'; break;
                    case 'r': ch = '\r'; break;
                    case '0': ch = '\0'; break;
                    case '\'': ch = '\''; break;
                    case '\\': ch = '\\'; break;
                    default: ch = escaped; break;
                }
            } else {
                ch = '\\';
            }
        } else {
            ch = advance_char();
        }
        if (!is_at_end() && peek_char() == '\'') {
            advance_char();
        }
        std::string s(1, ch);
        add(TokenKind::CharLiteral, s, start_line, start_col);
    }

    void Lexer::lex_raw_string(int start_line, int start_col) {
        // Raw string: r"..." - no escape processing
        std::string s;
        while (!is_at_end() && peek_char() != '"') {
            s.push_back(advance_char());
        }
        if (!is_at_end()) advance_char(); // consume closing '"'
        add(TokenKind::RawStringLiteral, s, start_line, start_col);
    }


    void Lexer::lex_multiline_string(int start_line, int start_col) {
        // Multi-line string: """...""" - preserves newlines, minimal escape processing
        std::string s;
        while (!is_at_end()) {
            char c = peek_char();
            if (c == '"') {
                // Check for closing """
                if (current_ + 2 < src_.size() && 
                    src_[current_ + 1] == '"' && 
                    src_[current_ + 2] == '"') {
                    advance_char();  // first "
                    advance_char();  // second "
                    advance_char();  // third "
                    add(TokenKind::MultiLineStringLiteral, s, start_line, start_col);
                    return;
                }
            }
            s.push_back(advance_char());
        }
        // Unterminated multi-line string
        add(TokenKind::MultiLineStringLiteral, s, start_line, start_col);
    }

    void Lexer::lex_number(int start_line, int start_col) {
        size_t start = current_ - 1;
        bool is_float = false;
        char first = src_[start];
        if (first == '0' && !is_at_end()) {
            char second = peek_char();
            if (second == 'b' || second == 'B') {
                advance_char();
                while (!is_at_end() && (peek_char() == '0' || peek_char() == '1' || peek_char() == '_')) {
                    advance_char();
                }
                std::string text = src_.substr(start, current_ - start);
                text.erase(std::remove(text.begin(), text.end(), '_'), text.end());
                long long value = std::stoll(text.substr(2), nullptr, 2);
                add(TokenKind::IntLiteral, std::to_string(value), start_line, start_col);
                return;
            }
            if (second == 'x' || second == 'X') {
                advance_char();
                while (!is_at_end() && (std::isxdigit(peek_char()) || peek_char() == '_')) {
                    advance_char();
                }
                std::string text = src_.substr(start, current_ - start);
                text.erase(std::remove(text.begin(), text.end(), '_'), text.end());
                long long value = std::stoll(text.substr(2), nullptr, 16);
                add(TokenKind::IntLiteral, std::to_string(value), start_line, start_col);
                return;
            }
            if (second == 'o' || second == 'O') {
                // Octal literal: 0o755
                advance_char();
                while (!is_at_end() && ((peek_char() >= '0' && peek_char() <= '7') || peek_char() == '_')) {
                    advance_char();
                }
                std::string text = src_.substr(start, current_ - start);
                text.erase(std::remove(text.begin(), text.end(), '_'), text.end());
                long long value = std::stoll(text.substr(2), nullptr, 8);
                add(TokenKind::IntLiteral, std::to_string(value), start_line, start_col);
                return;
            }
        }
        while (!is_at_end() && (is_digit(peek_char()) || peek_char() == '_')) advance_char();
        if (!is_at_end() && peek_char() == '.' && is_digit(peek_next())) {
            is_float = true;
            advance_char();
            while (!is_at_end() && (is_digit(peek_char()) || peek_char() == '_')) advance_char();
        }
        // Scientific notation: e or E followed by optional +/- and digits
        if (!is_at_end() && (peek_char() == 'e' || peek_char() == 'E')) {
            is_float = true;
            advance_char();  // consume 'e' or 'E'
            if (!is_at_end() && (peek_char() == '+' || peek_char() == '-')) {
                advance_char();  // consume sign
            }
            while (!is_at_end() && is_digit(peek_char())) advance_char();
        }
        std::string text = src_.substr(start, current_ - start);
        text.erase(std::remove(text.begin(), text.end(), '_'), text.end());
        add(is_float ? TokenKind::FloatLiteral : TokenKind::IntLiteral, text, start_line, start_col);
    }

    void Lexer::lex_identifier_or_keyword(int start_line, int start_col) {
        size_t start = current_ - 1;
        while (!is_at_end() && is_alnum(peek_char())) advance_char();
        std::string text = src_.substr(start, current_ - start);
        
        // Check for standalone underscore (wildcard/discard pattern)
        if (text == "_") {
            add(TokenKind::Underscore, text, start_line, start_col);
            return;
        }
        
        TokenKind kw{};
        if (keyword_kind(text, kw)) {
            add(kw, text, start_line, start_col);
        } else {
            add(TokenKind::Identifier, text, start_line, start_col);
        }
    }

    std::vector<Token> Lexer::tokenize() {
        tokens_.clear();
        while (!is_at_end()) {
            skip_whitespace_and_comments();
            if (is_at_end()) break;
            int start_line = line_;
            int start_col = col_;
            char c = advance_char();
            switch (c) {
            case '(': add(TokenKind::LParen, "(", start_line, start_col); break;
            case ')': add(TokenKind::RParen, ")", start_line, start_col); break;
            case '{': add(TokenKind::LBrace, "{", start_line, start_col); break;
            case '}': add(TokenKind::RBrace, "}", start_line, start_col); break;
            case '[': add(TokenKind::LBracket, "[", start_line, start_col); break;
            case ']': add(TokenKind::RBracket, "]", start_line, start_col); break;
            case ',': add(TokenKind::Comma, ",", start_line, start_col); break;
            case ';': add(TokenKind::Semicolon, ";", start_line, start_col); break;
            case '~': add(TokenKind::Tilde, "~", start_line, start_col); break;
            case '?':
                if (match_char('?')) add(TokenKind::QuestionQuestion, "??", start_line, start_col);
                else if (match_char('.')) add(TokenKind::QuestionDot, "?.", start_line, start_col);
                else add(TokenKind::Question, "?", start_line, start_col);
                break;
            case ':':
                if (match_char(':')) add(TokenKind::ColonColon, "::", start_line, start_col);
                else add(TokenKind::Colon, ":", start_line, start_col);
                break;
            case '.':
                if (match_char('.')) {
                    if (match_char('=')) add(TokenKind::DotDotEqual, "..=", start_line, start_col);
                    else add(TokenKind::DotDot, "..", start_line, start_col);
                } else {
                    add(TokenKind::Dot, ".", start_line, start_col);
                }
                break;
            case '+':
                if (match_char('+')) add(TokenKind::PlusPlus, "++", start_line, start_col);
                else if (match_char('=')) add(TokenKind::PlusEqual, "+=", start_line, start_col);
                else add(TokenKind::Plus, "+", start_line, start_col);
                break;
            case '-':
                if (match_char('-')) add(TokenKind::MinusMinus, "--", start_line, start_col);
                else if (match_char('=')) add(TokenKind::MinusEqual, "-=", start_line, start_col);
                else if (match_char('>')) add(TokenKind::Arrow, "->", start_line, start_col);
                else add(TokenKind::Minus, "-", start_line, start_col);
                break;
            case '*':
                if (match_char('*')) {
                    if (match_char('=')) add(TokenKind::StarStarEqual, "**=", start_line, start_col);
                    else add(TokenKind::StarStar, "**", start_line, start_col);
                }
                else if (match_char('=')) add(TokenKind::StarEqual, "*=", start_line, start_col);
                else add(TokenKind::Star, "*", start_line, start_col);
                break;
            case '/':
                if (match_char('/') && peek_char() == '/') {
                    // Doc comment: /// ...
                    advance_char();  // consume third /
                    std::string doc;
                    // Skip leading whitespace
                    while (peek_char() == ' ' || peek_char() == '\t') advance_char();
                    // Collect the rest of the line
                    while (!is_at_end() && peek_char() != '\n') {
                        doc += advance_char();
                    }
                    add(TokenKind::DocComment, doc, start_line, start_col);
                }
                else if (match_char('=')) add(TokenKind::SlashEqual, "/=", start_line, start_col);
                else add(TokenKind::Slash, "/", start_line, start_col);
                break;
            case '%':
                if (match_char('=')) add(TokenKind::PercentEqual, "%=", start_line, start_col);
                else add(TokenKind::Percent, "%", start_line, start_col);
                break;
            case '#':
                add(TokenKind::Hash, "#", start_line, start_col);
                break;
            case '=':
                if (match_char('=')) add(TokenKind::EqualEqual, "==", start_line, start_col);
                else if (match_char('>')) add(TokenKind::FatArrow, "=>", start_line, start_col);
                else add(TokenKind::Assign, "=", start_line, start_col);
                break;
            case '!':
                if (match_char('=')) add(TokenKind::BangEqual, "!=", start_line, start_col);
                else add(TokenKind::Bang, "!", start_line, start_col);
                break;
            case '<':
                if (match_char('<')) {
                    if (match_char('=')) add(TokenKind::LessLessEqual, "<<=", start_line, start_col);
                    else add(TokenKind::LessLess, "<<", start_line, start_col);
                }
                else if (match_char('=')) add(TokenKind::LessEqual, "<=", start_line, start_col);
                else add(TokenKind::Less, "<", start_line, start_col);
                break;
            case '>':
                if (match_char('>')) {
                    if (match_char('=')) add(TokenKind::GreaterGreaterEqual, ">>=", start_line, start_col);
                    else add(TokenKind::GreaterGreater, ">>", start_line, start_col);
                }
                else if (match_char('=')) add(TokenKind::GreaterEqual, ">=", start_line, start_col);
                else add(TokenKind::Greater, ">", start_line, start_col);
                break;
            case '&':
                if (match_char('&')) add(TokenKind::AndAnd, "&&", start_line, start_col);
                else if (match_char('=')) add(TokenKind::AndEqual, "&=", start_line, start_col);
                else add(TokenKind::And, "&", start_line, start_col);
                break;
            case '|':
                if (match_char('|')) add(TokenKind::OrOr, "||", start_line, start_col);
                else if (match_char('=')) add(TokenKind::OrEqual, "|=", start_line, start_col);
                else add(TokenKind::Or, "|", start_line, start_col);
                break;
            case '^':
                if (match_char('=')) add(TokenKind::CaretEqual, "^=", start_line, start_col);
                else add(TokenKind::Caret, "^", start_line, start_col);
                break;
            case '\'':
                lex_char(start_line, start_col);
                break;
            case '"':
                // Check for triple-quoted multi-line string
                if (peek_char() == '"' && peek_next() == '"') {
                    advance_char();  // consume second "
                    advance_char();  // consume third "
                    lex_multiline_string(start_line, start_col);
                } else {
                    lex_string(start_line, start_col);
                }
                break;
            case 'f':
                if (peek_char() == '"') {
                    advance_char();
                    lex_fstring(start_line, start_col);
                } else {
                    lex_identifier_or_keyword(start_line, start_col);
                }
                break;

            case 'r':
                // Check for raw string: r"..."
                if (peek_char() == '"') {
                    advance_char(); // consume '"'
                    lex_raw_string(start_line, start_col);
                } else {
                    lex_identifier_or_keyword(start_line, start_col);
                }
                break;
            default:
                if (is_digit(c)) {
                    lex_number(start_line, start_col);
                }
                else if (is_alpha(c)) {
                    lex_identifier_or_keyword(start_line, start_col);
                }
                break;
            }
        }
        add(TokenKind::EndOfFile, "", line_, col_);
        return tokens_;
    }

} // namespace mana::frontend
