#pragma once

#include "token.h"

#include <string>
#include <vector>

class Lexer
{
public:
    explicit Lexer(const std::string& source);

    std::vector<Token> tokenize();

private:
    std::string source;

    size_t current = 0;
    int line = 1;
    int column = 1;

    char peek() const;
    char peekNext() const;
    char advance();

    bool isAtEnd() const;

    void skipWhitespace();
    void scanToken(std::vector<Token>& tokens);

    void scanIdentifier(std::vector<Token>& tokens);
    void scanNumber(std::vector<Token>& tokens);
    void scanString(std::vector<Token>& tokens);
    void scanCharacter(std::vector<Token>& tokens);

    void addToken(
        std::vector<Token>& tokens,
        TokenType type,
        const std::string& lexeme
    );

    bool isAlpha(char c) const;
    bool isDigit(char c) const;
    bool isAlphaNumeric(char c) const;
};