#include "type_checker.h"

#include <utility>

// ============================================================
// Entry point
// ============================================================

void TypeChecker::check(
    const Program& program
)
{
    variables.clear();
    errors.clear();

    for (const auto& statement :
         program.statements)
    {
        checkStatement(
            statement.get()
        );
    }
}

// ============================================================
// Errors
// ============================================================

bool TypeChecker::hasErrors() const
{
    return !errors.empty();
}

const std::vector<std::string>&
TypeChecker::getErrors() const
{
    return errors;
}

// ============================================================
// Statements
// ============================================================

void TypeChecker::checkStatement(
    const Statement* statement
)
{
    if (statement == nullptr)
    {
        typeError(
            nullptr,
            "Invalid statement."
        );

        return;
    }

    if (auto variable =
        dynamic_cast<const VariableDeclarationStmt*>(
            statement))
    {
        checkVariableDeclaration(variable);
        return;
    }

    if (auto expression =
        dynamic_cast<const ExpressionStmt*>(
            statement))
    {
        checkExpressionStatement(expression);
        return;
    }

    if (auto ifStatement =
        dynamic_cast<const IfStmt*>(
            statement))
    {
        checkIfStatement(ifStatement);
        return;
    }

    if (auto whileStatement =
        dynamic_cast<const WhileStmt*>(
            statement))
    {
        checkWhileStatement(whileStatement);
        return;
    }

    if (auto forStatement =
        dynamic_cast<const ForStmt*>(
            statement))
    {
        checkForStatement(forStatement);
        return;
    }

    if (auto function =
        dynamic_cast<const FunctionStmt*>(
            statement))
    {
        checkFunctionStatement(function);
        return;
    }

    if (auto returnStatement =
        dynamic_cast<const ReturnStmt*>(
            statement))
    {
        checkReturnStatement(returnStatement);
        return;
    }

    if (auto breakStatement =
        dynamic_cast<const BreakStmt*>(
            statement))
    {
        checkBreakStatement(breakStatement);
        return;
    }

    if (auto continueStatement =
        dynamic_cast<const ContinueStmt*>(
            statement))
    {
        checkContinueStatement(continueStatement);
        return;
    }

    typeError(
        statement,
        "Unknown statement."
    );
}

// ============================================================
// If
// ============================================================

void TypeChecker::checkIfStatement(
    const IfStmt* statement
)
{
    JettType conditionType =
        checkExpression(
            statement->condition.get()
        );

    if (conditionType != JettType::UNKNOWN &&
        conditionType != JettType::BOOL)
    {
        typeError(
            statement->condition.get(),
            "If condition must be Bool, but received " +
            typeToString(conditionType) +
            "."
        );
    }

    for (const auto& child :
         statement->thenBranch)
    {
        checkStatement(child.get());
    }

    if (statement->hasElse)
    {
        for (const auto& child :
             statement->elseBranch)
        {
            checkStatement(child.get());
        }
    }
}

// ============================================================
// While
// ============================================================

void TypeChecker::checkWhileStatement(
    const WhileStmt* statement
)
{
    JettType conditionType =
        checkExpression(
            statement->condition.get()
        );

    if (conditionType != JettType::UNKNOWN &&
        conditionType != JettType::BOOL)
    {
        typeError(
            statement->condition.get(),
            "While condition must be Bool, but received " +
            typeToString(conditionType) +
            "."
        );
    }

    for (const auto& child :
         statement->body)
    {
        checkStatement(child.get());
    }
}

// ============================================================
// For
// ============================================================

void TypeChecker::checkForStatement(
    const ForStmt* statement
)
{
    if (statement->initializer)
    {
        checkStatement(
            statement->initializer.get()
        );
    }

    if (statement->condition)
    {
        JettType conditionType =
            checkExpression(
                statement->condition.get()
            );

        if (conditionType != JettType::UNKNOWN &&
            conditionType != JettType::BOOL)
        {
            typeError(
                statement->condition.get(),
                "For condition must be Bool, but received " +
                typeToString(conditionType) +
                "."
            );
        }
    }

    for (const auto& child :
         statement->body)
    {
        checkStatement(child.get());
    }

    if (statement->increment)
    {
        checkExpression(
            statement->increment.get()
        );
    }
}

// ============================================================
// Break
// ============================================================

void TypeChecker::checkBreakStatement(
    const BreakStmt* statement
)
{
    (void)statement;
}

// ============================================================
// Continue
// ============================================================

void TypeChecker::checkContinueStatement(
    const ContinueStmt* statement
)
{
    (void)statement;
}

// ============================================================
// Function
// ============================================================

void TypeChecker::checkFunctionStatement(
    const FunctionStmt* statement
)
{
    auto previousVariables =
        std::move(variables);

    variables.clear();

    for (const auto& parameter :
         statement->parameters)
    {
        JettType parameterType =
            typeFromName(
                parameter.type
            );

        if (parameterType ==
            JettType::UNKNOWN)
        {
            typeError(
                statement,
                "Unknown parameter type '" +
                parameter.type +
                "'."
            );

            continue;
        }

        variables[
            parameter.name
        ] = parameterType;
    }

    for (const auto& child :
         statement->body)
    {
        checkStatement(child.get());
    }

    variables =
        std::move(previousVariables);
}

// ============================================================
// Return
// ============================================================

void TypeChecker::checkReturnStatement(
    const ReturnStmt* statement
)
{
    if (statement->value)
    {
        checkExpression(
            statement->value.get()
        );
    }
}

// ============================================================
// Variable Declaration
// ============================================================

void TypeChecker::checkVariableDeclaration(
    const VariableDeclarationStmt* statement
)
{
    const std::string& name =
        statement->name;

    if (variables.find(name) !=
        variables.end())
    {
        typeError(
            statement,
            "Variable '" +
            name +
            "' has already been declared."
        );

        return;
    }

    JettType actualType =
        checkExpression(
            statement->initializer.get()
        );

    if (statement->declaredType.empty())
    {
        if (actualType != JettType::UNKNOWN)
        {
            variables[name] =
                actualType;
        }

        return;
    }

    JettType expectedType =
        typeFromName(
            statement->declaredType
        );

    if (expectedType ==
        JettType::UNKNOWN)
    {
        typeError(
            statement,
            "Unknown type '" +
            statement->declaredType +
            "'."
        );

        return;
    }

    if (actualType ==
        JettType::UNKNOWN)
    {
        return;
    }

    if (!isAssignable(
            expectedType,
            actualType))
    {
        typeError(
            statement,
            "'" +
            name +
            "' expects " +
            typeToString(expectedType) +
            " but received " +
            typeToString(actualType) +
            "."
        );

        return;
    }

    variables[name] =
        expectedType;
}

// ============================================================
// Expression Statement
// ============================================================

void TypeChecker::checkExpressionStatement(
    const ExpressionStmt* statement
)
{
    checkExpression(
        statement->expression.get()
    );
}

// ============================================================
// Expressions
// ============================================================

JettType TypeChecker::checkExpression(
    const Expression* expression
)
{
    if (expression == nullptr)
    {
        typeError(
            nullptr,
            "Invalid expression."
        );

        return JettType::UNKNOWN;
    }

    if (auto literal =
        dynamic_cast<const LiteralExpr*>(
            expression))
    {
        return checkLiteral(literal);
    }

    if (auto array =
        dynamic_cast<const ArrayLiteralExpr*>(
            expression))
    {
        return checkArrayLiteral(array);
    }

    if (auto variable =
        dynamic_cast<const VariableExpr*>(
            expression))
    {
        return checkVariable(variable);
    }

    if (auto assignment =
        dynamic_cast<const AssignmentExpr*>(
            expression))
    {
        JettType valueType =
            checkExpression(
                assignment->value.get()
            );

        auto found =
            variables.find(
                assignment->name
            );

        if (found ==
            variables.end())
        {
            typeError(
                assignment,
                "Variable '" +
                assignment->name +
                "' was not declared."
            );

            return JettType::UNKNOWN;
        }

        if (!isAssignable(
                found->second,
                valueType))
        {
            typeError(
                assignment,
                "Cannot assign " +
                typeToString(valueType) +
                " to " +
                typeToString(found->second) +
                "."
            );

            return JettType::UNKNOWN;
        }

        return found->second;
    }

    if (auto index =
        dynamic_cast<const IndexExpr*>(
            expression))
    {
        return checkIndex(index);
    }

    if (auto indexAssignment =
        dynamic_cast<const IndexAssignmentExpr*>(
            expression))
    {
        return checkIndexAssignment(
            indexAssignment
        );
    }

    if (auto binary =
        dynamic_cast<const BinaryExpr*>(
            expression))
    {
        return checkBinary(binary);
    }

    if (auto unary =
        dynamic_cast<const UnaryExpr*>(
            expression))
    {
        return checkUnary(unary);
    }

    if (auto call =
        dynamic_cast<const CallExpr*>(
            expression))
    {
        return checkCall(call);
    }

    typeError(
        expression,
        "Unknown expression."
    );

    return JettType::UNKNOWN;
}

// ============================================================
// Literal
// ============================================================

JettType TypeChecker::checkLiteral(
    const LiteralExpr* expression
)
{
    if (std::holds_alternative<long long>(
            expression->value))
    {
        return JettType::INT;
    }

    if (std::holds_alternative<double>(
            expression->value))
    {
        return JettType::DOUB;
    }

    if (std::holds_alternative<std::string>(
            expression->value))
    {
        return JettType::STR;
    }

    if (std::holds_alternative<bool>(
            expression->value))
    {
        return JettType::BOOL;
    }

    if (std::holds_alternative<char>(
            expression->value))
    {
        return JettType::CHAR;
    }

    return JettType::UNKNOWN;
}

// ============================================================
// Array Literal
// ============================================================

JettType TypeChecker::checkArrayLiteral(
    const ArrayLiteralExpr* expression
)
{
    if (expression->elements.empty())
    {
        typeError(
            expression,
            "Empty arrays are not supported yet."
        );

        return JettType::UNKNOWN;
    }

    JettType elementType =
        JettType::UNKNOWN;

    for (const auto& element :
         expression->elements)
    {
        JettType currentType =
            checkExpression(
                element.get()
            );

        if (currentType ==
            JettType::UNKNOWN)
        {
            return JettType::UNKNOWN;
        }

        if (elementType ==
            JettType::UNKNOWN)
        {
            elementType =
                currentType;

            continue;
        }

        if (elementType !=
            currentType)
        {
            if (!(isNumeric(elementType) &&
                  isNumeric(currentType)))
            {
                typeError(
                    element.get(),
                    "All array elements must have "
                    "the same type."
                );

                return JettType::UNKNOWN;
            }
        }
    }

    switch (elementType)
    {
        case JettType::INT:
            return JettType::INT_ARRAY;

        case JettType::FL:
            return JettType::FL_ARRAY;

        case JettType::DOUB:
            return JettType::DOUB_ARRAY;

        case JettType::STR:
            return JettType::STR_ARRAY;

        case JettType::BOOL:
            return JettType::BOOL_ARRAY;

        case JettType::CHAR:
            return JettType::CHAR_ARRAY;

        case JettType::BYTE:
            return JettType::BYTE_ARRAY;

        case JettType::LONG:
            return JettType::LONG_ARRAY;

        default:
            return JettType::UNKNOWN;
    }
}

// ============================================================
// Variable
// ============================================================

JettType TypeChecker::checkVariable(
    const VariableExpr* expression
)
{
    auto found =
        variables.find(
            expression->name
        );

    if (found ==
        variables.end())
    {
        typeError(
            expression,
            "Variable '" +
            expression->name +
            "' was not declared."
        );

        return JettType::UNKNOWN;
    }

    return found->second;
}

// ============================================================
// Array Index
// ============================================================

JettType TypeChecker::checkIndex(
    const IndexExpr* expression
)
{
    JettType arrayType =
        checkExpression(
            expression->array.get()
        );

    JettType indexType =
        checkExpression(
            expression->index.get()
        );

    if (indexType != JettType::INT)
    {
        typeError(
            expression->index.get(),
            "Array index must be Int."
        );

        return JettType::UNKNOWN;
    }

    if (!isArray(arrayType))
    {
        typeError(
            expression->array.get(),
            "Only arrays can be indexed."
        );

        return JettType::UNKNOWN;
    }

    return arrayElementType(arrayType);
}

// ============================================================
// Array Index Assignment
// ============================================================

JettType TypeChecker::checkIndexAssignment(
    const IndexAssignmentExpr* expression
)
{
    auto index =
        dynamic_cast<const IndexExpr*>(
            expression->target.get()
        );

    if (index == nullptr)
    {
        typeError(
            expression,
            "Invalid array assignment target."
        );

        return JettType::UNKNOWN;
    }

    JettType arrayType =
        checkExpression(
            index->array.get()
        );

    if (!isArray(arrayType))
    {
        typeError(
            index->array.get(),
            "Only arrays can be indexed."
        );

        return JettType::UNKNOWN;
    }

    JettType indexType =
        checkExpression(
            index->index.get()
        );

    if (indexType != JettType::INT)
    {
        typeError(
            index->index.get(),
            "Array index must be Int."
        );

        return JettType::UNKNOWN;
    }

    JettType valueType =
        checkExpression(
            expression->value.get()
        );

    if (valueType ==
        JettType::UNKNOWN)
    {
        return JettType::UNKNOWN;
    }

    JettType elementType =
        arrayElementType(arrayType);

    if (!isAssignable(
            elementType,
            valueType))
    {
        typeError(
            expression,
            "Cannot assign " +
            typeToString(valueType) +
            " to array element of type " +
            typeToString(elementType) +
            "."
        );

        return JettType::UNKNOWN;
    }

    return elementType;
}

// ============================================================
// Binary
// ============================================================

JettType TypeChecker::checkBinary(
    const BinaryExpr* expression
)
{
    JettType leftType =
        checkExpression(
            expression->left.get()
        );

    JettType rightType =
        checkExpression(
            expression->right.get()
        );

    if (leftType == JettType::UNKNOWN ||
        rightType == JettType::UNKNOWN)
    {
        return JettType::UNKNOWN;
    }

    switch (expression->op)
    {
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
        {
            if (expression->op == TokenType::PLUS &&
                leftType == JettType::STR &&
                rightType == JettType::STR)
            {
                return JettType::STR;
            }

            if (isNumeric(leftType) &&
                isNumeric(rightType))
            {
                if (leftType == JettType::DOUB ||
                    rightType == JettType::DOUB)
                {
                    return JettType::DOUB;
                }

                if (leftType == JettType::FL ||
                    rightType == JettType::FL)
                {
                    return JettType::FL;
                }

                if (leftType == JettType::LONG ||
                    rightType == JettType::LONG)
                {
                    return JettType::LONG;
                }

                return JettType::INT;
            }

            typeError(
                expression,
                "Arithmetic operator cannot be used "
                "with " +
                typeToString(leftType) +
                " and " +
                typeToString(rightType) +
                "."
            );

            return JettType::UNKNOWN;
        }

        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
        {
            if (isNumeric(leftType) &&
                isNumeric(rightType))
            {
                return JettType::BOOL;
            }

            typeError(
                expression,
                "Comparison requires numeric values."
            );

            return JettType::UNKNOWN;
        }

        case TokenType::EQUAL_EQUAL:
        case TokenType::NOT_EQUAL:
        {
            if (leftType == rightType)
            {
                return JettType::BOOL;
            }

            if (isNumeric(leftType) &&
                isNumeric(rightType))
            {
                return JettType::BOOL;
            }

            typeError(
                expression,
                "Cannot compare " +
                typeToString(leftType) +
                " with " +
                typeToString(rightType) +
                "."
            );

            return JettType::UNKNOWN;
        }

        case TokenType::AND:
        {
            if (leftType == JettType::BOOL &&
                rightType == JettType::BOOL)
            {
                return JettType::BOOL;
            }

            typeError(
                expression,
                "The '&&' operator requires Bool values."
            );

            return JettType::UNKNOWN;
        }

        case TokenType::OR:
        {
            if (leftType == JettType::BOOL &&
                rightType == JettType::BOOL)
            {
                return JettType::BOOL;
            }

            typeError(
                expression,
                "The '||' operator requires Bool values."
            );

            return JettType::UNKNOWN;
        }

        default:
            break;
    }

    typeError(
        expression,
        "Unsupported binary operator."
    );

    return JettType::UNKNOWN;
}

// ============================================================
// Unary
// ============================================================

JettType TypeChecker::checkUnary(
    const UnaryExpr* expression
)
{
    JettType rightType =
        checkExpression(
            expression->right.get()
        );

    if (rightType == JettType::UNKNOWN)
    {
        return JettType::UNKNOWN;
    }

    switch (expression->op)
    {
        case TokenType::MINUS:
        case TokenType::PLUS:
        {
            if (!isNumeric(rightType))
            {
                typeError(
                    expression,
                    "Unary numeric operator "
                    "requires a number."
                );

                return JettType::UNKNOWN;
            }

            return rightType;
        }

        case TokenType::NOT:
        {
            if (rightType != JettType::BOOL)
            {
                typeError(
                    expression,
                    "The '!' operator requires "
                    "a Bool value."
                );

                return JettType::UNKNOWN;
            }

            return JettType::BOOL;
        }

        default:
            break;
    }

    typeError(
        expression,
        "Unsupported unary operator."
    );

    return JettType::UNKNOWN;
}

// ============================================================
// Function calls / built-ins
// ============================================================

JettType TypeChecker::checkCall(
    const CallExpr* expression
)
{
    auto variable =
        dynamic_cast<const VariableExpr*>(
            expression->callee.get()
        );

    if (!variable)
    {
        typeError(
            expression,
            "Invalid function call."
        );

        return JettType::UNKNOWN;
    }

    const std::string& name =
        variable->name;

    // --------------------------------------------------------
    // print(...)
    // --------------------------------------------------------

    if (name == "print")
    {
        for (const auto& argument :
             expression->arguments)
        {
            checkExpression(
                argument.get()
            );
        }

        return JettType::VOID;
    }

    // --------------------------------------------------------
    // input(prompt)
    // --------------------------------------------------------

    if (name == "input")
    {
        if (expression->arguments.size() > 1)
        {
            typeError(
                expression,
                "input() accepts zero or one argument."
            );

            return JettType::UNKNOWN;
        }

        if (!expression->arguments.empty())
        {
            JettType promptType =
                checkExpression(
                    expression->arguments[0].get()
                );

            if (promptType != JettType::STR &&
                promptType != JettType::UNKNOWN)
            {
                typeError(
                    expression->arguments[0].get(),
                    "input() prompt must be Str."
                );

                return JettType::UNKNOWN;
            }
        }

        return JettType::STR;
    }

    // --------------------------------------------------------
    // length(value)
    // --------------------------------------------------------

    if (name == "length")
    {
        if (expression->arguments.size() != 1)
        {
            typeError(
                expression,
                "length() expects exactly one argument."
            );

            return JettType::UNKNOWN;
        }

        JettType argumentType =
            checkExpression(
                expression->arguments[0].get()
            );

        if (argumentType != JettType::STR &&
            !isArray(argumentType) &&
            argumentType != JettType::UNKNOWN)
        {
            typeError(
                expression->arguments[0].get(),
                "length() expects a Str or array."
            );

            return JettType::UNKNOWN;
        }

        return JettType::INT;
    }

    // --------------------------------------------------------
    // upper(str)
    // --------------------------------------------------------

    if (name == "upper")
    {
        if (expression->arguments.size() != 1)
        {
            typeError(
                expression,
                "upper() expects exactly one argument."
            );

            return JettType::UNKNOWN;
        }

        JettType type =
            checkExpression(
                expression->arguments[0].get()
            );

        if (type != JettType::STR &&
            type != JettType::UNKNOWN)
        {
            typeError(
                expression->arguments[0].get(),
                "upper() expects Str."
            );

            return JettType::UNKNOWN;
        }

        return JettType::STR;
    }

    // --------------------------------------------------------
    // lower(str)
    // --------------------------------------------------------

    if (name == "lower")
    {
        if (expression->arguments.size() != 1)
        {
            typeError(
                expression,
                "lower() expects exactly one argument."
            );

            return JettType::UNKNOWN;
        }

        JettType type =
            checkExpression(
                expression->arguments[0].get()
            );

        if (type != JettType::STR &&
            type != JettType::UNKNOWN)
        {
            typeError(
                expression->arguments[0].get(),
                "lower() expects Str."
            );

            return JettType::UNKNOWN;
        }

        return JettType::STR;
    }

    // --------------------------------------------------------
    // trim(str)
    // --------------------------------------------------------

    if (name == "trim")
    {
        if (expression->arguments.size() != 1)
        {
            typeError(
                expression,
                "trim() expects exactly one argument."
            );

            return JettType::UNKNOWN;
        }

        JettType type =
            checkExpression(
                expression->arguments[0].get()
            );

        if (type != JettType::STR &&
            type != JettType::UNKNOWN)
        {
            typeError(
                expression->arguments[0].get(),
                "trim() expects Str."
            );

            return JettType::UNKNOWN;
        }

        return JettType::STR;
    }

    // --------------------------------------------------------
    // contains(str, sub)
    // --------------------------------------------------------

    if (name == "contains")
    {
        if (expression->arguments.size() != 2)
        {
            typeError(
                expression,
                "contains() expects two arguments."
            );

            return JettType::UNKNOWN;
        }

        JettType a =
            checkExpression(
                expression->arguments[0].get()
            );

        JettType b =
            checkExpression(
                expression->arguments[1].get()
            );

        if ((a != JettType::STR &&
             a != JettType::UNKNOWN) ||
            (b != JettType::STR &&
             b != JettType::UNKNOWN))
        {
            typeError(
                expression,
                "contains() expects two Str arguments."
            );

            return JettType::UNKNOWN;
        }

        return JettType::BOOL;
    }

    // --------------------------------------------------------
    // startsWith(str, prefix)
    // --------------------------------------------------------

    if (name == "startsWith")
    {
        if (expression->arguments.size() != 2)
        {
            typeError(
                expression,
                "startsWith() expects two arguments."
            );

            return JettType::UNKNOWN;
        }

        JettType a =
            checkExpression(
                expression->arguments[0].get()
            );

        JettType b =
            checkExpression(
                expression->arguments[1].get()
            );

        if ((a != JettType::STR &&
             a != JettType::UNKNOWN) ||
            (b != JettType::STR &&
             b != JettType::UNKNOWN))
        {
            typeError(
                expression,
                "startsWith() expects two Str arguments."
            );

            return JettType::UNKNOWN;
        }

        return JettType::BOOL;
    }

    // --------------------------------------------------------
    // endsWith(str, suffix)
    // --------------------------------------------------------

    if (name == "endsWith")
    {
        if (expression->arguments.size() != 2)
        {
            typeError(
                expression,
                "endsWith() expects two arguments."
            );

            return JettType::UNKNOWN;
        }

        JettType a =
            checkExpression(
                expression->arguments[0].get()
            );

        JettType b =
            checkExpression(
                expression->arguments[1].get()
            );

        if ((a != JettType::STR &&
             a != JettType::UNKNOWN) ||
            (b != JettType::STR &&
             b != JettType::UNKNOWN))
        {
            typeError(
                expression,
                "endsWith() expects two Str arguments."
            );

            return JettType::UNKNOWN;
        }

        return JettType::BOOL;
    }

    // --------------------------------------------------------
    // substring(str, start, length)
    // --------------------------------------------------------

    if (name == "substring")
    {
        if (expression->arguments.size() != 3)
        {
            typeError(
                expression,
                "substring() expects three arguments."
            );

            return JettType::UNKNOWN;
        }

        JettType stringType =
            checkExpression(
                expression->arguments[0].get()
            );

        JettType startType =
            checkExpression(
                expression->arguments[1].get()
            );

        JettType lengthType =
            checkExpression(
                expression->arguments[2].get()
            );

        if (stringType != JettType::STR &&
            stringType != JettType::UNKNOWN)
        {
            typeError(
                expression->arguments[0].get(),
                "substring() first argument must be Str."
            );

            return JettType::UNKNOWN;
        }

        if (startType != JettType::INT &&
            startType != JettType::UNKNOWN)
        {
            typeError(
                expression->arguments[1].get(),
                "substring() start must be Int."
            );

            return JettType::UNKNOWN;
        }

        if (lengthType != JettType::INT &&
            lengthType != JettType::UNKNOWN)
        {
            typeError(
                expression->arguments[2].get(),
                "substring() length must be Int."
            );

            return JettType::UNKNOWN;
        }

        return JettType::STR;
    }

    // --------------------------------------------------------
    // User-defined functions
    // --------------------------------------------------------

    for (const auto& argument :
         expression->arguments)
    {
        checkExpression(
            argument.get()
        );
    }

    return JettType::UNKNOWN;
}

// ============================================================
// Type name
// ============================================================

JettType TypeChecker::typeFromName(
    const std::string& name
) const
{
    if (name == "Int")
        return JettType::INT;

    if (name == "Fl")
        return JettType::FL;

    if (name == "Doub")
        return JettType::DOUB;

    if (name == "Str")
        return JettType::STR;

    if (name == "Bool")
        return JettType::BOOL;

    if (name == "Char")
        return JettType::CHAR;

    if (name == "Byte")
        return JettType::BYTE;

    if (name == "Long")
        return JettType::LONG;

    if (name == "Void")
        return JettType::VOID;

    if (name == "Int[]")
        return JettType::INT_ARRAY;

    if (name == "Fl[]")
        return JettType::FL_ARRAY;

    if (name == "Doub[]")
        return JettType::DOUB_ARRAY;

    if (name == "Str[]")
        return JettType::STR_ARRAY;

    if (name == "Bool[]")
        return JettType::BOOL_ARRAY;

    if (name == "Char[]")
        return JettType::CHAR_ARRAY;

    if (name == "Byte[]")
        return JettType::BYTE_ARRAY;

    if (name == "Long[]")
        return JettType::LONG_ARRAY;

    return JettType::UNKNOWN;
}

// ============================================================
// Array helpers
// ============================================================

bool TypeChecker::isArray(
    JettType type
) const
{
    return
        type == JettType::INT_ARRAY ||
        type == JettType::FL_ARRAY ||
        type == JettType::DOUB_ARRAY ||
        type == JettType::STR_ARRAY ||
        type == JettType::BOOL_ARRAY ||
        type == JettType::CHAR_ARRAY ||
        type == JettType::BYTE_ARRAY ||
        type == JettType::LONG_ARRAY;
}

JettType TypeChecker::arrayElementType(
    JettType type
) const
{
    switch (type)
    {
        case JettType::INT_ARRAY:
            return JettType::INT;

        case JettType::FL_ARRAY:
            return JettType::FL;

        case JettType::DOUB_ARRAY:
            return JettType::DOUB;

        case JettType::STR_ARRAY:
            return JettType::STR;

        case JettType::BOOL_ARRAY:
            return JettType::BOOL;

        case JettType::CHAR_ARRAY:
            return JettType::CHAR;

        case JettType::BYTE_ARRAY:
            return JettType::BYTE;

        case JettType::LONG_ARRAY:
            return JettType::LONG;

        default:
            return JettType::UNKNOWN;
    }
}

// ============================================================
// Numeric
// ============================================================

bool TypeChecker::isNumeric(
    JettType type
) const
{
    return
        type == JettType::INT ||
        type == JettType::FL ||
        type == JettType::DOUB ||
        type == JettType::BYTE ||
        type == JettType::LONG;
}

// ============================================================
// Assignment compatibility
// ============================================================

bool TypeChecker::isAssignable(
    JettType expected,
    JettType actual
) const
{
    if (expected == actual)
    {
        return true;
    }

    if (isNumeric(expected) &&
        isNumeric(actual))
    {
        return true;
    }

    return false;
}

// ============================================================
// Type -> String
// ============================================================

std::string TypeChecker::typeToString(
    JettType type
) const
{
    switch (type)
    {
        case JettType::INT:
            return "Int";

        case JettType::FL:
            return "Fl";

        case JettType::DOUB:
            return "Doub";

        case JettType::STR:
            return "Str";

        case JettType::BOOL:
            return "Bool";

        case JettType::CHAR:
            return "Char";

        case JettType::BYTE:
            return "Byte";

        case JettType::LONG:
            return "Long";

        case JettType::VOID:
            return "Void";

        case JettType::INT_ARRAY:
            return "Int[]";

        case JettType::FL_ARRAY:
            return "Fl[]";

        case JettType::DOUB_ARRAY:
            return "Doub[]";

        case JettType::STR_ARRAY:
            return "Str[]";

        case JettType::BOOL_ARRAY:
            return "Bool[]";

        case JettType::CHAR_ARRAY:
            return "Char[]";

        case JettType::BYTE_ARRAY:
            return "Byte[]";

        case JettType::LONG_ARRAY:
            return "Long[]";

        default:
            return "Unknown";
    }
}

// ============================================================
// Type Error
// ============================================================

void TypeChecker::typeError(
    const ASTNode* node,
    const std::string& message
)
{
    if (node != nullptr)
    {
        errors.push_back(
            "Error at line " +
            std::to_string(node->line) +
            ", column " +
            std::to_string(node->column) +
            ": " +
            message
        );
    }
    else
    {
        errors.push_back(
            "Error: " +
            message
        );
    }
}