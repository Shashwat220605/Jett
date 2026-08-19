#pragma once

#include <string>

enum class TokenType
{
    // ========================================================
    // Literals
    // ========================================================

    INTEGER,
    FLOAT,
    STRING,
    CHARACTER,

    // ========================================================
    // Identifiers
    // ========================================================

    IDENTIFIER,

    // ========================================================
    // Types
    // ========================================================

    INT_TYPE,
    FL_TYPE,
    DOUB_TYPE,
    STR_TYPE,
    BOOL_TYPE,
    CHAR_TYPE,
    BYTE_TYPE,
    LONG_TYPE,
    VOID_TYPE,

    // ========================================================
    // Keywords
    // ========================================================

    IS,

    IF,
    ELSE,
    WHILE,

    FN,

    FOR,
    SWITCH,
    CASE,
    DEFAULT,

    BREAK,
    CONTINUE,
    USE,
    RETURN,

    TRUE,
    FALSE,

    // ========================================================
    // Arithmetic operators
    // ========================================================

    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,

    // Assignment
    ASSIGN,

    PLUS_EQUAL,
    MINUS_EQUAL,
    STAR_EQUAL,
    SLASH_EQUAL,
    PERCENT_EQUAL,

    INCREMENT,
    DECREMENT,

    // ========================================================
    // Comparison
    // ========================================================

    EQUAL_EQUAL,
    NOT_EQUAL,

    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,

    // ========================================================
    // Logical operators
    // ========================================================

    NOT,

    AND,
    OR,

    // ========================================================
    // Bitwise operators
    // ========================================================

    BIT_AND,
    BIT_OR,
    BIT_XOR,
    BIT_NOT,

    LEFT_SHIFT,
    RIGHT_SHIFT,

    // ========================================================
    // Other operators
    // ========================================================

    ARROW,

    // ========================================================
    // Punctuation
    // ========================================================

    LEFT_PAREN,
    RIGHT_PAREN,

    LEFT_BRACE,
    RIGHT_BRACE,

    LEFT_BRACKET,
    RIGHT_BRACKET,

    COMMA,
    SEMICOLON,
    COLON,
    DOT,

    // ========================================================
    // End / unknown
    // ========================================================

    END_OF_FILE,
    UNKNOWN
};

// ============================================================
// Token
// ============================================================

struct Token
{
    TokenType type;

    std::string lexeme;

    int line;

    int column;

    // Default constructor
    Token()
        : type(TokenType::UNKNOWN),
          lexeme(""),
          line(0),
          column(0)
    {
    }

    // Normal constructor
    Token(
        TokenType type,
        const std::string& lexeme,
        int line = 0,
        int column = 0
    )
        : type(type),
          lexeme(lexeme),
          line(line),
          column(column)
    {
    }
};

std::string tokenTypeToString(TokenType type);