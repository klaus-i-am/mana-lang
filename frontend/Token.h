#pragma once
#include <string>

namespace mana::frontend {

    enum class TokenKind {
        EndOfFile,

        Identifier,
        IntLiteral,
        FloatLiteral,
        StringLiteral,
        FStringLiteral,     // f"..." interpolated string
        CharLiteral,        // character literals
        RawStringLiteral,   // r"..." raw strings (no escape processing)
        MultiLineStringLiteral,  // """...""" multi-line strings
        DocComment,              // /// documentation comments

        // Keywords
        KwModule,
        KwImport,
        KwFn,
        KwStruct,
        KwEnum,
        KwLet,
        KwReturn,
        KwIf,
        KwElse,
        KwWhile,
        KwFor,
        KwBreak,
        KwContinue,
        KwDefer,
        KwScope,
        KwTrue,
        KwFalse,
        KwSelf,
        KwMatch,
        KwTrait,
        KwImpl,
        KwDyn,
        KwIn,
        KwMut,          // mut for mutable references
        KwMove,         // move for explicit ownership transfer
        KwConst,        // const for immutable bindings
        KwNone,         // None for Option type
        KwType,         // type for type aliases
        KwLoop,         // loop for infinite loops
        KwAs,           // as for type casting
        KwUse,          // use for imports
        KwPub,          // pub for public visibility
        KwFrom,         // from for selective imports
        KwAsync,        // async for async functions
        KwAwait,        // await for awaiting futures
        KwWhere,        // where for generic constraints
        KwStatic,       // static for static methods
        KwVariant,      // variant as synonym for enum (vNext)
        KwWhen,         // when as preferred pattern matching (vNext)

        // Punctuation
        LParen, RParen,
        LBrace, RBrace,
        LBracket, RBracket,
        Comma,
        Semicolon,
        Colon,
        ColonColon, // ::
        Dot,
        Arrow,      // ->
        FatArrow,   // =>
        Assign,     // =
        Underscore, // _ (wildcard/discard)

        // Operators
        Plus, Minus, Star, Slash, Percent,
        PlusPlus, MinusMinus,       // ++ --
        PlusEqual, MinusEqual,      // += -=
        StarStar,                              // ** power
        StarStarEqual,                         // **= power-assign
        StarEqual, SlashEqual, PercentEqual,  // *= /= %=
        EqualEqual, BangEqual,
        Less, LessEqual,
        Greater, GreaterEqual,
        LessLess, GreaterGreater,   // << >>
        AndAnd, OrOr,
        And, Or, Caret, Tilde,      // & | ^ ~
        AndEqual, OrEqual, CaretEqual,  // &= |= ^=
        LessLessEqual, GreaterGreaterEqual,  // <<= >>=
        Bang,
        Question,                   // ? for error propagation
        QuestionDot,                // ?. for optional chaining
        QuestionQuestion,           // ?? for null coalescing
        DotDot,                     // .. for range
        DotDotEqual,                // ..= for inclusive range
        Hash                        // # for attributes
    };

    struct Token {
        TokenKind kind{};
        std::string lexeme;
        int line = 1;
        int column = 1;

        Token() = default;
        Token(TokenKind k, std::string lx, int l, int c)
            : kind(k), lexeme(std::move(lx)), line(l), column(c) {
        }
    };

    const char* token_kind_name(TokenKind k);

} // namespace mana::frontend
