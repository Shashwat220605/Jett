#include "type_checker.h"

#include <utility>

void TypeChecker::check(const Program& program)
{
    variables.clear();
    functions.clear();
    errors.clear();
    currentFunction = nullptr;
    checkingFunction = false;

    // Register every function before checking any body. This makes
    // forward calls and recursive calls visible to the type checker.
    for (const auto& statement : program.statements)
    {
        auto function = dynamic_cast<const FunctionStmt*>(statement.get());
        if (!function) continue;

        if (functions.find(function->name) != functions.end())
        {
            typeError(function, "Function '" + function->name + "' has already been declared.");
            continue;
        }

        FunctionInfo info;
        for (const auto& parameter : function->parameters)
        {
            JettType type = typeFromName(parameter.type);
            if (type == JettType::UNKNOWN)
            {
                typeError(function, "Unknown parameter type '" + parameter.type + "'.");
            }
            info.parameterTypes.push_back(type);
        }
        functions.emplace(function->name, info);
    }

    // Infer function return types first. Recursive calls are allowed to be
    // UNKNOWN during inference; concrete return statements establish the type.
    for (const auto& statement : program.statements)
    {
        auto function = dynamic_cast<const FunctionStmt*>(statement.get());
        if (function) checkFunctionStatement(function);
    }

    // Re-check function bodies now that inferred return types are known. This
    // validates recursive calls and return expressions against their final type.
    for (const auto& statement : program.statements)
    {
        auto function = dynamic_cast<const FunctionStmt*>(statement.get());
        if (!function) continue;

        auto previousVariables = std::move(variables);
        const FunctionStmt* previousFunction = currentFunction;
        JettType previousReturn = inferredReturnType;
        bool previousChecking = checkingFunction;

        variables.clear();
        currentFunction = function;
        checkingFunction = true;
        inferredReturnType = functions[function->name].returnType;

        for (std::size_t i = 0; i < function->parameters.size(); ++i)
        {
            variables[function->parameters[i].name] =
                functions[function->name].parameterTypes[i];
        }

        for (const auto& child : function->body)
            checkStatement(child.get());

        variables = std::move(previousVariables);
        currentFunction = previousFunction;
        inferredReturnType = previousReturn;
        checkingFunction = previousChecking;
    }

    // Finally check top-level statements.
    for (const auto& statement : program.statements)
    {
        if (!dynamic_cast<const FunctionStmt*>(statement.get()))
            checkStatement(statement.get());
    }
}

bool TypeChecker::hasErrors() const
{
    return !errors.empty();
}

const std::vector<std::string>& TypeChecker::getErrors() const
{
    return errors;
}

void TypeChecker::checkStatement(const Statement* statement)
{
    if (!statement)
    {
        typeError(nullptr, "Invalid statement.");
        return;
    }

    if (auto s = dynamic_cast<const VariableDeclarationStmt*>(statement))
        return checkVariableDeclaration(s);
    if (auto s = dynamic_cast<const ExpressionStmt*>(statement))
        return checkExpressionStatement(s);
    if (auto s = dynamic_cast<const IfStmt*>(statement))
        return checkIfStatement(s);
    if (auto s = dynamic_cast<const WhileStmt*>(statement))
        return checkWhileStatement(s);
    if (auto s = dynamic_cast<const ForStmt*>(statement))
        return checkForStatement(s);
    if (auto s = dynamic_cast<const FunctionStmt*>(statement))
        return checkFunctionStatement(s);
    if (auto s = dynamic_cast<const ReturnStmt*>(statement))
        return checkReturnStatement(s);
    if (auto s = dynamic_cast<const BreakStmt*>(statement))
        return checkBreakStatement(s);
    if (auto s = dynamic_cast<const ContinueStmt*>(statement))
        return checkContinueStatement(s);

    typeError(statement, "Unknown statement.");
}

void TypeChecker::checkIfStatement(const IfStmt* statement)
{
    JettType type = checkExpression(statement->condition.get());
    if (type != JettType::UNKNOWN && type != JettType::BOOL)
        typeError(statement->condition.get(), "If condition must be Bool, but received " + typeToString(type) + ".");

    for (const auto& child : statement->thenBranch) checkStatement(child.get());
    if (statement->hasElse)
        for (const auto& child : statement->elseBranch) checkStatement(child.get());
}

void TypeChecker::checkWhileStatement(const WhileStmt* statement)
{
    JettType type = checkExpression(statement->condition.get());
    if (type != JettType::UNKNOWN && type != JettType::BOOL)
        typeError(statement->condition.get(), "While condition must be Bool, but received " + typeToString(type) + ".");
    for (const auto& child : statement->body) checkStatement(child.get());
}

void TypeChecker::checkForStatement(const ForStmt* statement)
{
    if (statement->initializer) checkStatement(statement->initializer.get());
    if (statement->condition)
    {
        JettType type = checkExpression(statement->condition.get());
        if (type != JettType::UNKNOWN && type != JettType::BOOL)
            typeError(statement->condition.get(), "For condition must be Bool, but received " + typeToString(type) + ".");
    }
    for (const auto& child : statement->body) checkStatement(child.get());
    if (statement->increment) checkExpression(statement->increment.get());
}

void TypeChecker::checkBreakStatement(const BreakStmt*) {}
void TypeChecker::checkContinueStatement(const ContinueStmt*) {}

void TypeChecker::checkFunctionStatement(const FunctionStmt* statement)
{
    auto found = functions.find(statement->name);
    if (found == functions.end()) return;

    auto previousVariables = std::move(variables);
    const FunctionStmt* previousFunction = currentFunction;
    JettType previousReturn = inferredReturnType;
    bool previousChecking = checkingFunction;

    variables.clear();
    currentFunction = statement;
    checkingFunction = true;
    inferredReturnType = found->second.returnType;

    for (std::size_t i = 0; i < statement->parameters.size(); ++i)
    {
        JettType type = found->second.parameterTypes[i];
        if (type != JettType::UNKNOWN)
            variables[statement->parameters[i].name] = type;
    }

    // During the inference pass, returnType is UNKNOWN. Concrete returns update it.
    for (const auto& child : statement->body) checkStatement(child.get());

    if (inferredReturnType == JettType::UNKNOWN)
    {
        typeError(statement, "Could not infer a return type for function '" + statement->name + "'. Add a return statement with a concrete value.");
    }
    else
    {
        found->second.returnType = inferredReturnType;
    }

    variables = std::move(previousVariables);
    currentFunction = previousFunction;
    inferredReturnType = previousReturn;
    checkingFunction = previousChecking;
}

void TypeChecker::checkReturnStatement(const ReturnStmt* statement)
{
    if (!checkingFunction || currentFunction == nullptr)
    {
        typeError(statement, "'return' can only be used inside a function.");
        return;
    }

    JettType type = statement->value
        ? checkExpression(statement->value.get())
        : JettType::VOID;

    if (type == JettType::UNKNOWN) return;

    if (inferredReturnType == JettType::UNKNOWN)
    {
        inferredReturnType = type;
        return;
    }

    if (!isAssignable(inferredReturnType, type))
    {
        typeError(statement, "Function '" + currentFunction->name + "' returns both " +
            typeToString(inferredReturnType) + " and " + typeToString(type) + ".");
    }
}

void TypeChecker::checkVariableDeclaration(const VariableDeclarationStmt* statement)
{
    if (variables.find(statement->name) != variables.end())
    {
        typeError(statement, "Variable '" + statement->name + "' has already been declared.");
        return;
    }

    JettType actual = checkExpression(statement->initializer.get());
    if (actual == JettType::UNKNOWN) return;

    if (statement->declaredType.empty())
    {
        variables[statement->name] = actual;
        return;
    }

    JettType expected = typeFromName(statement->declaredType);
    if (expected == JettType::UNKNOWN)
    {
        typeError(statement, "Unknown type '" + statement->declaredType + "'.");
        return;
    }

    if (!isAssignable(expected, actual))
    {
        typeError(statement, "'" + statement->name + "' expects " + typeToString(expected) +
            " but received " + typeToString(actual) + ".");
        return;
    }

    variables[statement->name] = expected;
}

void TypeChecker::checkExpressionStatement(const ExpressionStmt* statement)
{
    checkExpression(statement->expression.get());
}

JettType TypeChecker::checkExpression(const Expression* expression)
{
    if (!expression)
    {
        typeError(nullptr, "Invalid expression.");
        return JettType::UNKNOWN;
    }

    if (auto e = dynamic_cast<const LiteralExpr*>(expression)) return checkLiteral(e);
    if (auto e = dynamic_cast<const ArrayLiteralExpr*>(expression)) return checkArrayLiteral(e);
    if (auto e = dynamic_cast<const VariableExpr*>(expression)) return checkVariable(e);

    if (auto e = dynamic_cast<const AssignmentExpr*>(expression))
    {
        JettType value = checkExpression(e->value.get());
        auto it = variables.find(e->name);
        if (it == variables.end())
        {
            typeError(e, "Variable '" + e->name + "' was not declared.");
            return JettType::UNKNOWN;
        }
        if (value != JettType::UNKNOWN && !isAssignable(it->second, value))
        {
            typeError(e, "Cannot assign " + typeToString(value) + " to " + typeToString(it->second) + ".");
            return JettType::UNKNOWN;
        }
        return it->second;
    }

    if (auto e = dynamic_cast<const IndexExpr*>(expression)) return checkIndex(e);
    if (auto e = dynamic_cast<const IndexAssignmentExpr*>(expression)) return checkIndexAssignment(e);
    if (auto e = dynamic_cast<const BinaryExpr*>(expression)) return checkBinary(e);
    if (auto e = dynamic_cast<const UnaryExpr*>(expression)) return checkUnary(e);
    if (auto e = dynamic_cast<const CallExpr*>(expression)) return checkCall(e);

    typeError(expression, "Unknown expression.");
    return JettType::UNKNOWN;
}

JettType TypeChecker::checkLiteral(const LiteralExpr* expression)
{
    if (std::holds_alternative<long long>(expression->value)) return JettType::INT;
    if (std::holds_alternative<double>(expression->value)) return JettType::DOUB;
    if (std::holds_alternative<std::string>(expression->value)) return JettType::STR;
    if (std::holds_alternative<bool>(expression->value)) return JettType::BOOL;
    if (std::holds_alternative<char>(expression->value)) return JettType::CHAR;
    return JettType::UNKNOWN;
}

JettType TypeChecker::checkArrayLiteral(const ArrayLiteralExpr* expression)
{
    if (expression->elements.empty())
    {
        typeError(expression, "Empty arrays are not supported yet.");
        return JettType::UNKNOWN;
    }

    JettType element = JettType::UNKNOWN;
    for (const auto& value : expression->elements)
    {
        JettType current = checkExpression(value.get());
        if (current == JettType::UNKNOWN) return JettType::UNKNOWN;
        if (element == JettType::UNKNOWN) element = current;
        else if (element != current && !(isNumeric(element) && isNumeric(current)))
        {
            typeError(value.get(), "All array elements must have the same type.");
            return JettType::UNKNOWN;
        }
    }

    switch (element)
    {
        case JettType::INT: return JettType::INT_ARRAY;
        case JettType::FL: return JettType::FL_ARRAY;
        case JettType::DOUB: return JettType::DOUB_ARRAY;
        case JettType::STR: return JettType::STR_ARRAY;
        case JettType::BOOL: return JettType::BOOL_ARRAY;
        case JettType::CHAR: return JettType::CHAR_ARRAY;
        case JettType::BYTE: return JettType::BYTE_ARRAY;
        case JettType::LONG: return JettType::LONG_ARRAY;
        default: return JettType::UNKNOWN;
    }
}

JettType TypeChecker::checkVariable(const VariableExpr* expression)
{
    auto it = variables.find(expression->name);
    if (it == variables.end())
    {
        typeError(expression, "Variable '" + expression->name + "' was not declared.");
        return JettType::UNKNOWN;
    }
    return it->second;
}

JettType TypeChecker::checkIndex(const IndexExpr* expression)
{
    JettType array = checkExpression(expression->array.get());
    JettType index = checkExpression(expression->index.get());
    if (index != JettType::UNKNOWN && index != JettType::INT)
    {
        typeError(expression->index.get(), "Array index must be Int.");
        return JettType::UNKNOWN;
    }
    if (array == JettType::UNKNOWN) return JettType::UNKNOWN;
    if (!isArray(array))
    {
        typeError(expression->array.get(), "Only arrays can be indexed.");
        return JettType::UNKNOWN;
    }
    return arrayElementType(array);
}

JettType TypeChecker::checkIndexAssignment(const IndexAssignmentExpr* expression)
{
    auto index = dynamic_cast<const IndexExpr*>(expression->target.get());
    if (!index)
    {
        typeError(expression, "Invalid array assignment target.");
        return JettType::UNKNOWN;
    }

    JettType array = checkExpression(index->array.get());
    JettType indexType = checkExpression(index->index.get());
    JettType value = checkExpression(expression->value.get());
    if (indexType != JettType::UNKNOWN && indexType != JettType::INT)
    {
        typeError(index->index.get(), "Array index must be Int.");
        return JettType::UNKNOWN;
    }
    if (!isArray(array))
    {
        typeError(index->array.get(), "Only arrays can be indexed.");
        return JettType::UNKNOWN;
    }
    JettType element = arrayElementType(array);
    if (value != JettType::UNKNOWN && !isAssignable(element, value))
    {
        typeError(expression, "Cannot assign " + typeToString(value) +
            " to array element of type " + typeToString(element) + ".");
        return JettType::UNKNOWN;
    }
    return element;
}

JettType TypeChecker::checkBinary(const BinaryExpr* expression)
{
    JettType left = checkExpression(expression->left.get());
    JettType right = checkExpression(expression->right.get());
    if (left == JettType::UNKNOWN || right == JettType::UNKNOWN) return JettType::UNKNOWN;

    switch (expression->op)
    {
        case TokenType::PLUS:
            if (left == JettType::STR && right == JettType::STR) return JettType::STR;
            [[fallthrough]];
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
            if (isNumeric(left) && isNumeric(right))
            {
                if (left == JettType::DOUB || right == JettType::DOUB) return JettType::DOUB;
                if (left == JettType::FL || right == JettType::FL) return JettType::FL;
                if (left == JettType::LONG || right == JettType::LONG) return JettType::LONG;
                return JettType::INT;
            }
            typeError(expression, "Arithmetic operator cannot be used with " + typeToString(left) + " and " + typeToString(right) + ".");
            return JettType::UNKNOWN;

        case TokenType::GREATER:
        case TokenType::GREATER_EQUAL:
        case TokenType::LESS:
        case TokenType::LESS_EQUAL:
            if (isNumeric(left) && isNumeric(right)) return JettType::BOOL;
            typeError(expression, "Comparison requires numeric values.");
            return JettType::UNKNOWN;

        case TokenType::EQUAL_EQUAL:
        case TokenType::NOT_EQUAL:
            if (left == right || (isNumeric(left) && isNumeric(right))) return JettType::BOOL;
            typeError(expression, "Cannot compare " + typeToString(left) + " with " + typeToString(right) + ".");
            return JettType::UNKNOWN;

        case TokenType::AND:
        case TokenType::OR:
            if (left == JettType::BOOL && right == JettType::BOOL) return JettType::BOOL;
            typeError(expression, "Logical operators require Bool values.");
            return JettType::UNKNOWN;

        default:
            typeError(expression, "Unsupported binary operator.");
            return JettType::UNKNOWN;
    }
}

JettType TypeChecker::checkUnary(const UnaryExpr* expression)
{
    JettType type = checkExpression(expression->right.get());
    if (type == JettType::UNKNOWN) return type;

    if ((expression->op == TokenType::PLUS || expression->op == TokenType::MINUS) && isNumeric(type)) return type;
    if (expression->op == TokenType::NOT && type == JettType::BOOL) return JettType::BOOL;

    typeError(expression, "Invalid unary operator for type " + typeToString(type) + ".");
    return JettType::UNKNOWN;
}

JettType TypeChecker::checkCall(const CallExpr* expression)
{
    auto variable = dynamic_cast<const VariableExpr*>(expression->callee.get());
    if (!variable)
    {
        typeError(expression, "Invalid function call.");
        return JettType::UNKNOWN;
    }

    const std::string& name = variable->name;

    if (name == "print")
    {
        for (const auto& argument : expression->arguments) checkExpression(argument.get());
        return JettType::VOID;
    }

    if (name == "input")
    {
        if (expression->arguments.size() > 1)
        {
            typeError(expression, "input() accepts zero or one argument.");
            return JettType::UNKNOWN;
        }
        if (!expression->arguments.empty())
        {
            JettType type = checkExpression(expression->arguments[0].get());
            if (type != JettType::UNKNOWN && type != JettType::STR)
                typeError(expression->arguments[0].get(), "input() prompt must be Str.");
        }
        return JettType::STR;
    }

    if (name == "length")
    {
        if (expression->arguments.size() != 1)
        {
            typeError(expression, "length() expects exactly one argument.");
            return JettType::UNKNOWN;
        }
        JettType type = checkExpression(expression->arguments[0].get());
        if (type != JettType::UNKNOWN && type != JettType::STR && !isArray(type))
            typeError(expression->arguments[0].get(), "length() expects a Str or array.");
        return JettType::INT;
    }

    if (name == "upper" || name == "lower" || name == "trim")
    {
        if (expression->arguments.size() != 1)
        {
            typeError(expression, name + "() expects exactly one argument.");
            return JettType::UNKNOWN;
        }
        JettType type = checkExpression(expression->arguments[0].get());
        if (type != JettType::UNKNOWN && type != JettType::STR)
            typeError(expression->arguments[0].get(), name + "() expects Str.");
        return JettType::STR;
    }

    if (name == "contains" || name == "startsWith" || name == "endsWith")
    {
        if (expression->arguments.size() != 2)
        {
            typeError(expression, name + "() expects two arguments.");
            return JettType::UNKNOWN;
        }
        JettType a = checkExpression(expression->arguments[0].get());
        JettType b = checkExpression(expression->arguments[1].get());
        if ((a != JettType::UNKNOWN && a != JettType::STR) ||
            (b != JettType::UNKNOWN && b != JettType::STR))
            typeError(expression, name + "() expects two Str arguments.");
        return JettType::BOOL;
    }

    if (name == "substring")
    {
        if (expression->arguments.size() != 3)
        {
            typeError(expression, "substring() expects three arguments.");
            return JettType::UNKNOWN;
        }
        JettType text = checkExpression(expression->arguments[0].get());
        JettType start = checkExpression(expression->arguments[1].get());
        JettType length = checkExpression(expression->arguments[2].get());
        if (text != JettType::UNKNOWN && text != JettType::STR) typeError(expression, "substring() first argument must be Str.");
        if (start != JettType::UNKNOWN && start != JettType::INT) typeError(expression, "substring() start must be Int.");
        if (length != JettType::UNKNOWN && length != JettType::INT) typeError(expression, "substring() length must be Int.");
        return JettType::STR;
    }

    auto function = functions.find(name);
    if (function == functions.end())
    {
        typeError(expression, "Function '" + name + "' was not declared.");
        return JettType::UNKNOWN;
    }

    const auto& info = function->second;
    if (expression->arguments.size() != info.parameterTypes.size())
    {
        typeError(expression, "Function '" + name + "' expects " +
            std::to_string(info.parameterTypes.size()) + " argument(s), but got " +
            std::to_string(expression->arguments.size()) + ".");
        return JettType::UNKNOWN;
    }

    for (std::size_t i = 0; i < expression->arguments.size(); ++i)
    {
        JettType actual = checkExpression(expression->arguments[i].get());
        JettType expected = info.parameterTypes[i];
        if (actual != JettType::UNKNOWN && expected != JettType::UNKNOWN && !isAssignable(expected, actual))
        {
            typeError(expression->arguments[i].get(), "Argument " + std::to_string(i + 1) +
                " of '" + name + "' expects " + typeToString(expected) +
                " but received " + typeToString(actual) + ".");
        }
    }

    return info.returnType;
}

JettType TypeChecker::typeFromName(const std::string& name) const
{
    if (name == "Int") return JettType::INT;
    if (name == "Fl") return JettType::FL;
    if (name == "Doub") return JettType::DOUB;
    if (name == "Str") return JettType::STR;
    if (name == "Bool") return JettType::BOOL;
    if (name == "Char") return JettType::CHAR;
    if (name == "Byte") return JettType::BYTE;
    if (name == "Long") return JettType::LONG;
    if (name == "Void") return JettType::VOID;
    if (name == "Int[]") return JettType::INT_ARRAY;
    if (name == "Fl[]") return JettType::FL_ARRAY;
    if (name == "Doub[]") return JettType::DOUB_ARRAY;
    if (name == "Str[]") return JettType::STR_ARRAY;
    if (name == "Bool[]") return JettType::BOOL_ARRAY;
    if (name == "Char[]") return JettType::CHAR_ARRAY;
    if (name == "Byte[]") return JettType::BYTE_ARRAY;
    if (name == "Long[]") return JettType::LONG_ARRAY;
    return JettType::UNKNOWN;
}

JettType TypeChecker::arrayElementType(JettType type) const
{
    switch (type)
    {
        case JettType::INT_ARRAY: return JettType::INT;
        case JettType::FL_ARRAY: return JettType::FL;
        case JettType::DOUB_ARRAY: return JettType::DOUB;
        case JettType::STR_ARRAY: return JettType::STR;
        case JettType::BOOL_ARRAY: return JettType::BOOL;
        case JettType::CHAR_ARRAY: return JettType::CHAR;
        case JettType::BYTE_ARRAY: return JettType::BYTE;
        case JettType::LONG_ARRAY: return JettType::LONG;
        default: return JettType::UNKNOWN;
    }
}

bool TypeChecker::isArray(JettType type) const
{
    return type == JettType::INT_ARRAY || type == JettType::FL_ARRAY ||
           type == JettType::DOUB_ARRAY || type == JettType::STR_ARRAY ||
           type == JettType::BOOL_ARRAY || type == JettType::CHAR_ARRAY ||
           type == JettType::BYTE_ARRAY || type == JettType::LONG_ARRAY;
}

bool TypeChecker::isNumeric(JettType type) const
{
    return type == JettType::INT || type == JettType::FL || type == JettType::DOUB ||
           type == JettType::BYTE || type == JettType::LONG;
}

bool TypeChecker::isAssignable(JettType expected, JettType actual) const
{
    if (expected == actual) return true;
    return isNumeric(expected) && isNumeric(actual);
}

std::string TypeChecker::typeToString(JettType type) const
{
    switch (type)
    {
        case JettType::INT: return "Int";
        case JettType::FL: return "Fl";
        case JettType::DOUB: return "Doub";
        case JettType::STR: return "Str";
        case JettType::BOOL: return "Bool";
        case JettType::CHAR: return "Char";
        case JettType::BYTE: return "Byte";
        case JettType::LONG: return "Long";
        case JettType::VOID: return "Void";
        case JettType::INT_ARRAY: return "Int[]";
        case JettType::FL_ARRAY: return "Fl[]";
        case JettType::DOUB_ARRAY: return "Doub[]";
        case JettType::STR_ARRAY: return "Str[]";
        case JettType::BOOL_ARRAY: return "Bool[]";
        case JettType::CHAR_ARRAY: return "Char[]";
        case JettType::BYTE_ARRAY: return "Byte[]";
        case JettType::LONG_ARRAY: return "Long[]";
        default: return "Unknown";
    }
}

void TypeChecker::typeError(const ASTNode* node, const std::string& message)
{
    if (node)
    {
        errors.push_back("Error at line " + std::to_string(node->line) +
            ", column " + std::to_string(node->column) + ": " + message);
    }
    else
    {
        errors.push_back("Error: " + message);
    }
}
