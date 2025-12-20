#include "Token.h"

namespace mana::frontend {

    const char* token_kind_name(TokenKind k) {
        switch (k) {
        case TokenKind::EndOfFile: return "EndOfFile";
        case TokenKind::Identifier: return "Identifier";
        case TokenKind::IntLiteral: return "IntLiteral";
        case TokenKind::FloatLiteral: return "FloatLiteral";
        case TokenKind::StringLiteral: return "StringLiteral";
        case TokenKind::FStringLiteral: return "FStringLiteral";
        case TokenKind::CharLiteral: return "CharLiteral";
        case TokenKind::RawStringLiteral: return "RawStringLiteral";
        case TokenKind::MultiLineStringLiteral: return "MultiLineStringLiteral";
        case TokenKind::DocComment: return "DocComment";

        case TokenKind::KwModule: return "KwModule";
        case TokenKind::KwImport: return "KwImport";
        case TokenKind::KwFn: return "KwFn";
        case TokenKind::KwStruct: return "KwStruct";
        case TokenKind::KwEnum: return "KwEnum";
        case TokenKind::KwLet: return "KwLet";
        case TokenKind::KwReturn: return "KwReturn";
        case TokenKind::KwIf: return "KwIf";
        case TokenKind::KwElse: return "KwElse";
        case TokenKind::KwWhile: return "KwWhile";
        case TokenKind::KwFor: return "KwFor";
        case TokenKind::KwBreak: return "KwBreak";
        case TokenKind::KwContinue: return "KwContinue";
        case TokenKind::KwDefer: return "KwDefer";
        case TokenKind::KwScope: return "KwScope";
        case TokenKind::KwTrue: return "KwTrue";
        case TokenKind::KwFalse: return "KwFalse";
        case TokenKind::KwSelf: return "KwSelf";
        case TokenKind::KwMatch: return "KwMatch";
        case TokenKind::KwTrait: return "KwTrait";
        case TokenKind::KwImpl: return "KwImpl";
        case TokenKind::KwDyn: return "KwDyn";
        case TokenKind::KwIn: return "KwIn";
        case TokenKind::KwMut: return "KwMut";
        case TokenKind::KwMove: return "KwMove";
        case TokenKind::KwConst: return "KwConst";
        case TokenKind::KwNone: return "KwNone";
        case TokenKind::KwType: return "KwType";
        case TokenKind::KwLoop: return "KwLoop";
        case TokenKind::KwAs: return "KwAs";
        case TokenKind::KwUse: return "KwUse";
        case TokenKind::KwPub: return "KwPub";
        case TokenKind::KwFrom: return "KwFrom";
        case TokenKind::KwAsync: return "KwAsync";
        case TokenKind::KwAwait: return "KwAwait";
        case TokenKind::KwWhere: return "KwWhere";
        case TokenKind::KwStatic: return "KwStatic";
        case TokenKind::KwVariant: return "KwVariant";

        case TokenKind::LParen: return "LParen";
        case TokenKind::RParen: return "RParen";
        case TokenKind::LBrace: return "LBrace";
        case TokenKind::RBrace: return "RBrace";
        case TokenKind::LBracket: return "LBracket";
        case TokenKind::RBracket: return "RBracket";
        case TokenKind::Comma: return "Comma";
        case TokenKind::Semicolon: return "Semicolon";
        case TokenKind::Colon: return "Colon";
        case TokenKind::ColonColon: return "ColonColon";
        case TokenKind::Dot: return "Dot";
        case TokenKind::Arrow: return "Arrow";
        case TokenKind::Assign: return "Assign";

        case TokenKind::Plus: return "Plus";
        case TokenKind::Minus: return "Minus";
        case TokenKind::Star: return "Star";
        case TokenKind::StarStar: return "StarStar";
        case TokenKind::StarStarEqual: return "StarStarEqual";
        case TokenKind::Slash: return "Slash";
        case TokenKind::Percent: return "Percent";
        case TokenKind::PlusPlus: return "PlusPlus";
        case TokenKind::MinusMinus: return "MinusMinus";
        case TokenKind::PlusEqual: return "PlusEqual";
        case TokenKind::MinusEqual: return "MinusEqual";
        case TokenKind::StarEqual: return "StarEqual";
        case TokenKind::SlashEqual: return "SlashEqual";
        case TokenKind::PercentEqual: return "PercentEqual";
        case TokenKind::EqualEqual: return "EqualEqual";
        case TokenKind::BangEqual: return "BangEqual";
        case TokenKind::Less: return "Less";
        case TokenKind::LessEqual: return "LessEqual";
        case TokenKind::Greater: return "Greater";
        case TokenKind::GreaterEqual: return "GreaterEqual";
        case TokenKind::LessLess: return "LessLess";
        case TokenKind::GreaterGreater: return "GreaterGreater";
        case TokenKind::And: return "And";
        case TokenKind::Or: return "Or";
        case TokenKind::Caret: return "Caret";
        case TokenKind::Tilde: return "Tilde";
        case TokenKind::AndEqual: return "AndEqual";
        case TokenKind::OrEqual: return "OrEqual";
        case TokenKind::CaretEqual: return "CaretEqual";
        case TokenKind::LessLessEqual: return "LessLessEqual";
        case TokenKind::GreaterGreaterEqual: return "GreaterGreaterEqual";
        case TokenKind::AndAnd: return "AndAnd";
        case TokenKind::OrOr: return "OrOr";
        case TokenKind::Bang: return "Bang";
        case TokenKind::Question: return "Question";
        case TokenKind::QuestionDot: return "QuestionDot";
        case TokenKind::QuestionQuestion: return "QuestionQuestion";
        case TokenKind::DotDot: return "DotDot";
        case TokenKind::DotDotEqual: return "DotDotEqual";
        case TokenKind::Hash: return "Hash";
        case TokenKind::FatArrow: return "FatArrow";
        case TokenKind::Underscore: return "Underscore";
        default: return "Unknown";
        }
    }

} // namespace mana::frontend

