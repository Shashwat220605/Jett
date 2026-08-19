#pragma once

#include "token.h"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// ============================================================
// Base AST Node
// ============================================================

struct ASTNode
{
    int line;
    int column;

    ASTNode(
        int line = 0,
        int column = 0
    )
        : line(line),
          column(column)
    {
    }

    virtual ~ASTNode() = default;
};

// ============================================================
// Expressions
// ============================================================

struct Expression : public ASTNode
{
    Expression(
        int line = 0,
        int column = 0
    )
        : ASTNode(line, column)
    {
    }

    virtual ~Expression() = default;
};

// ============================================================
// Literal
// ============================================================

using LiteralValue = std::variant<
    long long,
    double,
    std::string,
    bool,
    char
>;

struct LiteralExpr : public Expression
{
    LiteralValue value;

    LiteralExpr(
        LiteralValue value,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          value(std::move(value))
    {
    }
};

// ============================================================
// Array Literal
//
// [10, 20, 30]
// ============================================================

struct ArrayLiteralExpr : public Expression
{
    std::vector<std::unique_ptr<Expression>>
        elements;

    ArrayLiteralExpr(
        std::vector<std::unique_ptr<Expression>>
            elements,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          elements(std::move(elements))
    {
    }
};

// ============================================================
// Variable
// ============================================================

struct VariableExpr : public Expression
{
    std::string name;

    VariableExpr(
        const std::string& name,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          name(name)
    {
    }
};

// ============================================================
// Assignment
// ============================================================

struct AssignmentExpr : public Expression
{
    std::string name;

    std::unique_ptr<Expression> value;

    AssignmentExpr(
        const std::string& name,
        std::unique_ptr<Expression> value,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          name(name),
          value(std::move(value))
    {
    }
};

// ============================================================
// Index Assignment
//
// nums[1] = 99;
// ============================================================

struct IndexAssignmentExpr : public Expression
{
    std::unique_ptr<Expression> target;

    std::unique_ptr<Expression> value;

    IndexAssignmentExpr(
        std::unique_ptr<Expression> target,
        std::unique_ptr<Expression> value,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          target(std::move(target)),
          value(std::move(value))
    {
    }
};
// ============================================================
// Array Index
//
// nums[0]
// ============================================================

struct IndexExpr : public Expression
{
    std::unique_ptr<Expression> array;

    std::unique_ptr<Expression> index;

    IndexExpr(
        std::unique_ptr<Expression> array,
        std::unique_ptr<Expression> index,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          array(std::move(array)),
          index(std::move(index))
    {
    }
};

// ============================================================
// Binary
// ============================================================

struct BinaryExpr : public Expression
{
    std::unique_ptr<Expression> left;

    TokenType op;

    std::unique_ptr<Expression> right;

    BinaryExpr(
        std::unique_ptr<Expression> left,
        TokenType op,
        std::unique_ptr<Expression> right,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          left(std::move(left)),
          op(op),
          right(std::move(right))
    {
    }
};

// ============================================================
// Unary
// ============================================================

struct UnaryExpr : public Expression
{
    TokenType op;

    std::unique_ptr<Expression> right;

    UnaryExpr(
        TokenType op,
        std::unique_ptr<Expression> right,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          op(op),
          right(std::move(right))
    {
    }
};

// ============================================================
// Function Call
// ============================================================

struct CallExpr : public Expression
{
    std::unique_ptr<Expression> callee;

    std::vector<std::unique_ptr<Expression>>
        arguments;

    CallExpr(
        std::unique_ptr<Expression> callee,
        std::vector<std::unique_ptr<Expression>>
            arguments,
        int line = 0,
        int column = 0
    )
        : Expression(line, column),
          callee(std::move(callee)),
          arguments(std::move(arguments))
    {
    }
};

// ============================================================
// Statements
// ============================================================

struct Statement : public ASTNode
{
    Statement(
        int line = 0,
        int column = 0
    )
        : ASTNode(line, column)
    {
    }

    virtual ~Statement() = default;
};

// ============================================================
// Variable Declaration
// ============================================================

struct VariableDeclarationStmt
    : public Statement
{
    std::string name;

    std::string declaredType;

    std::unique_ptr<Expression>
        initializer;

    VariableDeclarationStmt(
        const std::string& name,
        const std::string& declaredType,
        std::unique_ptr<Expression> initializer,
        int line = 0,
        int column = 0
    )
        : Statement(line, column),
          name(name),
          declaredType(declaredType),
          initializer(std::move(initializer))
    {
    }
};

// ============================================================
// Expression Statement
// ============================================================

struct ExpressionStmt : public Statement
{
    std::unique_ptr<Expression>
        expression;

    ExpressionStmt(
        std::unique_ptr<Expression> expression,
        int line = 0,
        int column = 0
    )
        : Statement(line, column),
          expression(std::move(expression))
    {
    }
};

// ============================================================
// If
// ============================================================

struct IfStmt : public Statement
{
    std::unique_ptr<Expression>
        condition;

    std::vector<std::unique_ptr<Statement>>
        thenBranch;

    std::vector<std::unique_ptr<Statement>>
        elseBranch;

    bool hasElse;

    IfStmt(
        std::unique_ptr<Expression> condition,
        std::vector<std::unique_ptr<Statement>>
            thenBranch,
        std::vector<std::unique_ptr<Statement>>
            elseBranch,
        bool hasElse,
        int line = 0,
        int column = 0
    )
        : Statement(line, column),
          condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)),
          hasElse(hasElse)
    {
    }
};

// ============================================================
// Function Parameter
// ============================================================

struct FunctionParameter
{
    std::string name;

    std::string type;

    FunctionParameter(
        const std::string& name,
        const std::string& type
    )
        : name(name),
          type(type)
    {
    }
};

// ============================================================
// Function Statement
// ============================================================

struct FunctionStmt : public Statement
{
    std::string name;

    std::vector<FunctionParameter>
        parameters;

    std::vector<std::unique_ptr<Statement>>
        body;

    FunctionStmt(
        const std::string& name,
        std::vector<FunctionParameter>
            parameters,
        std::vector<std::unique_ptr<Statement>>
            body,
        int line = 0,
        int column = 0
    )
        : Statement(line, column),
          name(name),
          parameters(std::move(parameters)),
          body(std::move(body))
    {
    }
};

// ============================================================
// Return
// ============================================================

struct ReturnStmt : public Statement
{
    std::unique_ptr<Expression>
        value;

    ReturnStmt(
        std::unique_ptr<Expression> value,
        int line = 0,
        int column = 0
    )
        : Statement(line, column),
          value(std::move(value))
    {
    }
};

// ============================================================
// While
// ============================================================

struct WhileStmt : public Statement
{
    std::unique_ptr<Expression>
        condition;

    std::vector<std::unique_ptr<Statement>>
        body;

    WhileStmt(
        std::unique_ptr<Expression> condition,
        std::vector<std::unique_ptr<Statement>>
            body,
        int line = 0,
        int column = 0
    )
        : Statement(line, column),
          condition(std::move(condition)),
          body(std::move(body))
    {
    }
};

// ============================================================
// For
// ============================================================

struct ForStmt : public Statement
{
    std::unique_ptr<Statement>
        initializer;

    std::unique_ptr<Expression>
        condition;

    std::unique_ptr<Expression>
        increment;

    std::vector<std::unique_ptr<Statement>>
        body;

    ForStmt(
        std::unique_ptr<Statement> initializer,
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> increment,
        std::vector<std::unique_ptr<Statement>>
            body,
        int line = 0,
        int column = 0
    )
        : Statement(line, column),
          initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body))
    {
    }
};

// ============================================================
// Break
// ============================================================

struct BreakStmt : public Statement
{
    BreakStmt(
        int line = 0,
        int column = 0
    )
        : Statement(line, column)
    {
    }
};

// ============================================================
// Continue
// ============================================================

struct ContinueStmt : public Statement
{
    ContinueStmt(
        int line = 0,
        int column = 0
    )
        : Statement(line, column)
    {
    }
};

// ============================================================
// Program
// ============================================================

struct Program : public ASTNode
{
    std::vector<std::unique_ptr<Statement>>
        statements;

    Program(
        int line = 0,
        int column = 0
    )
        : ASTNode(line, column)
    {
    }
};