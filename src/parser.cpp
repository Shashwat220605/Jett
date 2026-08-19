#include "parser.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>

// ============================================================
// Constructor
// ============================================================

Parser::Parser(
    const std::vector<Token>& tokens
)
    : tokens(tokens)
{
}

// ============================================================
// Basic parser helpers
// ============================================================

bool Parser::isAtEnd() const
{
    return peek().type ==
           TokenType::END_OF_FILE;
}

const Token& Parser::peek() const
{
    return tokens[current];
}

const Token& Parser::previous() const
{
    return tokens[current - 1];
}

const Token& Parser::advance()
{
    if (!isAtEnd())
    {
        current++;
    }

    return previous();
}

bool Parser::check(
    TokenType type
) const
{
    if (isAtEnd())
    {
        return type ==
               TokenType::END_OF_FILE;
    }

    return peek().type == type;
}

bool Parser::match(
    TokenType type
)
{
    if (!check(type))
    {
        return false;
    }

    advance();

    return true;
}

const Token& Parser::consume(
    TokenType type,
    const std::string& message
)
{
    if (check(type))
    {
        return advance();
    }

    error(
        peek(),
        message
    );
}

// ============================================================
// Error handling
// ============================================================

[[noreturn]]
void Parser::error(
    const Token& token,
    const std::string& message
)
{
    std::cerr
        << "Error at line "
        << token.line
        << ", column "
        << token.column
        << ": "
        << message
        << '\n';

    std::exit(1);
}

// ============================================================
// Parse program
// ============================================================

std::unique_ptr<Program>
Parser::parse()
{
    auto program =
        std::make_unique<Program>();

    while (!isAtEnd())
    {
        program->statements.push_back(
            parseStatement()
        );
    }

    return program;
}

// ============================================================
// Parse statement
// ============================================================

std::unique_ptr<Statement>
Parser::parseStatement()
{
    // --------------------------------------------------------
    // Function
    // --------------------------------------------------------

    if (check(TokenType::FN))
    {
        return parseFunctionStatement();
    }

    // --------------------------------------------------------
    // If
    // --------------------------------------------------------

    if (check(TokenType::IF))
    {
        return parseIfStatement();
    }

    // --------------------------------------------------------
    // While
    // --------------------------------------------------------

    if (check(TokenType::WHILE))
    {
        return parseWhileStatement();
    }

    // --------------------------------------------------------
    // For
    // --------------------------------------------------------

    if (check(TokenType::FOR))
    {
        return parseForStatement();
    }

    // --------------------------------------------------------
    // Return
    // --------------------------------------------------------

    if (check(TokenType::RETURN))
    {
        return parseReturnStatement();
    }

    // --------------------------------------------------------
    // Break
    // --------------------------------------------------------

    if (check(TokenType::BREAK))
    {
        return parseBreakStatement();
    }

    // --------------------------------------------------------
    // Continue
    // --------------------------------------------------------

    if (check(TokenType::CONTINUE))
    {
        return parseContinueStatement();
    }

    // --------------------------------------------------------
    // Explicit variable declaration
    //
    // Int x is 10;
    // Str name is "Jett";
    // Int[] nums is [1, 2, 3];
    // --------------------------------------------------------

    if (check(TokenType::INT_TYPE) ||
        check(TokenType::FL_TYPE) ||
        check(TokenType::DOUB_TYPE) ||
        check(TokenType::STR_TYPE) ||
        check(TokenType::BOOL_TYPE) ||
        check(TokenType::CHAR_TYPE) ||
        check(TokenType::BYTE_TYPE) ||
        check(TokenType::LONG_TYPE) ||
        check(TokenType::VOID_TYPE))
    {
        return parseVariableDeclaration();
    }

    // --------------------------------------------------------
    // Type-inferred declaration
    //
    // name is "Jett";
    // nums is [1, 2, 3];
    // --------------------------------------------------------

    if (check(TokenType::IDENTIFIER) &&
        current + 1 < tokens.size() &&
        tokens[current + 1].type ==
            TokenType::IS)
    {
        return parseVariableDeclaration();
    }

    // --------------------------------------------------------
    // Expression
    // --------------------------------------------------------

    return parseExpressionStatement();
}

// ============================================================
// Parse if / else if / else
// ============================================================

std::unique_ptr<Statement>
Parser::parseIfStatement()
{
    const Token& ifToken =
        consume(
            TokenType::IF,
            "Expected 'if'."
        );

    // --------------------------------------------------------
    // Condition
    // --------------------------------------------------------

    auto condition =
        parseExpression();

    // --------------------------------------------------------
    // Opening brace
    // --------------------------------------------------------

    consume(
        TokenType::LEFT_BRACE,
        "Expected '{' after if condition."
    );

    // --------------------------------------------------------
    // Then branch
    // --------------------------------------------------------

    std::vector<std::unique_ptr<Statement>>
        thenBranch;

    while (!check(TokenType::RIGHT_BRACE) &&
           !isAtEnd())
    {
        thenBranch.push_back(
            parseStatement()
        );
    }

    consume(
        TokenType::RIGHT_BRACE,
        "Expected '}' after if block."
    );

    // --------------------------------------------------------
    // Else branch
    // --------------------------------------------------------

    std::vector<std::unique_ptr<Statement>>
        elseBranch;

    bool hasElse = false;

    if (match(TokenType::ELSE))
    {
        hasElse = true;

        // ----------------------------------------------------
        // else if
        // ----------------------------------------------------

        if (check(TokenType::IF))
        {
            elseBranch.push_back(
                parseIfStatement()
            );
        }
        else
        {
            consume(
                TokenType::LEFT_BRACE,
                "Expected '{' after 'else'."
            );

            while (!check(TokenType::RIGHT_BRACE) &&
                   !isAtEnd())
            {
                elseBranch.push_back(
                    parseStatement()
                );
            }

            consume(
                TokenType::RIGHT_BRACE,
                "Expected '}' after else block."
            );
        }
    }

    return std::make_unique<IfStmt>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch),
        hasElse,
        ifToken.line,
        ifToken.column
    );
}

// ============================================================
// Parse while
// ============================================================

std::unique_ptr<Statement>
Parser::parseWhileStatement()
{
    const Token& whileToken =
        consume(
            TokenType::WHILE,
            "Expected 'while'."
        );

    // --------------------------------------------------------
    // Condition
    // --------------------------------------------------------

    auto condition =
        parseExpression();

    // --------------------------------------------------------
    // Opening brace
    // --------------------------------------------------------

    consume(
        TokenType::LEFT_BRACE,
        "Expected '{' after while condition."
    );

    // --------------------------------------------------------
    // Body
    // --------------------------------------------------------

    std::vector<std::unique_ptr<Statement>>
        body;

    while (!check(TokenType::RIGHT_BRACE) &&
           !isAtEnd())
    {
        body.push_back(
            parseStatement()
        );
    }

    consume(
        TokenType::RIGHT_BRACE,
        "Expected '}' after while block."
    );

    return std::make_unique<WhileStmt>(
        std::move(condition),
        std::move(body),
        whileToken.line,
        whileToken.column
    );
}

// ============================================================
// Parse for
// ============================================================

std::unique_ptr<Statement>
Parser::parseForStatement()
{
    const Token& forToken =
        consume(
            TokenType::FOR,
            "Expected 'for'."
        );

    // --------------------------------------------------------
    // Initializer
    // --------------------------------------------------------

    std::unique_ptr<Statement>
        initializer;

    if (check(TokenType::INT_TYPE) ||
        check(TokenType::FL_TYPE) ||
        check(TokenType::DOUB_TYPE) ||
        check(TokenType::STR_TYPE) ||
        check(TokenType::BOOL_TYPE) ||
        check(TokenType::CHAR_TYPE) ||
        check(TokenType::BYTE_TYPE) ||
        check(TokenType::LONG_TYPE) ||
        check(TokenType::VOID_TYPE))
    {
        initializer =
            parseVariableDeclaration();
    }
    else if (check(TokenType::IDENTIFIER) &&
             current + 1 < tokens.size() &&
             tokens[current + 1].type ==
                 TokenType::IS)
    {
        initializer =
            parseVariableDeclaration();
    }
    else
    {
        initializer =
            parseExpressionStatement();
    }

    // --------------------------------------------------------
    // Condition
    // --------------------------------------------------------

    auto condition =
        parseExpression();

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after for condition."
    );

    // --------------------------------------------------------
    // Increment
    // --------------------------------------------------------

    auto increment =
        parseExpression();

    // --------------------------------------------------------
    // Opening brace
    // --------------------------------------------------------

    consume(
        TokenType::LEFT_BRACE,
        "Expected '{' after for clause."
    );

    // --------------------------------------------------------
    // Body
    // --------------------------------------------------------

    std::vector<std::unique_ptr<Statement>>
        body;

    while (!check(TokenType::RIGHT_BRACE) &&
           !isAtEnd())
    {
        body.push_back(
            parseStatement()
        );
    }

    consume(
        TokenType::RIGHT_BRACE,
        "Expected '}' after for block."
    );

    return std::make_unique<ForStmt>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body),
        forToken.line,
        forToken.column
    );
}

// ============================================================
// Parse break
// ============================================================

std::unique_ptr<Statement>
Parser::parseBreakStatement()
{
    const Token& breakToken =
        consume(
            TokenType::BREAK,
            "Expected 'break'."
        );

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after 'break'."
    );

    return std::make_unique<BreakStmt>(
        breakToken.line,
        breakToken.column
    );
}

// ============================================================
// Parse continue
// ============================================================

std::unique_ptr<Statement>
Parser::parseContinueStatement()
{
    const Token& continueToken =
        consume(
            TokenType::CONTINUE,
            "Expected 'continue'."
        );

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after 'continue'."
    );

    return std::make_unique<ContinueStmt>(
        continueToken.line,
        continueToken.column
    );
}

// ============================================================
// Parse variable declaration
//
// Int x is 10;
// Str name is "Jett";
// Int[] nums is [1, 2, 3];
// ============================================================

std::unique_ptr<Statement>
Parser::parseVariableDeclaration()
{
    std::string declaredType;

    const Token& startToken =
        peek();

    // --------------------------------------------------------
    // Explicit type
    // --------------------------------------------------------

    if (check(TokenType::INT_TYPE) ||
        check(TokenType::FL_TYPE) ||
        check(TokenType::DOUB_TYPE) ||
        check(TokenType::STR_TYPE) ||
        check(TokenType::BOOL_TYPE) ||
        check(TokenType::CHAR_TYPE) ||
        check(TokenType::BYTE_TYPE) ||
        check(TokenType::LONG_TYPE) ||
        check(TokenType::VOID_TYPE))
    {
        declaredType =
            advance().lexeme;

        // ----------------------------------------------------
        // Array type
        //
        // Int[]
        // Str[]
        // etc.
        // ----------------------------------------------------

        if (match(TokenType::LEFT_BRACKET))
        {
            consume(
                TokenType::RIGHT_BRACKET,
                "Expected ']' in array type."
            );

            declaredType += "[]";
        }
    }

    // --------------------------------------------------------
    // Variable name
    // --------------------------------------------------------

    const Token& name =
        consume(
            TokenType::IDENTIFIER,
            "Expected a variable name."
        );

    // --------------------------------------------------------
    // is
    // --------------------------------------------------------

    consume(
        TokenType::IS,
        "Expected 'is' after variable name."
    );

    // --------------------------------------------------------
    // Initializer
    // --------------------------------------------------------

    auto initializer =
        parseExpression();

    // --------------------------------------------------------
    // Semicolon
    // --------------------------------------------------------

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after variable declaration."
    );

    return std::make_unique<VariableDeclarationStmt>(
        name.lexeme,
        declaredType,
        std::move(initializer),
        startToken.line,
        startToken.column
    );
}

// ============================================================
// Parse function
//
// fn add(Int a, Int b) {
//     return a + b;
// }
// ============================================================

std::unique_ptr<Statement>
Parser::parseFunctionStatement()
{
    const Token& fnToken =
        consume(
            TokenType::FN,
            "Expected 'fn'."
        );

    // --------------------------------------------------------
    // Function name
    // --------------------------------------------------------

    const Token& name =
        consume(
            TokenType::IDENTIFIER,
            "Expected function name."
        );

    // --------------------------------------------------------
    // Opening parenthesis
    // --------------------------------------------------------

    consume(
        TokenType::LEFT_PAREN,
        "Expected '(' after function name."
    );

    // --------------------------------------------------------
    // Parameters
    // --------------------------------------------------------

    std::vector<FunctionParameter>
        parameters;

    if (!check(TokenType::RIGHT_PAREN))
    {
        do
        {
            std::string parameterType;

            // ------------------------------------------------
            // Parameter type
            // ------------------------------------------------

            if (check(TokenType::INT_TYPE) ||
                check(TokenType::FL_TYPE) ||
                check(TokenType::DOUB_TYPE) ||
                check(TokenType::STR_TYPE) ||
                check(TokenType::BOOL_TYPE) ||
                check(TokenType::CHAR_TYPE) ||
                check(TokenType::BYTE_TYPE) ||
                check(TokenType::LONG_TYPE) ||
                check(TokenType::VOID_TYPE))
            {
                parameterType =
                    advance().lexeme;

                // --------------------------------------------
                // Array parameter type
                // --------------------------------------------

                if (match(TokenType::LEFT_BRACKET))
                {
                    consume(
                        TokenType::RIGHT_BRACKET,
                        "Expected ']' in array parameter type."
                    );

                    parameterType += "[]";
                }
            }
            else
            {
                error(
                    peek(),
                    "Expected parameter type."
                );
            }

            // ------------------------------------------------
            // Parameter name
            // ------------------------------------------------

            const Token& parameterName =
                consume(
                    TokenType::IDENTIFIER,
                    "Expected parameter name."
                );

            parameters.emplace_back(
                parameterName.lexeme,
                parameterType
            );

        } while (match(TokenType::COMMA));
    }

    // --------------------------------------------------------
    // Closing parenthesis
    // --------------------------------------------------------

    consume(
        TokenType::RIGHT_PAREN,
        "Expected ')' after function parameters."
    );

    // --------------------------------------------------------
    // Opening brace
    // --------------------------------------------------------

    consume(
        TokenType::LEFT_BRACE,
        "Expected '{' before function body."
    );

    // --------------------------------------------------------
    // Function body
    // --------------------------------------------------------

    std::vector<std::unique_ptr<Statement>>
        body;

    while (!check(TokenType::RIGHT_BRACE) &&
           !isAtEnd())
    {
        body.push_back(
            parseStatement()
        );
    }

    // --------------------------------------------------------
    // Closing brace
    // --------------------------------------------------------

    consume(
        TokenType::RIGHT_BRACE,
        "Expected '}' after function body."
    );

    return std::make_unique<FunctionStmt>(
        name.lexeme,
        std::move(parameters),
        std::move(body),
        fnToken.line,
        fnToken.column
    );
}

// ============================================================
// Parse return
//
// return a + b;
// ============================================================

std::unique_ptr<Statement>
Parser::parseReturnStatement()
{
    const Token& returnToken =
        consume(
            TokenType::RETURN,
            "Expected 'return'."
        );

    std::unique_ptr<Expression>
        value;

    if (!check(TokenType::SEMICOLON))
    {
        value =
            parseExpression();
    }

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after return statement."
    );

    return std::make_unique<ReturnStmt>(
        std::move(value),
        returnToken.line,
        returnToken.column
    );
}

// ============================================================
// Parse expression statement
// ============================================================

std::unique_ptr<Statement>
Parser::parseExpressionStatement()
{
    const Token& startToken =
        peek();

    auto expression =
        parseExpression();

    consume(
        TokenType::SEMICOLON,
        "Expected ';' after expression."
    );

    return std::make_unique<ExpressionStmt>(
        std::move(expression),
        startToken.line,
        startToken.column
    );
}

// ============================================================
// Expressions
// ============================================================

std::unique_ptr<Expression>
Parser::parseExpression()
{
    return parseAssignment();
}

// ============================================================
// Assignment
//
// x = 10;
// nums[1] = 99;
// ============================================================

std::unique_ptr<Expression>
Parser::parseAssignment()
{
    auto expression =
        parseLogicalOr();

    if (match(TokenType::ASSIGN))
    {
        const Token& equalsToken =
            previous();

        auto value =
            parseAssignment();

        // ----------------------------------------------------
        // Normal variable assignment
        // ----------------------------------------------------

        if (auto variable =
            dynamic_cast<VariableExpr*>(
                expression.get()))
        {
            return std::make_unique<AssignmentExpr>(
                variable->name,
                std::move(value),
                equalsToken.line,
                equalsToken.column
            );
        }

        // ----------------------------------------------------
        // Array index assignment
        // ----------------------------------------------------

        if (dynamic_cast<IndexExpr*>(
                expression.get()))
        {
            return std::make_unique<IndexAssignmentExpr>(
                std::move(expression),
                std::move(value),
                equalsToken.line,
                equalsToken.column
            );
        }

        error(
            equalsToken,
            "Invalid assignment target."
        );
    }

    return expression;
}

// ============================================================
// Logical OR
//
// a || b
// ============================================================

std::unique_ptr<Expression>
Parser::parseLogicalOr()
{
    auto expression =
        parseLogicalAnd();

    while (match(TokenType::OR))
    {
        TokenType operatorType =
            previous().type;

        int line =
            previous().line;

        int column =
            previous().column;

        auto right =
            parseLogicalAnd();

        expression =
            std::make_unique<BinaryExpr>(
                std::move(expression),
                operatorType,
                std::move(right),
                line,
                column
            );
    }

    return expression;
}

// ============================================================
// Logical AND
//
// a && b
// ============================================================

std::unique_ptr<Expression>
Parser::parseLogicalAnd()
{
    auto expression =
        parseEquality();

    while (match(TokenType::AND))
    {
        TokenType operatorType =
            previous().type;

        int line =
            previous().line;

        int column =
            previous().column;

        auto right =
            parseEquality();

        expression =
            std::make_unique<BinaryExpr>(
                std::move(expression),
                operatorType,
                std::move(right),
                line,
                column
            );
    }

    return expression;
}

// ============================================================
// Equality
//
// a == b
// a != b
// ============================================================

std::unique_ptr<Expression>
Parser::parseEquality()
{
    auto expression =
        parseComparison();

    while (match(TokenType::EQUAL_EQUAL) ||
           match(TokenType::NOT_EQUAL))
    {
        TokenType operatorType =
            previous().type;

        int line =
            previous().line;

        int column =
            previous().column;

        auto right =
            parseComparison();

        expression =
            std::make_unique<BinaryExpr>(
                std::move(expression),
                operatorType,
                std::move(right),
                line,
                column
            );
    }

    return expression;
}

// ============================================================
// Comparison
//
// a > b
// a >= b
// a < b
// a <= b
// ============================================================

std::unique_ptr<Expression>
Parser::parseComparison()
{
    auto expression =
        parseTerm();

    while (match(TokenType::GREATER) ||
           match(TokenType::GREATER_EQUAL) ||
           match(TokenType::LESS) ||
           match(TokenType::LESS_EQUAL))
    {
        TokenType operatorType =
            previous().type;

        int line =
            previous().line;

        int column =
            previous().column;

        auto right =
            parseTerm();

        expression =
            std::make_unique<BinaryExpr>(
                std::move(expression),
                operatorType,
                std::move(right),
                line,
                column
            );
    }

    return expression;
}

// ============================================================
// Addition / subtraction
// ============================================================

std::unique_ptr<Expression>
Parser::parseTerm()
{
    auto expression =
        parseFactor();

    while (match(TokenType::PLUS) ||
           match(TokenType::MINUS))
    {
        TokenType operatorType =
            previous().type;

        int line =
            previous().line;

        int column =
            previous().column;

        auto right =
            parseFactor();

        expression =
            std::make_unique<BinaryExpr>(
                std::move(expression),
                operatorType,
                std::move(right),
                line,
                column
            );
    }

    return expression;
}

// ============================================================
// Multiplication / division / modulo
// ============================================================

std::unique_ptr<Expression>
Parser::parseFactor()
{
    auto expression =
        parseUnary();

    while (match(TokenType::STAR) ||
           match(TokenType::SLASH) ||
           match(TokenType::PERCENT))
    {
        TokenType operatorType =
            previous().type;

        int line =
            previous().line;

        int column =
            previous().column;

        auto right =
            parseUnary();

        expression =
            std::make_unique<BinaryExpr>(
                std::move(expression),
                operatorType,
                std::move(right),
                line,
                column
            );
    }

    return expression;
}

// ============================================================
// Unary
//
// !x
// -x
// +x
// ============================================================

std::unique_ptr<Expression>
Parser::parseUnary()
{
    if (match(TokenType::NOT) ||
        match(TokenType::MINUS) ||
        match(TokenType::PLUS))
    {
        TokenType operatorType =
            previous().type;

        int line =
            previous().line;

        int column =
            previous().column;

        auto right =
            parseUnary();

        return std::make_unique<UnaryExpr>(
            operatorType,
            std::move(right),
            line,
            column
        );
    }

    return parseCall();
}

// ============================================================
// Function calls + array indexing
//
// print(x)
// nums[0]
// print(nums[0])
// ============================================================

std::unique_ptr<Expression>
Parser::parseCall()
{
    auto expression =
        parsePrimary();

    while (true)
    {
        // ----------------------------------------------------
        // Function call
        // ----------------------------------------------------

        if (match(TokenType::LEFT_PAREN))
        {
            int line =
                previous().line;

            int column =
                previous().column;

            std::vector<std::unique_ptr<Expression>>
                arguments;

            if (!check(TokenType::RIGHT_PAREN))
            {
                do
                {
                    arguments.push_back(
                        parseExpression()
                    );

                } while (match(TokenType::COMMA));
            }

            consume(
                TokenType::RIGHT_PAREN,
                "Expected ')' after function arguments."
            );

            expression =
                std::make_unique<CallExpr>(
                    std::move(expression),
                    std::move(arguments),
                    line,
                    column
                );

            continue;
        }

        // ----------------------------------------------------
        // Array indexing
        //
        // nums[0]
        // nums[i + 1]
        // ----------------------------------------------------

        if (match(TokenType::LEFT_BRACKET))
        {
            int line =
                previous().line;

            int column =
                previous().column;

            auto index =
                parseExpression();

            consume(
                TokenType::RIGHT_BRACKET,
                "Expected ']' after array index."
            );

            expression =
                std::make_unique<IndexExpr>(
                    std::move(expression),
                    std::move(index),
                    line,
                    column
                );

            continue;
        }

        break;
    }

    return expression;
}

// ============================================================
// Array literal
//
// [10, 20, 30]
// ============================================================

std::unique_ptr<Expression>
Parser::parseArrayLiteral()
{
    const Token& startToken =
        consume(
            TokenType::LEFT_BRACKET,
            "Expected '['."
        );

    std::vector<std::unique_ptr<Expression>>
        elements;

    if (!check(TokenType::RIGHT_BRACKET))
    {
        do
        {
            elements.push_back(
                parseExpression()
            );

        } while (match(TokenType::COMMA));
    }

    consume(
        TokenType::RIGHT_BRACKET,
        "Expected ']' after array literal."
    );

    return std::make_unique<ArrayLiteralExpr>(
        std::move(elements),
        startToken.line,
        startToken.column
    );
}

// ============================================================
// Primary expressions
// ============================================================

std::unique_ptr<Expression>
Parser::parsePrimary()
{
    // --------------------------------------------------------
    // Integer
    // --------------------------------------------------------

    if (match(TokenType::INTEGER))
    {
        const Token& token =
            previous();

        long long value =
            std::stoll(
                token.lexeme
            );

        return std::make_unique<LiteralExpr>(
            value,
            token.line,
            token.column
        );
    }

    // --------------------------------------------------------
    // Float
    // --------------------------------------------------------

    if (match(TokenType::FLOAT))
    {
        const Token& token =
            previous();

        double value =
            std::stod(
                token.lexeme
            );

        return std::make_unique<LiteralExpr>(
            value,
            token.line,
            token.column
        );
    }

    // --------------------------------------------------------
    // String
    //
    // IMPORTANT:
    // The lexer has already removed the surrounding quotes.
    // Do NOT remove first/last characters here.
    // --------------------------------------------------------

    if (match(TokenType::STRING))
    {
        const Token& token =
            previous();

        std::string text =
            token.lexeme;

        return std::make_unique<LiteralExpr>(
            text,
            token.line,
            token.column
        );
    }

    // --------------------------------------------------------
    // Character
    // --------------------------------------------------------

    if (match(TokenType::CHARACTER))
    {
        const Token& token =
            previous();

        std::string text =
            token.lexeme;

        if (text.size() >= 1)
        {
            return std::make_unique<LiteralExpr>(
                text[0],
                token.line,
                token.column
            );
        }

        error(
            token,
            "Invalid character literal."
        );
    }

    // --------------------------------------------------------
    // true
    // --------------------------------------------------------

    if (match(TokenType::TRUE))
    {
        const Token& token =
            previous();

        return std::make_unique<LiteralExpr>(
            true,
            token.line,
            token.column
        );
    }

    // --------------------------------------------------------
    // false
    // --------------------------------------------------------

    if (match(TokenType::FALSE))
    {
        const Token& token =
            previous();

        return std::make_unique<LiteralExpr>(
            false,
            token.line,
            token.column
        );
    }

    // --------------------------------------------------------
    // Array literal
    // --------------------------------------------------------

    if (check(TokenType::LEFT_BRACKET))
    {
        return parseArrayLiteral();
    }

    // --------------------------------------------------------
    // Identifier
    // --------------------------------------------------------

    if (match(TokenType::IDENTIFIER))
    {
        const Token& token =
            previous();

        return std::make_unique<VariableExpr>(
            token.lexeme,
            token.line,
            token.column
        );
    }

    // --------------------------------------------------------
    // Parenthesized expression
    // --------------------------------------------------------

    if (match(TokenType::LEFT_PAREN))
    {
        auto expression =
            parseExpression();

        consume(
            TokenType::RIGHT_PAREN,
            "Expected ')' after expression."
        );

        return expression;
    }

    error(
        peek(),
        "Expected an expression."
    );
}