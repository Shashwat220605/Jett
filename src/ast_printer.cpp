#include "ast_printer.h"
#include "token.h"

#include <iostream>

void ASTPrinter::printIndent(int indent)
{
    for (int i = 0; i < indent; ++i)
    {
        std::cout << "    ";
    }
}

// ============================================================
// Program
// ============================================================

void ASTPrinter::print(const Program& program)
{
    std::cout << "Program\n";

    for (const auto& statement : program.statements)
    {
        printStatement(statement.get(), 1);
    }
}

// ============================================================
// Statement
// ============================================================

void ASTPrinter::printStatement(
    const Statement* statement,
    int indent)
{
    if (statement == nullptr)
    {
        printIndent(indent);
        std::cout << "null\n";
        return;
    }

    // ========================================================
    // Variable declaration
    // ========================================================

    if (auto variable =
        dynamic_cast<const VariableDeclarationStmt*>(statement))
    {
        printIndent(indent);
        std::cout << "VariableDeclaration\n";

        printIndent(indent + 1);
        std::cout
            << "Name: "
            << variable->name
            << "\n";

        printIndent(indent + 1);
        std::cout << "Type: ";

        if (variable->declaredType.empty())
        {
            std::cout << "inferred\n";
        }
        else
        {
            std::cout
                << variable->declaredType
                << "\n";
        }

        printIndent(indent + 1);
        std::cout << "Value:\n";

        printExpression(
            variable->initializer.get(),
            indent + 2
        );

        return;
    }

    // ========================================================
    // Expression statement
    // ========================================================

    if (auto expression =
        dynamic_cast<const ExpressionStmt*>(statement))
    {
        printIndent(indent);
        std::cout << "ExpressionStatement\n";

        printExpression(
            expression->expression.get(),
            indent + 1
        );

        return;
    }

    // ========================================================
    // IF statement
    // ========================================================

    if (auto ifStatement =
        dynamic_cast<const IfStmt*>(statement))
    {
        printIndent(indent);
        std::cout << "IfStatement\n";

        printIndent(indent + 1);
        std::cout << "Condition:\n";

        printExpression(
            ifStatement->condition.get(),
            indent + 2
        );

        printIndent(indent + 1);
        std::cout << "Then:\n";

        for (const auto& child :
             ifStatement->thenBranch)
        {
            printStatement(
                child.get(),
                indent + 2
            );
        }

        if (ifStatement->hasElse)
        {
            printIndent(indent + 1);
            std::cout << "Else:\n";

            for (const auto& child :
                 ifStatement->elseBranch)
            {
                printStatement(
                    child.get(),
                    indent + 2
                );
            }
        }

        return;
    }

    // ========================================================
    // WHILE statement
    // ========================================================

    if (auto whileStatement =
        dynamic_cast<const WhileStmt*>(statement))
    {
        printIndent(indent);
        std::cout << "WhileStatement\n";

        printIndent(indent + 1);
        std::cout << "Condition:\n";

        printExpression(
            whileStatement->condition.get(),
            indent + 2
        );

        printIndent(indent + 1);
        std::cout << "Body:\n";

        for (const auto& child :
             whileStatement->body)
        {
            printStatement(
                child.get(),
                indent + 2
            );
        }

        return;
    }

    // ========================================================
    // Unknown statement
    // ========================================================

    printIndent(indent);
    std::cout << "UnknownStatement\n";
}

// ============================================================
// Expression
// ============================================================

void ASTPrinter::printExpression(
    const Expression* expression,
    int indent)
{
    if (expression == nullptr)
    {
        printIndent(indent);
        std::cout << "null\n";
        return;
    }

    // ========================================================
    // Literal
    // ========================================================

    if (auto literal =
        dynamic_cast<const LiteralExpr*>(expression))
    {
        printIndent(indent);

        std::cout << "Literal: ";

        std::visit(
            [](const auto& value)
            {
                std::cout << value;
            },
            literal->value
        );

        std::cout << "\n";

        return;
    }

    // ========================================================
    // Variable
    // ========================================================

    if (auto variable =
        dynamic_cast<const VariableExpr*>(expression))
    {
        printIndent(indent);

        std::cout
            << "Variable: "
            << variable->name
            << "\n";

        return;
    }

    // ========================================================
    // Assignment
    // ========================================================

    if (auto assignment =
        dynamic_cast<const AssignmentExpr*>(expression))
    {
        printIndent(indent);

        std::cout
            << "Assignment: "
            << assignment->name
            << "\n";

        printIndent(indent + 1);
        std::cout << "Value:\n";

        printExpression(
            assignment->value.get(),
            indent + 2
        );

        return;
    }

    // ========================================================
    // Function call
    // ========================================================

    if (auto call =
        dynamic_cast<const CallExpr*>(expression))
    {
        printIndent(indent);
        std::cout << "FunctionCall\n";

        printIndent(indent + 1);
        std::cout << "Callee:\n";

        printExpression(
            call->callee.get(),
            indent + 2
        );

        printIndent(indent + 1);
        std::cout << "Arguments:\n";

        if (call->arguments.empty())
        {
            printIndent(indent + 2);
            std::cout << "None\n";
        }
        else
        {
            for (const auto& argument :
                 call->arguments)
            {
                printExpression(
                    argument.get(),
                    indent + 2
                );
            }
        }

        return;
    }

    // ========================================================
    // Binary expression
    // ========================================================

    if (auto binary =
        dynamic_cast<const BinaryExpr*>(expression))
    {
        printIndent(indent);

        std::cout
            << "BinaryExpression: "
            << tokenTypeToString(binary->op)
            << "\n";

        printIndent(indent + 1);
        std::cout << "Left:\n";

        printExpression(
            binary->left.get(),
            indent + 2
        );

        printIndent(indent + 1);
        std::cout << "Right:\n";

        printExpression(
            binary->right.get(),
            indent + 2
        );

        return;
    }

    // ========================================================
    // Unary expression
    // ========================================================

    if (auto unary =
        dynamic_cast<const UnaryExpr*>(expression))
    {
        printIndent(indent);

        std::cout
            << "UnaryExpression: "
            << tokenTypeToString(unary->op)
            << "\n";

        printIndent(indent + 1);
        std::cout << "Operand:\n";

        printExpression(
            unary->right.get(),
            indent + 2
        );

        return;
    }

    // ========================================================
    // Unknown expression
    // ========================================================

    printIndent(indent);
    std::cout << "UnknownExpression\n";
}