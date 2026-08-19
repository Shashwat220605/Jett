#pragma once

#include "ast.h"

#include <string>
#include <unordered_map>
#include <vector>

enum class JettType
{
    INT,
    FL,
    DOUB,
    STR,
    BOOL,
    CHAR,
    BYTE,
    LONG,
    VOID,

    INT_ARRAY,
    FL_ARRAY,
    DOUB_ARRAY,
    STR_ARRAY,
    BOOL_ARRAY,
    CHAR_ARRAY,
    BYTE_ARRAY,
    LONG_ARRAY,

    UNKNOWN
};

class TypeChecker
{
public:

    void check(
        const Program& program
    );

    bool hasErrors() const;

    const std::vector<std::string>&
    getErrors() const;

private:

    // ========================================================
    // Statements
    // ========================================================

    void checkStatement(
        const Statement* statement
    );

    void checkIfStatement(
        const IfStmt* statement
    );

    void checkWhileStatement(
        const WhileStmt* statement
    );

    void checkForStatement(
        const ForStmt* statement
    );

    void checkFunctionStatement(
        const FunctionStmt* statement
    );

    void checkReturnStatement(
        const ReturnStmt* statement
    );

    void checkBreakStatement(
        const BreakStmt* statement
    );

    void checkContinueStatement(
        const ContinueStmt* statement
    );

    void checkVariableDeclaration(
        const VariableDeclarationStmt* statement
    );

    void checkExpressionStatement(
        const ExpressionStmt* statement
    );

    // ========================================================
    // Expressions
    // ========================================================

    JettType checkExpression(
        const Expression* expression
    );

    JettType checkLiteral(
        const LiteralExpr* expression
    );

    JettType checkArrayLiteral(
        const ArrayLiteralExpr* expression
    );

    JettType checkVariable(
        const VariableExpr* expression
    );

    JettType checkBinary(
        const BinaryExpr* expression
    );

    JettType checkUnary(
        const UnaryExpr* expression
    );

    JettType checkCall(
        const CallExpr* expression
    );

    JettType checkIndex(
        const IndexExpr* expression
    );

    JettType checkIndexAssignment(
    const IndexAssignmentExpr* expression
);

    // ========================================================
    // Type helpers
    // ========================================================

    JettType typeFromName(
        const std::string& name
    ) const;

    JettType arrayElementType(
        JettType type
    ) const;

    bool isArray(
        JettType type
    ) const;

    bool isNumeric(
        JettType type
    ) const;

    bool isAssignable(
        JettType expected,
        JettType actual
    ) const;

    std::string typeToString(
        JettType type
    ) const;

    // ========================================================
    // Error handling
    // ========================================================

    void typeError(
        const ASTNode* node,
        const std::string& message
    );

    // ========================================================
    // Variables
    // ========================================================

    std::unordered_map<
        std::string,
        JettType
    > variables;

    // ========================================================
    // Errors
    // ========================================================

    std::vector<std::string>
        errors;
};