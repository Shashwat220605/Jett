#include "token.h"

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
        case TokenType::INTEGER: return "INTEGER";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::CHARACTER: return "CHARACTER";
        case TokenType::IDENTIFIER: return "IDENTIFIER";

        case TokenType::INT_TYPE: return "INT";
        case TokenType::FL_TYPE: return "FL";
        case TokenType::DOUB_TYPE: return "DOUB";
        case TokenType::STR_TYPE: return "STR";
        case TokenType::BOOL_TYPE: return "BOOL";
        case TokenType::CHAR_TYPE: return "CHAR";
        case TokenType::BYTE_TYPE: return "BYTE";
        case TokenType::LONG_TYPE: return "LONG";
        case TokenType::VOID_TYPE: return "VOID";

        case TokenType::IS: return "IS";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::WHILE: return "WHILE";
        case TokenType::FN: return "FN";
        case TokenType::FOR: return "FOR";
        case TokenType::SWITCH: return "SWITCH";
        case TokenType::CASE: return "CASE";
        case TokenType::DEFAULT: return "DEFAULT";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::USE: return "USE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";

        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";

        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::PLUS_EQUAL: return "PLUS_EQUAL";
        case TokenType::MINUS_EQUAL: return "MINUS_EQUAL";
        case TokenType::STAR_EQUAL: return "STAR_EQUAL";
        case TokenType::SLASH_EQUAL: return "SLASH_EQUAL";
        case TokenType::PERCENT_EQUAL: return "PERCENT_EQUAL";

        case TokenType::INCREMENT: return "INCREMENT";
        case TokenType::DECREMENT: return "DECREMENT";

        case TokenType::EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TokenType::NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::GREATER: return "GREATER";
        case TokenType::GREATER_EQUAL: return "GREATER_EQUAL";
        case TokenType::LESS: return "LESS";
        case TokenType::LESS_EQUAL: return "LESS_EQUAL";

        case TokenType::NOT: return "NOT";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";

        case TokenType::BIT_AND: return "BIT_AND";
        case TokenType::BIT_OR: return "BIT_OR";
        case TokenType::BIT_XOR: return "BIT_XOR";
        case TokenType::BIT_NOT: return "BIT_NOT";

        case TokenType::LEFT_SHIFT: return "LEFT_SHIFT";
        case TokenType::RIGHT_SHIFT: return "RIGHT_SHIFT";

        case TokenType::ARROW: return "ARROW";

        case TokenType::LEFT_PAREN: return "LEFT_PAREN";
        case TokenType::RIGHT_PAREN: return "RIGHT_PAREN";
        case TokenType::LEFT_BRACE: return "LEFT_BRACE";
        case TokenType::RIGHT_BRACE: return "RIGHT_BRACE";
        case TokenType::LEFT_BRACKET: return "LEFT_BRACKET";
        case TokenType::RIGHT_BRACKET: return "RIGHT_BRACKET";

        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COLON: return "COLON";
        case TokenType::DOT: return "DOT";

        case TokenType::END_OF_FILE: return "END_OF_FILE";
        case TokenType::UNKNOWN: return "UNKNOWN";
    }

    return "UNKNOWN";
}
