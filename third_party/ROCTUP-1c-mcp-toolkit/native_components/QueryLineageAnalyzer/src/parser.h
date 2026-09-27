#pragma once

#include "ast.h"
#include "tokenizer.h"

#include <string>
#include <vector>

namespace lineage {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    QueryBatchNode ParseBatch();

private:
    StatementNode ParseStatement();
    std::shared_ptr<SelectStatementNode> ParseSelectStatement();
    std::shared_ptr<DestroyStatementNode> ParseDestroyStatement();
    void ParseSelectModifiers(SelectStatementNode& stmt);
    std::vector<SelectItemNode> ParseSelectItems();
    SelectItemNode ParseSelectItem();
    std::vector<SourceNode> ParseFromClause();
    SourceNode ParseSource();
    std::shared_ptr<ExprNode> ParseExpressionUntil(const std::vector<TokenKind>& stop_tokens);
    std::shared_ptr<ExprNode> ParsePrimary();
    std::shared_ptr<ExprNode> ParseCaseExpression();
    std::shared_ptr<ExprNode> ParseFunctionCall(const Token& function_name);
    std::vector<std::shared_ptr<ExprNode>> ParseArgumentList();
    std::string ParseOptionalAlias();
    std::string GuessResultName(const std::shared_ptr<ExprNode>& expr) const;
    void ParseOptionalInto(SelectStatementNode& stmt);
    void ParseOptionalWhereGroupHavingOrderTotals();
    void ParseSkipSectionUntil(const std::vector<TokenKind>& stop_tokens);
    void ParseOptionalUnion(SelectStatementNode& stmt);
    bool IsJoinStart() const;
    bool IsStopToken(TokenKind kind, const std::vector<TokenKind>& stop_tokens) const;
    bool Match(TokenKind kind);
    bool Check(TokenKind kind) const;
    const Token& Advance();
    const Token& Peek(int offset = 0) const;
    bool IsAtEnd() const;
    std::string ParseNamePath();

    std::vector<Token> tokens_;
    size_t current_ = 0;
    int recursion_depth_ = 0;
    static constexpr int kMaxRecursionDepth = 64;
};

QueryBatchNode ParseQueryBatch(const std::string& query_text);

}  // namespace lineage
