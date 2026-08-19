#include "interpreter.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

// ============================================================
// Execute Program
// ============================================================

void Interpreter::execute(
    const Program& program
)
{
    variables.clear();
    functions.clear();

    for (const auto& statement :
         program.statements)
    {
        if (auto function =
            dynamic_cast<const FunctionStmt*>(
                statement.get()))
        {
            if (functions.find(function->name) !=
                functions.end())
            {
                runtimeError(
                    "Function '" +
                    function->name +
                    "' was already declared."
                );
            }

            functions[
                function->name
            ] = function;
        }
    }

    for (const auto& statement :
         program.statements)
    {
        if (dynamic_cast<const FunctionStmt*>(
                statement.get()))
        {
            continue;
        }

        executeStatement(
            statement.get()
        );
    }
}

// ============================================================
// Execute Statement
// ============================================================

void Interpreter::executeStatement(
    const Statement* statement
)
{
    if (statement == nullptr)
    {
        runtimeError(
            "Invalid statement."
        );
    }

    if (auto function =
        dynamic_cast<const FunctionStmt*>(
            statement))
    {
        (void)function;
        return;
    }

    if (auto returnStatement =
        dynamic_cast<const ReturnStmt*>(
            statement))
    {
        Value value;

        if (returnStatement->value)
        {
            value =
                evaluate(
                    returnStatement->value.get()
                );
        }
        else
        {
            value = 0LL;
        }

        throw ReturnSignal{
            std::move(value)
        };
    }

    if (dynamic_cast<const BreakStmt*>(
            statement))
    {
        throw BreakSignal{};
    }

    if (dynamic_cast<const ContinueStmt*>(
            statement))
    {
        throw ContinueSignal{};
    }

    if (auto variable =
        dynamic_cast<const VariableDeclarationStmt*>(
            statement))
    {
        executeVariableDeclaration(
            variable
        );

        return;
    }

    if (auto expression =
        dynamic_cast<const ExpressionStmt*>(
            statement))
    {
        executeExpressionStatement(
            expression
        );

        return;
    }

    if (auto ifStatement =
        dynamic_cast<const IfStmt*>(
            statement))
    {
        executeIfStatement(
            ifStatement
        );

        return;
    }

    if (auto whileStatement =
        dynamic_cast<const WhileStmt*>(
            statement))
    {
        executeWhileStatement(
            whileStatement
        );

        return;
    }

    if (auto forStatement =
        dynamic_cast<const ForStmt*>(
            statement))
    {
        executeForStatement(
            forStatement
        );

        return;
    }

    runtimeError(
        "Unknown statement."
    );
}

// ============================================================
// If
// ============================================================

void Interpreter::executeIfStatement(
    const IfStmt* statement
)
{
    Value condition =
        evaluate(
            statement->condition.get()
        );

    if (isTruthy(condition))
    {
        for (const auto& child :
             statement->thenBranch)
        {
            executeStatement(
                child.get()
            );
        }
    }
    else if (statement->hasElse)
    {
        for (const auto& child :
             statement->elseBranch)
        {
            executeStatement(
                child.get()
            );
        }
    }
}

// ============================================================
// While
// ============================================================

void Interpreter::executeWhileStatement(
    const WhileStmt* statement
)
{
    while (isTruthy(
        evaluate(
            statement->condition.get()
        )))
    {
        try
        {
            for (const auto& child :
                 statement->body)
            {
                executeStatement(
                    child.get()
                );
            }
        }
        catch (const BreakSignal&)
        {
            break;
        }
        catch (const ContinueSignal&)
        {
            continue;
        }
    }
}

// ============================================================
// For
// ============================================================

void Interpreter::executeForStatement(
    const ForStmt* statement
)
{
    if (statement->initializer)
    {
        executeStatement(
            statement->initializer.get()
        );
    }

    while (isTruthy(
        evaluate(
            statement->condition.get()
        )))
    {
        bool shouldContinue = false;

        try
        {
            for (const auto& child :
                 statement->body)
            {
                executeStatement(
                    child.get()
                );
            }
        }
        catch (const BreakSignal&)
        {
            break;
        }
        catch (const ContinueSignal&)
        {
            shouldContinue = true;
        }

        if (statement->increment)
        {
            evaluate(
                statement->increment.get()
            );
        }

        if (shouldContinue)
        {
            continue;
        }
    }
}

// ============================================================
// Execute Function
// ============================================================

void Interpreter::executeFunction(
    const FunctionStmt* function,
    const std::vector<Value>& arguments
)
{
    if (function == nullptr)
    {
        runtimeError(
            "Invalid function."
        );
    }

    if (arguments.size() !=
        function->parameters.size())
    {
        runtimeError(
            "Function '" +
            function->name +
            "' expected " +
            std::to_string(
                function->parameters.size()
            ) +
            " argument(s), but got " +
            std::to_string(
                arguments.size()
            ) +
            "."
        );
    }

    auto previousVariables =
        std::move(variables);

    variables.clear();

    for (std::size_t i = 0;
         i < function->parameters.size();
         ++i)
    {
        variables[
            function->parameters[i].name
        ] = arguments[i];
    }

    try
    {
        for (const auto& statement :
             function->body)
        {
            executeStatement(
                statement.get()
            );
        }
    }
    catch (...)
    {
        variables =
            std::move(previousVariables);

        throw;
    }

    variables =
        std::move(previousVariables);
}

// ============================================================
// Variable Declaration
// ============================================================

void Interpreter::executeVariableDeclaration(
    const VariableDeclarationStmt* statement
)
{
    Value value =
        evaluate(
            statement->initializer.get()
        );

    variables[
        statement->name
    ] = value;
}

// ============================================================
// Expression Statement
// ============================================================

void Interpreter::executeExpressionStatement(
    const ExpressionStmt* statement
)
{
    evaluate(
        statement->expression.get()
    );
}

// ============================================================
// Evaluate Expression
// ============================================================

Interpreter::Value
Interpreter::evaluate(
    const Expression* expression
)
{
    if (expression == nullptr)
    {
        runtimeError(
            "Invalid expression."
        );
    }

    if (auto literal =
        dynamic_cast<const LiteralExpr*>(
            expression))
    {
        return evaluateLiteral(
            literal
        );
    }

    if (auto array =
        dynamic_cast<const ArrayLiteralExpr*>(
            expression))
    {
        return evaluateArrayLiteral(
            array
        );
    }

    if (auto variable =
        dynamic_cast<const VariableExpr*>(
            expression))
    {
        return evaluateVariable(
            variable
        );
    }

    if (auto index =
        dynamic_cast<const IndexExpr*>(
            expression))
    {
        return evaluateIndex(
            index
        );
    }

    if (auto indexAssignment =
        dynamic_cast<const IndexAssignmentExpr*>(
            expression))
    {
        return evaluateIndexAssignment(
            indexAssignment
        );
    }

    if (auto assignment =
        dynamic_cast<const AssignmentExpr*>(
            expression))
    {
        return evaluateAssignment(
            assignment
        );
    }

    if (auto binary =
        dynamic_cast<const BinaryExpr*>(
            expression))
    {
        return evaluateBinary(
            binary
        );
    }

    if (auto unary =
        dynamic_cast<const UnaryExpr*>(
            expression))
    {
        return evaluateUnary(
            unary
        );
    }

    if (auto call =
        dynamic_cast<const CallExpr*>(
            expression))
    {
        return evaluateCall(
            call
        );
    }

    runtimeError(
        "Unknown expression."
    );
}

// ============================================================
// Literal
// ============================================================

Interpreter::Value
Interpreter::evaluateLiteral(
    const LiteralExpr* expression
)
{
    return std::visit(
        [](const auto& value) -> Interpreter::Value
        {
            return Interpreter::Value(value);
        },
        expression->value
    );
}

// ============================================================
// Array Literal
// ============================================================

Interpreter::Value
Interpreter::evaluateArrayLiteral(
    const ArrayLiteralExpr* expression
)
{
    auto array =
        std::make_shared<ArrayValue>();

    for (const auto& element :
         expression->elements)
    {
        array->elements.push_back(
            evaluate(
                element.get()
            )
        );
    }

    return array;
}

// ============================================================
// Variable
// ============================================================

Interpreter::Value
Interpreter::evaluateVariable(
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
        runtimeError(
            "Variable '" +
            expression->name +
            "' was not declared."
        );
    }

    return found->second;
}

// ============================================================
// Array Index
// ============================================================

Interpreter::Value
Interpreter::evaluateIndex(
    const IndexExpr* expression
)
{
    Value arrayValue =
        evaluate(
            expression->array.get()
        );

    Value indexValue =
        evaluate(
            expression->index.get()
        );

    if (!std::holds_alternative<
            std::shared_ptr<ArrayValue>>(
            arrayValue))
    {
        runtimeError(
            "Only arrays can be indexed."
        );
    }

    if (!std::holds_alternative<long long>(
            indexValue))
    {
        runtimeError(
            "Array index must be an integer."
        );
    }

    long long index =
        std::get<long long>(
            indexValue
        );

    auto array =
        std::get<
            std::shared_ptr<ArrayValue>>(
                arrayValue
            );

    if (index < 0 ||
        static_cast<std::size_t>(index) >=
            array->elements.size())
    {
        runtimeError(
            "Array index out of bounds."
        );
    }

    return array->elements[
        static_cast<std::size_t>(index)
    ];
}

// ============================================================
// Array Index Assignment
// ============================================================

Interpreter::Value
Interpreter::evaluateIndexAssignment(
    const IndexAssignmentExpr* expression
)
{
    auto index =
        dynamic_cast<const IndexExpr*>(
            expression->target.get()
        );

    if (index == nullptr)
    {
        runtimeError(
            "Invalid array assignment target."
        );
    }

    Value arrayValue =
        evaluate(
            index->array.get()
        );

    if (!std::holds_alternative<
            std::shared_ptr<ArrayValue>>(
            arrayValue))
    {
        runtimeError(
            "Only arrays can be indexed."
        );
    }

    auto array =
        std::get<
            std::shared_ptr<ArrayValue>>(
                arrayValue
            );

    Value indexValue =
        evaluate(
            index->index.get()
        );

    if (!std::holds_alternative<long long>(
            indexValue))
    {
        runtimeError(
            "Array index must be an integer."
        );
    }

    long long indexNumber =
        std::get<long long>(
            indexValue
        );

    if (indexNumber < 0 ||
        static_cast<std::size_t>(
            indexNumber
        ) >= array->elements.size())
    {
        runtimeError(
            "Array index out of bounds."
        );
    }

    Value value =
        evaluate(
            expression->value.get()
        );

    array->elements[
        static_cast<std::size_t>(
            indexNumber
        )
    ] = value;

    return value;
}

// ============================================================
// Assignment
// ============================================================

Interpreter::Value
Interpreter::evaluateAssignment(
    const AssignmentExpr* expression
)
{
    Value value =
        evaluate(
            expression->value.get()
        );

    auto found =
        variables.find(
            expression->name
        );

    if (found ==
        variables.end())
    {
        runtimeError(
            "Variable '" +
            expression->name +
            "' was not declared."
        );
    }

    found->second = value;

    return value;
}

// ============================================================
// Binary
// ============================================================

Interpreter::Value
Interpreter::evaluateBinary(
    const BinaryExpr* expression
)
{
    Value left =
        evaluate(
            expression->left.get()
        );

    Value right =
        evaluate(
            expression->right.get()
        );

    auto numericValue =
        [](const Value& value) -> double
    {
        if (std::holds_alternative<long long>(
                value))
        {
            return static_cast<double>(
                std::get<long long>(value)
            );
        }

        if (std::holds_alternative<double>(
                value))
        {
            return std::get<double>(
                value
            );
        }

        return 0.0;
    };

    switch (expression->op)
    {
        case TokenType::PLUS:
        {
            if (std::holds_alternative<std::string>(left) &&
                std::holds_alternative<std::string>(right))
            {
                return
                    std::get<std::string>(left) +
                    std::get<std::string>(right);
            }

            if (std::holds_alternative<long long>(left) &&
                std::holds_alternative<long long>(right))
            {
                return
                    std::get<long long>(left) +
                    std::get<long long>(right);
            }

            return
                numericValue(left) +
                numericValue(right);
        }

        case TokenType::MINUS:
        {
            if (std::holds_alternative<long long>(left) &&
                std::holds_alternative<long long>(right))
            {
                return
                    std::get<long long>(left) -
                    std::get<long long>(right);
            }

            return
                numericValue(left) -
                numericValue(right);
        }

        case TokenType::STAR:
        {
            if (std::holds_alternative<long long>(left) &&
                std::holds_alternative<long long>(right))
            {
                return
                    std::get<long long>(left) *
                    std::get<long long>(right);
            }

            return
                numericValue(left) *
                numericValue(right);
        }

        case TokenType::SLASH:
        {
            double divisor =
                numericValue(right);

            if (divisor == 0.0)
            {
                runtimeError(
                    "Division by zero."
                );
            }

            if (std::holds_alternative<long long>(left) &&
                std::holds_alternative<long long>(right))
            {
                return
                    std::get<long long>(left) /
                    std::get<long long>(right);
            }

            return
                numericValue(left) /
                divisor;
        }

        case TokenType::PERCENT:
        {
            if (!std::holds_alternative<long long>(left) ||
                !std::holds_alternative<long long>(right))
            {
                runtimeError(
                    "The '%' operator requires integers."
                );
            }

            long long divisor =
                std::get<long long>(right);

            if (divisor == 0)
            {
                runtimeError(
                    "Modulo by zero."
                );
            }

            return
                std::get<long long>(left) %
                divisor;
        }

        case TokenType::GREATER:
            return
                numericValue(left) >
                numericValue(right);

        case TokenType::GREATER_EQUAL:
            return
                numericValue(left) >=
                numericValue(right);

        case TokenType::LESS:
            return
                numericValue(left) <
                numericValue(right);

        case TokenType::LESS_EQUAL:
            return
                numericValue(left) <=
                numericValue(right);

        case TokenType::EQUAL_EQUAL:
        {
            if (left.index() !=
                right.index())
            {
                bool leftNumeric =
                    std::holds_alternative<long long>(left) ||
                    std::holds_alternative<double>(left);

                bool rightNumeric =
                    std::holds_alternative<long long>(right) ||
                    std::holds_alternative<double>(right);

                if (leftNumeric &&
                    rightNumeric)
                {
                    return
                        numericValue(left) ==
                        numericValue(right);
                }

                return false;
            }

            return left == right;
        }

        case TokenType::NOT_EQUAL:
        {
            if (left.index() !=
                right.index())
            {
                bool leftNumeric =
                    std::holds_alternative<long long>(left) ||
                    std::holds_alternative<double>(left);

                bool rightNumeric =
                    std::holds_alternative<long long>(right) ||
                    std::holds_alternative<double>(right);

                if (leftNumeric &&
                    rightNumeric)
                {
                    return
                        numericValue(left) !=
                        numericValue(right);
                }

                return true;
            }

            return left != right;
        }

        case TokenType::AND:
            return
                isTruthy(left) &&
                isTruthy(right);

        case TokenType::OR:
            return
                isTruthy(left) ||
                isTruthy(right);

        default:
            break;
    }

    runtimeError(
        "Unsupported binary operator."
    );
}

// ============================================================
// Unary
// ============================================================

Interpreter::Value
Interpreter::evaluateUnary(
    const UnaryExpr* expression
)
{
    Value right =
        evaluate(
            expression->right.get()
        );

    switch (expression->op)
    {
        case TokenType::MINUS:
        {
            if (std::holds_alternative<long long>(right))
            {
                return
                    -std::get<long long>(right);
            }

            if (std::holds_alternative<double>(right))
            {
                return
                    -std::get<double>(right);
            }

            runtimeError(
                "Unary '-' requires a number."
            );
        }

        case TokenType::PLUS:
        {
            if (std::holds_alternative<long long>(right) ||
                std::holds_alternative<double>(right))
            {
                return right;
            }

            runtimeError(
                "Unary '+' requires a number."
            );
        }

        case TokenType::NOT:
            return !isTruthy(right);

        default:
            break;
    }

    runtimeError(
        "Unsupported unary operator."
    );
}

// ============================================================
// Function Call + Built-ins
// ============================================================

Interpreter::Value
Interpreter::evaluateCall(
    const CallExpr* expression
)
{
    auto variable =
        dynamic_cast<const VariableExpr*>(
            expression->callee.get()
        );

    if (!variable)
    {
        runtimeError(
            "Invalid function call."
        );
    }

    const std::string& name =
        variable->name;

    // ========================================================
    // print(...)
    // ========================================================

    if (name == "print")
    {
        callPrint(
            expression
        );

        return 0LL;
    }

    // ========================================================
    // input([prompt])
    // ========================================================

    if (name == "input")
    {
        if (expression->arguments.size() > 1)
        {
            runtimeError(
                "input() accepts zero or one argument."
            );
        }

        if (!expression->arguments.empty())
        {
            Value prompt =
                evaluate(
                    expression->arguments[0].get()
                );

            if (!std::holds_alternative<std::string>(
                    prompt))
            {
                runtimeError(
                    "input() prompt must be a string."
                );
            }

            std::cout
                << std::get<std::string>(
                    prompt
                );
        }

        std::string input;

        if (!std::getline(
                std::cin,
                input))
        {
            return std::string();
        }

        return input;
    }

    // ========================================================
    // length(value)
    // ========================================================

    if (name == "length")
    {
        if (expression->arguments.size() != 1)
        {
            runtimeError(
                "length() expects exactly one argument."
            );
        }

        Value value =
            evaluate(
                expression->arguments[0].get()
            );

        if (std::holds_alternative<std::string>(
                value))
        {
            return static_cast<long long>(
                std::get<std::string>(
                    value
                ).size()
            );
        }

        if (std::holds_alternative<
                std::shared_ptr<ArrayValue>>(
                value))
        {
            auto array =
                std::get<
                    std::shared_ptr<ArrayValue>>(
                        value
                    );

            return static_cast<long long>(
                array->elements.size()
            );
        }

        runtimeError(
            "length() expects a string or array."
        );
    }

    // ========================================================
    // upper(str)
    // ========================================================

    if (name == "upper")
    {
        if (expression->arguments.size() != 1)
        {
            runtimeError(
                "upper() expects exactly one argument."
            );
        }

        Value value =
            evaluate(
                expression->arguments[0].get()
            );

        if (!std::holds_alternative<std::string>(
                value))
        {
            runtimeError(
                "upper() expects a string."
            );
        }

        std::string result =
            std::get<std::string>(
                value
            );

        std::transform(
            result.begin(),
            result.end(),
            result.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(
                    std::toupper(c)
                );
            }
        );

        return result;
    }

    // ========================================================
    // lower(str)
    // ========================================================

    if (name == "lower")
    {
        if (expression->arguments.size() != 1)
        {
            runtimeError(
                "lower() expects exactly one argument."
            );
        }

        Value value =
            evaluate(
                expression->arguments[0].get()
            );

        if (!std::holds_alternative<std::string>(
                value))
        {
            runtimeError(
                "lower() expects a string."
            );
        }

        std::string result =
            std::get<std::string>(
                value
            );

        std::transform(
            result.begin(),
            result.end(),
            result.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(
                    std::tolower(c)
                );
            }
        );

        return result;
    }

    // ========================================================
    // trim(str)
    // ========================================================

    if (name == "trim")
    {
        if (expression->arguments.size() != 1)
        {
            runtimeError(
                "trim() expects exactly one argument."
            );
        }

        Value value =
            evaluate(
                expression->arguments[0].get()
            );

        if (!std::holds_alternative<std::string>(
                value))
        {
            runtimeError(
                "trim() expects a string."
            );
        }

        const std::string& text =
            std::get<std::string>(
                value
            );

        std::size_t start = 0;
        std::size_t end = text.size();

        while (start < end &&
               std::isspace(
                   static_cast<unsigned char>(
                       text[start]
                   )))
        {
            start++;
        }

        while (end > start &&
               std::isspace(
                   static_cast<unsigned char>(
                       text[end - 1]
                   )))
        {
            end--;
        }

        return text.substr(
            start,
            end - start
        );
    }

    // ========================================================
    // contains(str, sub)
    // ========================================================

    if (name == "contains")
    {
        if (expression->arguments.size() != 2)
        {
            runtimeError(
                "contains() expects two arguments."
            );
        }

        Value text =
            evaluate(
                expression->arguments[0].get()
            );

        Value sub =
            evaluate(
                expression->arguments[1].get()
            );

        if (!std::holds_alternative<std::string>(text) ||
            !std::holds_alternative<std::string>(sub))
        {
            runtimeError(
                "contains() expects two strings."
            );
        }

        return
            std::get<std::string>(text)
                .find(
                    std::get<std::string>(sub)
                )
            != std::string::npos;
    }

    // ========================================================
    // startsWith(str, prefix)
    // ========================================================

    if (name == "startsWith")
    {
        if (expression->arguments.size() != 2)
        {
            runtimeError(
                "startsWith() expects two arguments."
            );
        }

        Value text =
            evaluate(
                expression->arguments[0].get()
            );

        Value prefix =
            evaluate(
                expression->arguments[1].get()
            );

        if (!std::holds_alternative<std::string>(text) ||
            !std::holds_alternative<std::string>(prefix))
        {
            runtimeError(
                "startsWith() expects two strings."
            );
        }

        const std::string& a =
            std::get<std::string>(text);

        const std::string& b =
            std::get<std::string>(prefix);

        return
            a.size() >= b.size() &&
            a.compare(
                0,
                b.size(),
                b
            ) == 0;
    }

    // ========================================================
    // endsWith(str, suffix)
    // ========================================================

    if (name == "endsWith")
    {
        if (expression->arguments.size() != 2)
        {
            runtimeError(
                "endsWith() expects two arguments."
            );
        }

        Value text =
            evaluate(
                expression->arguments[0].get()
            );

        Value suffix =
            evaluate(
                expression->arguments[1].get()
            );

        if (!std::holds_alternative<std::string>(text) ||
            !std::holds_alternative<std::string>(suffix))
        {
            runtimeError(
                "endsWith() expects two strings."
            );
        }

        const std::string& a =
            std::get<std::string>(text);

        const std::string& b =
            std::get<std::string>(suffix);

        return
            a.size() >= b.size() &&
            a.compare(
                a.size() - b.size(),
                b.size(),
                b
            ) == 0;
    }

    // ========================================================
    // substring(str, start, length)
    // ========================================================

    if (name == "substring")
    {
        if (expression->arguments.size() != 3)
        {
            runtimeError(
                "substring() expects three arguments."
            );
        }

        Value text =
            evaluate(
                expression->arguments[0].get()
            );

        Value start =
            evaluate(
                expression->arguments[1].get()
            );

        Value length =
            evaluate(
                expression->arguments[2].get()
            );

        if (!std::holds_alternative<std::string>(text))
        {
            runtimeError(
                "substring() first argument must be a string."
            );
        }

        if (!std::holds_alternative<long long>(start) ||
            !std::holds_alternative<long long>(length))
        {
            runtimeError(
                "substring() start and length must be Int."
            );
        }

        long long startValue =
            std::get<long long>(start);

        long long lengthValue =
            std::get<long long>(length);

        const std::string& source =
            std::get<std::string>(text);

        if (startValue < 0 ||
            lengthValue < 0 ||
            static_cast<std::size_t>(startValue) >
                source.size())
        {
            runtimeError(
                "substring() range is out of bounds."
            );
        }

        return source.substr(
            static_cast<std::size_t>(startValue),
            static_cast<std::size_t>(lengthValue)
        );
    }

    // ========================================================
    // User-defined function
    // ========================================================

    auto found =
        functions.find(name);

    if (found ==
        functions.end())
    {
        runtimeError(
            "Function '" +
            name +
            "' was not found."
        );
    }

    std::vector<Value>
        arguments;

    for (const auto& argument :
         expression->arguments)
    {
        arguments.push_back(
            evaluate(
                argument.get()
            )
        );
    }

    try
    {
        executeFunction(
            found->second,
            arguments
        );
    }
    catch (const ReturnSignal& signal)
    {
        return signal.value;
    }

    return 0LL;
}

// ============================================================
// Print
// ============================================================

void Interpreter::callPrint(
    const CallExpr* expression
)
{
    for (std::size_t i = 0;
         i < expression->arguments.size();
         ++i)
    {
        Value value =
            evaluate(
                expression->arguments[i].get()
            );

        std::cout
            << valueToString(value);

        if (i + 1 <
            expression->arguments.size())
        {
            std::cout << ' ';
        }
    }

    std::cout << '\n';
}

// ============================================================
// Value -> String
// ============================================================

std::string Interpreter::valueToString(
    const Value& value
) const
{
    if (std::holds_alternative<long long>(
            value))
    {
        return std::to_string(
            std::get<long long>(value)
        );
    }

    if (std::holds_alternative<double>(
            value))
    {
        std::ostringstream output;

        output
            << std::get<double>(value);

        return output.str();
    }

    if (std::holds_alternative<std::string>(
            value))
    {
        return
            std::get<std::string>(value);
    }

    if (std::holds_alternative<bool>(
            value))
    {
        return
            std::get<bool>(value)
                ? "true"
                : "false";
    }

    if (std::holds_alternative<char>(
            value))
    {
        return std::string(
            1,
            std::get<char>(value)
        );
    }

    if (std::holds_alternative<
            std::shared_ptr<ArrayValue>>(
            value))
    {
        auto array =
            std::get<
                std::shared_ptr<ArrayValue>>(
                    value
                );

        std::ostringstream output;

        output << "[";

        for (std::size_t i = 0;
             i < array->elements.size();
             ++i)
        {
            output
                << valueToString(
                    array->elements[i]
                );

            if (i + 1 <
                array->elements.size())
            {
                output << ", ";
            }
        }

        output << "]";

        return output.str();
    }

    return "null";
}

// ============================================================
// Truthiness
// ============================================================

bool Interpreter::isTruthy(
    const Value& value
) const
{
    if (std::holds_alternative<bool>(value))
    {
        return
            std::get<bool>(value);
    }

    if (std::holds_alternative<long long>(value))
    {
        return
            std::get<long long>(value) != 0;
    }

    if (std::holds_alternative<double>(value))
    {
        return
            std::get<double>(value) != 0.0;
    }

    if (std::holds_alternative<std::string>(value))
    {
        return
            !std::get<std::string>(value).empty();
    }

    if (std::holds_alternative<char>(value))
    {
        return
            std::get<char>(value) != '\0';
    }

    if (std::holds_alternative<
            std::shared_ptr<ArrayValue>>(value))
    {
        auto array =
            std::get<
                std::shared_ptr<ArrayValue>>(
                    value
                );

        return
            !array->elements.empty();
    }

    return false;
}

// ============================================================
// Runtime Error
// ============================================================

[[noreturn]]
void Interpreter::runtimeError(
    const std::string& message
) const
{
    std::cerr
        << "Runtime Error: "
        << message
        << '\n';

    std::exit(1);
}