#include "lexer.h"

#include <cctype>
#include <unordered_map>

// ============================================================
// Constructor
// ============================================================

Lexer::Lexer(const std::string& source)
    : source(source),
      current(0),
      line(1),
      column(1)
{
}

// ============================================================
// Tokenize
// ============================================================

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!isAtEnd())
    {
        skipWhitespace();

        if (isAtEnd())
        {
            break;
        }

        scanToken(tokens);
    }

    tokens.emplace_back(
        TokenType::END_OF_FILE,
        "",
        line,
        column
    );

    return tokens;
}

// ============================================================
// Basic character functions
// ============================================================

bool Lexer::isAtEnd() const
{
    return current >= source.size();
}

char Lexer::peek() const
{
    if (isAtEnd())
    {
        return '\0';
    }

    return source[current];
}

char Lexer::peekNext() const
{
    if (current + 1 >= source.size())
    {
        return '\0';
    }

    return source[current + 1];
}

char Lexer::advance()
{
    if (isAtEnd())
    {
        return '\0';
    }

    char c = source[current++];

    if (c == '\n')
    {
        line++;
        column = 1;
    }
    else
    {
        column++;
    }

    return c;
}

// ============================================================
// Character classification
// ============================================================

bool Lexer::isAlpha(char c) const
{
    return std::isalpha(
        static_cast<unsigned char>(c)
    ) || c == '_';
}

bool Lexer::isDigit(char c) const
{
    return std::isdigit(
        static_cast<unsigned char>(c)
    );
}

bool Lexer::isAlphaNumeric(char c) const
{
    return isAlpha(c) || isDigit(c);
}

// ============================================================
// Skip whitespace and comments
// ============================================================

void Lexer::skipWhitespace()
{
    while (!isAtEnd())
    {
        char c = peek();

        // Spaces / tabs / carriage returns
        if (c == ' ' ||
            c == '\r' ||
            c == '\t')
        {
            advance();
            continue;
        }

        // New line
        if (c == '\n')
        {
            advance();
            continue;
        }

        // Single-line comment
        if (c == '/' &&
            peekNext() == '/')
        {
            while (!isAtEnd() &&
                   peek() != '\n')
            {
                advance();
            }

            continue;
        }

        // Block comment
        if (c == '/' &&
            peekNext() == '*')
        {
            advance();
            advance();

            while (!isAtEnd())
            {
                if (peek() == '*' &&
                    peekNext() == '/')
                {
                    advance();
                    advance();
                    break;
                }

                advance();
            }

            continue;
        }

        break;
    }
}

// ============================================================
// Add token
// ============================================================

void Lexer::addToken(
    std::vector<Token>& tokens,
    TokenType type,
    const std::string& lexeme)
{
    tokens.emplace_back(
        type,
        lexeme,
        line,
        column
    );
}

// ============================================================
// Scan one token
// ============================================================

void Lexer::scanToken(
    std::vector<Token>& tokens)
{
    int tokenLine = line;
    int tokenColumn = column;

    char c = advance();

    // ========================================================
    // Identifier / keyword
    // ========================================================

    if (isAlpha(c))
    {
        // We already consumed the first character.
        // scanIdentifier() expects to continue from here,
        // so move one character back temporarily.

        current--;
        column--;

        scanIdentifier(tokens);

        return;
    }

    // ========================================================
    // Number
    // ========================================================

    if (isDigit(c))
    {
        current--;
        column--;

        scanNumber(tokens);

        return;
    }

    // ========================================================
    // Operators
    // ========================================================

    switch (c)
    {
        // ----------------------------------------------------
        // +
        // ++
        // +=
        // ----------------------------------------------------

        case '+':
        {
            if (!isAtEnd() &&
                peek() == '+')
            {
                advance();

                tokens.emplace_back(
                    TokenType::INCREMENT,
                    "++",
                    tokenLine,
                    tokenColumn
                );
            }
            else if (!isAtEnd() &&
                     peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::PLUS_EQUAL,
                    "+=",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::PLUS,
                    "+",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // -
        // --
        // -=
        // ->
        // ----------------------------------------------------

        case '-':
        {
            if (!isAtEnd() &&
                peek() == '-')
            {
                advance();

                tokens.emplace_back(
                    TokenType::DECREMENT,
                    "--",
                    tokenLine,
                    tokenColumn
                );
            }
            else if (!isAtEnd() &&
                     peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::MINUS_EQUAL,
                    "-=",
                    tokenLine,
                    tokenColumn
                );
            }
            else if (!isAtEnd() &&
                     peek() == '>')
            {
                advance();

                tokens.emplace_back(
                    TokenType::ARROW,
                    "->",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::MINUS,
                    "-",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // *
        // *=
        // ----------------------------------------------------

        case '*':
        {
            if (!isAtEnd() &&
                peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::STAR_EQUAL,
                    "*=",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::STAR,
                    "*",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // /
        // /=
        // ----------------------------------------------------

        case '/':
        {
            if (!isAtEnd() &&
                peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::SLASH_EQUAL,
                    "/=",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::SLASH,
                    "/",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // %
        // %=
        // ----------------------------------------------------

        case '%':
        {
            if (!isAtEnd() &&
                peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::PERCENT_EQUAL,
                    "%=",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::PERCENT,
                    "%",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // =
        // ==
        // ----------------------------------------------------

        case '=':
        {
            if (!isAtEnd() &&
                peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::EQUAL_EQUAL,
                    "==",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                // NEW:
                // assignment operator
                //
                // count = count + 1;

                tokens.emplace_back(
                    TokenType::ASSIGN,
                    "=",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // !
        // !=
        // ----------------------------------------------------

        case '!':
        {
            if (!isAtEnd() &&
                peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::NOT_EQUAL,
                    "!=",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::NOT,
                    "!",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // >
        // >=
        // >>
        // ----------------------------------------------------

        case '>':
        {
            if (!isAtEnd() &&
                peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::GREATER_EQUAL,
                    ">=",
                    tokenLine,
                    tokenColumn
                );
            }
            else if (!isAtEnd() &&
                     peek() == '>')
            {
                advance();

                tokens.emplace_back(
                    TokenType::RIGHT_SHIFT,
                    ">>",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::GREATER,
                    ">",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // <
        // <=
        // <<
        // ----------------------------------------------------

        case '<':
        {
            if (!isAtEnd() &&
                peek() == '=')
            {
                advance();

                tokens.emplace_back(
                    TokenType::LESS_EQUAL,
                    "<=",
                    tokenLine,
                    tokenColumn
                );
            }
            else if (!isAtEnd() &&
                     peek() == '<')
            {
                advance();

                tokens.emplace_back(
                    TokenType::LEFT_SHIFT,
                    "<<",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::LESS,
                    "<",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // &
        // &&
        // ----------------------------------------------------

        case '&':
        {
            if (!isAtEnd() &&
                peek() == '&')
            {
                advance();

                tokens.emplace_back(
                    TokenType::AND,
                    "&&",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::BIT_AND,
                    "&",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // |
        // ||
        // ----------------------------------------------------

        case '|':
        {
            if (!isAtEnd() &&
                peek() == '|')
            {
                advance();

                tokens.emplace_back(
                    TokenType::OR,
                    "||",
                    tokenLine,
                    tokenColumn
                );
            }
            else
            {
                tokens.emplace_back(
                    TokenType::BIT_OR,
                    "|",
                    tokenLine,
                    tokenColumn
                );
            }

            return;
        }

        // ----------------------------------------------------
        // ^
        // ----------------------------------------------------

        case '^':
        {
            tokens.emplace_back(
                TokenType::BIT_XOR,
                "^",
                tokenLine,
                tokenColumn
            );

            return;
        }

        // ----------------------------------------------------
        // ~
        // ----------------------------------------------------

        case '~':
        {
            tokens.emplace_back(
                TokenType::BIT_NOT,
                "~",
                tokenLine,
                tokenColumn
            );

            return;
        }

        // ----------------------------------------------------
        // Parentheses
        // ----------------------------------------------------

        case '(':
            tokens.emplace_back(
                TokenType::LEFT_PAREN,
                "(",
                tokenLine,
                tokenColumn
            );
            return;

        case ')':
            tokens.emplace_back(
                TokenType::RIGHT_PAREN,
                ")",
                tokenLine,
                tokenColumn
            );
            return;

        // ----------------------------------------------------
        // Braces
        // ----------------------------------------------------

        case '{':
            tokens.emplace_back(
                TokenType::LEFT_BRACE,
                "{",
                tokenLine,
                tokenColumn
            );
            return;

        case '}':
            tokens.emplace_back(
                TokenType::RIGHT_BRACE,
                "}",
                tokenLine,
                tokenColumn
            );
            return;

        // ----------------------------------------------------
        // Brackets
        // ----------------------------------------------------

        case '[':
            tokens.emplace_back(
                TokenType::LEFT_BRACKET,
                "[",
                tokenLine,
                tokenColumn
            );
            return;

        case ']':
            tokens.emplace_back(
                TokenType::RIGHT_BRACKET,
                "]",
                tokenLine,
                tokenColumn
            );
            return;

        // ----------------------------------------------------
        // Punctuation
        // ----------------------------------------------------

        case ',':
            tokens.emplace_back(
                TokenType::COMMA,
                ",",
                tokenLine,
                tokenColumn
            );
            return;

        case ';':
            tokens.emplace_back(
                TokenType::SEMICOLON,
                ";",
                tokenLine,
                tokenColumn
            );
            return;

        case ':':
            tokens.emplace_back(
                TokenType::COLON,
                ":",
                tokenLine,
                tokenColumn
            );
            return;

        case '.':
            tokens.emplace_back(
                TokenType::DOT,
                ".",
                tokenLine,
                tokenColumn
            );
            return;

        // ----------------------------------------------------
        // String
        // ----------------------------------------------------

        case '"':
        {
            current--;
            column--;

            scanString(tokens);

            return;
        }

        // ----------------------------------------------------
        // Character
        // ----------------------------------------------------

        case '\'':
        {
            current--;
            column--;

            scanCharacter(tokens);

            return;
        }

        default:
            break;
    }

    // Unknown character
    tokens.emplace_back(
        TokenType::UNKNOWN,
        std::string(1, c),
        tokenLine,
        tokenColumn
    );
}

// ============================================================
// Identifier / keyword
// ============================================================

void Lexer::scanIdentifier(
    std::vector<Token>& tokens)
{
    int tokenLine = line;
    int tokenColumn = column;

    std::string text;

    while (!isAtEnd() &&
           isAlphaNumeric(peek()))
    {
        text += advance();
    }

    static const std::unordered_map<
        std::string,
        TokenType
    > keywords =
    {
        // Types
        {"Int", TokenType::INT_TYPE},
        {"Fl", TokenType::FL_TYPE},
        {"Doub", TokenType::DOUB_TYPE},
        {"Str", TokenType::STR_TYPE},
        {"Bool", TokenType::BOOL_TYPE},
        {"Char", TokenType::CHAR_TYPE},
        {"Byte", TokenType::BYTE_TYPE},
        {"Long", TokenType::LONG_TYPE},
        {"Void", TokenType::VOID_TYPE},

        // Language keywords
        {"is", TokenType::IS},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},

        // NEW
        {"while", TokenType::WHILE},

        {"fn", TokenType::FN},
        {"for", TokenType::FOR},
        {"switch", TokenType::SWITCH},
        {"case", TokenType::CASE},
        {"default", TokenType::DEFAULT},

        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"use", TokenType::USE},
        {"return", TokenType::RETURN},

        // Boolean literals
        {"true", TokenType::TRUE},
        {"false", TokenType::FALSE}
    };

    auto found = keywords.find(text);

    if (found != keywords.end())
    {
        tokens.emplace_back(
            found->second,
            text,
            tokenLine,
            tokenColumn
        );

        return;
    }

    tokens.emplace_back(
        TokenType::IDENTIFIER,
        text,
        tokenLine,
        tokenColumn
    );
}

// ============================================================
// Number
// ============================================================

void Lexer::scanNumber(
    std::vector<Token>& tokens)
{
    int tokenLine = line;
    int tokenColumn = column;

    std::string text;

    while (!isAtEnd() &&
           isDigit(peek()))
    {
        text += advance();
    }

    bool isFloat = false;

    if (!isAtEnd() &&
        peek() == '.' &&
        isDigit(peekNext()))
    {
        isFloat = true;

        text += advance();

        while (!isAtEnd() &&
               isDigit(peek()))
        {
            text += advance();
        }
    }

    tokens.emplace_back(
        isFloat
            ? TokenType::FLOAT
            : TokenType::INTEGER,
        text,
        tokenLine,
        tokenColumn
    );
}

// ============================================================
// String
// ============================================================

void Lexer::scanString(
    std::vector<Token>& tokens)
{
    int tokenLine = line;
    int tokenColumn = column;

    // Opening quote
    advance();

    std::string text;

    while (!isAtEnd() &&
           peek() != '"')
    {
        // Basic escape support
        if (peek() == '\\')
        {
            text += advance();

            if (!isAtEnd())
            {
                text += advance();
            }

            continue;
        }

        text += advance();
    }

    // Closing quote
    if (!isAtEnd())
    {
        advance();
    }

    tokens.emplace_back(
        TokenType::STRING,
        text,
        tokenLine,
        tokenColumn
    );
}

// ============================================================
// Character
// ============================================================

void Lexer::scanCharacter(
    std::vector<Token>& tokens)
{
    int tokenLine = line;
    int tokenColumn = column;

    // Opening quote
    advance();

    std::string text;

    if (!isAtEnd())
    {
        if (peek() == '\\')
        {
            text += advance();

            if (!isAtEnd())
            {
                text += advance();
            }
        }
        else
        {
            text += advance();
        }
    }

    // Closing quote
    if (!isAtEnd() &&
        peek() == '\'')
    {
        advance();
    }

    tokens.emplace_back(
        TokenType::CHARACTER,
        text,
        tokenLine,
        tokenColumn
    );
}