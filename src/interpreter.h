#pragma once

#include "ast.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Interpreter
{
private:

    // ============================================================
    // Runtime array
    // ============================================================

    struct ArrayValue;

    using Value = std::variant<
        long long,
        double,
        std::string,
        bool,
        char,
        std::shared_ptr<ArrayValue>
    >;

    struct ArrayValue
    {
        std::vector<Value> elements;
    };

public:

    void execute(
        const Program& program
    );

private:

    // ============================================================
    // Control-flow signals
    // ============================================================

    struct ReturnSignal
    {
        Value value;
    };

    struct BreakSignal
    {
    };

    struct ContinueSignal
    {
    };

    // ============================================================
    // Statement execution
    // ============================================================

    void executeStatement(
        const Statement* statement
    );

    void executeVariableDeclaration(
        const VariableDeclarationStmt* statement
    );

    void executeExpressionStatement(
        const ExpressionStmt* statement
    );

    void executeIfStatement(
        const IfStmt* statement
    );

    void executeWhileStatement(
        const WhileStmt* statement
    );

    void executeForStatement(
        const ForStmt* statement
    );

    void executeFunction(
        const FunctionStmt* function,
        const std::vector<Value>& arguments
    );

    // ============================================================
    // Expression evaluation
    // ============================================================

    Value evaluate(
        const Expression* expression
    );

    Value evaluateLiteral(
        const LiteralExpr* expression
    );

    Value evaluateArrayLiteral(
        const ArrayLiteralExpr* expression
    );

    Value evaluateVariable(
        const VariableExpr* expression
    );

    Value evaluateIndex(
        const IndexExpr* expression
    );

    Value evaluateBinary(
        const BinaryExpr* expression
    );

    Value evaluateUnary(
        const UnaryExpr* expression
    );

    Value evaluateAssignment(
        const AssignmentExpr* expression
    );

    Value evaluateCall(
        const CallExpr* expression
    );

    Value evaluateIndexAssignment(
    const IndexAssignmentExpr* expression
);

    // ============================================================
    // Built-in functions
    // ============================================================

    void callPrint(
        const CallExpr* expression
    );

    // ============================================================
    // Variables
    // ============================================================

    std::unordered_map<
        std::string,
        Value
    > variables;

    // ============================================================
    // User-defined functions
    // ============================================================

    std::unordered_map<
        std::string,
        const FunctionStmt*
    > functions;

    // ============================================================
    // Helpers
    // ============================================================

    std::string valueToString(
        const Value& value
    ) const;

    bool isTruthy(
        const Value& value
    ) const;

    [[noreturn]]
    void runtimeError(
        const std::string& message
    ) const;
};