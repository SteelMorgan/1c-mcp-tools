#pragma once

#include <string>
#include <vector>

namespace lineage {

enum class TokenKind {
    End,
    Identifier,
    StringLiteral,
    NumberLiteral,
    Parameter,
    Dot,
    Comma,
    LParen,
    RParen,
    Semicolon,
    Operator,
    KeywordSelect,
    KeywordFrom,
    KeywordAs,
    KeywordLeft,
    KeywordRight,
    KeywordInner,
    KeywordFull,
    KeywordJoin,
    KeywordUnion,
    KeywordAll,
    KeywordInto,
    KeywordDestroy,
    KeywordIsNull,
    KeywordCast,
    KeywordPresentation,
    KeywordCase,
    KeywordWhen,
    KeywordThen,
    KeywordElse,
    KeywordEnd,
    KeywordDistinct,
    KeywordAllowed,
    KeywordTop,
    KeywordWhere,
    KeywordGroup,
    KeywordOrder,
    KeywordPo,
    KeywordBy,
    KeywordOn,
    KeywordHaving,
    KeywordTotals,
    KeywordAnd,
    KeywordOr,
    KeywordNot,
    KeywordIn,
    KeywordNull,
    KeywordTrue,
    KeywordFalse,
    KeywordRefs
};

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;
    size_t position = 0;
};

std::vector<Token> Tokenize(const std::string& query);
bool IsKeyword(TokenKind kind);

}  // namespace lineage
