#pragma once

#include "ast.h"
#include "token.h"

#include <memory>
#include <string>
#include <vector>

class Parser
{
public:

    explicit Parser(
        const std::vector<Token>& tokens
    );

    std::unique_ptr<Program>
    parse();

private:

    // ========================================================
    // Token navigation
    // ========================================================

    const Token& peek() const;

    const Token& previous() const;

    const Token& advance();

    bool check(TokenType type) const;

    bool match(TokenType type);

    const Token& consume(
        TokenType type,
        const std::string& message
    );

    bool isAtEnd() const;

    // ========================================================
    // Error handling
    // ========================================================

    [[noreturn]]
    void error(
        const Token& token,
        const std::string& message
    );

    // ========================================================
    // Statements
    // ========================================================

    std::unique_ptr<Statement>
    parseStatement();

    std::unique_ptr<Statement>
    parseIfStatement();

    std::unique_ptr<Statement>
    parseWhileStatement();

    std::unique_ptr<Statement>
    parseForStatement();

    std::unique_ptr<Statement>
    parseBreakStatement();

    std::unique_ptr<Statement>
    parseContinueStatement();

    std::unique_ptr<Statement>
    parseVariableDeclaration();

    std::unique_ptr<Statement>
    parseExpressionStatement();

    std::unique_ptr<Statement>
    parseFunctionStatement();

    std::unique_ptr<Statement>
    parseReturnStatement();

    // ========================================================
    // Expressions
    // ========================================================

    std::unique_ptr<Expression>
    parseExpression();

    std::unique_ptr<Expression>
    parseAssignment();

    std::unique_ptr<Expression>
    parseLogicalOr();

    std::unique_ptr<Expression>
    parseLogicalAnd();

    std::unique_ptr<Expression>
    parseEquality();

    std::unique_ptr<Expression>
    parseComparison();

    std::unique_ptr<Expression>
    parseTerm();

    std::unique_ptr<Expression>
    parseFactor();

    std::unique_ptr<Expression>
    parseUnary();

    std::unique_ptr<Expression>
    parseCall();

    std::unique_ptr<Expression>
    parsePrimary();

    std::unique_ptr<Expression>
    parseArrayLiteral();

    // ========================================================
    // Parser state
    // ========================================================

    const std::vector<Token>& tokens;

    std::size_t current = 0;
};