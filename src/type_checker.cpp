#include "type_checker.h"

#include <utility>

void TypeChecker::check(const Program& program)
{
    variables.clear();
    functions.clear();
    errors.clear();
    currentFunction = nullptr;
    checkingFunction = false;

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
                typeError(function, "Unknown parameter type '" + parameter.type + "'.");
            info.parameterTypes.push_back(type);
        }
        functions.emplace(function->name, info);
    }

    for (const auto& statement : program.statements)
    {
        auto function = dynamic_cast<const FunctionStmt*>(statement.get());
        if (function) checkFunctionStatement(function);
    }

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
            variables[function->parameters[i].name] = functions[function->name].parameterTypes[i];

        for (const auto& child : function->body) checkStatement(child.get());

        variables = std::move(previousVariables);
        currentFunction = previousFunction;
        inferredReturnType = previousReturn;
        checkingFunction = previousChecking;
    }

    for (const auto& statement : program.statements)
        if (!dynamic_cast<const FunctionStmt*>(statement.get())) checkStatement(statement.get());
}

bool TypeChecker::hasErrors() const { return !errors.empty(); }
const std::vector<std::string>& TypeChecker::getErrors() const { return errors; }

void TypeChecker::checkStatement(const Statement* statement)
{
    if (!statement) { typeError(nullptr, "Invalid statement."); return; }
    if (auto s = dynamic_cast<const VariableDeclarationStmt*>(statement)) return checkVariableDeclaration(s);
    if (auto s = dynamic_cast<const ExpressionStmt*>(statement)) return checkExpressionStatement(s);
    if (auto s = dynamic_cast<const IfStmt*>(statement)) return checkIfStatement(s);
    if (auto s = dynamic_cast<const WhileStmt*>(statement)) return checkWhileStatement(s);
    if (auto s = dynamic_cast<const ForStmt*>(statement)) return checkForStatement(s);
    if (auto s = dynamic_cast<const FunctionStmt*>(statement)) return checkFunctionStatement(s);
    if (auto s = dynamic_cast<const ReturnStmt*>(statement)) return checkReturnStatement(s);
    if (auto s = dynamic_cast<const BreakStmt*>(statement)) return checkBreakStatement(s);
    if (auto s = dynamic_cast<const ContinueStmt*>(statement)) return checkContinueStatement(s);
    typeError(statement, "Unknown statement.");
}

void TypeChecker::checkIfStatement(const IfStmt* s)
{
    JettType t = checkExpression(s->condition.get());
    if (t != JettType::UNKNOWN && t != JettType::BOOL) typeError(s->condition.get(), "If condition must be Bool, but received " + typeToString(t) + ".");
    for (const auto& child : s->thenBranch) checkStatement(child.get());
    if (s->hasElse) for (const auto& child : s->elseBranch) checkStatement(child.get());
}

void TypeChecker::checkWhileStatement(const WhileStmt* s)
{
    JettType t = checkExpression(s->condition.get());
    if (t != JettType::UNKNOWN && t != JettType::BOOL) typeError(s->condition.get(), "While condition must be Bool, but received " + typeToString(t) + ".");
    for (const auto& child : s->body) checkStatement(child.get());
}

void TypeChecker::checkForStatement(const ForStmt* s)
{
    if (s->initializer) checkStatement(s->initializer.get());
    if (s->condition)
    {
        JettType t = checkExpression(s->condition.get());
        if (t != JettType::UNKNOWN && t != JettType::BOOL) typeError(s->condition.get(), "For condition must be Bool, but received " + typeToString(t) + ".");
    }
    for (const auto& child : s->body) checkStatement(child.get());
    if (s->increment) checkExpression(s->increment.get());
}

void TypeChecker::checkBreakStatement(const BreakStmt*) {}
void TypeChecker::checkContinueStatement(const ContinueStmt*) {}

void TypeChecker::checkFunctionStatement(const FunctionStmt* s)
{
    auto found = functions.find(s->name);
    if (found == functions.end()) return;

    auto previousVariables = std::move(variables);
    const FunctionStmt* previousFunction = currentFunction;
    JettType previousReturn = inferredReturnType;
    bool previousChecking = checkingFunction;

    variables.clear();
    currentFunction = s;
    checkingFunction = true;
    inferredReturnType = found->second.returnType;

    for (std::size_t i = 0; i < s->parameters.size(); ++i)
    {
        const auto& parameter = s->parameters[i];
        if (variables.find(parameter.name) != variables.end())
            typeError(s, "Parameter '" + parameter.name + "' has already been declared.");
        else
            variables[parameter.name] = found->second.parameterTypes[i];
    }

    for (const auto& child : s->body) checkStatement(child.get());

    if (inferredReturnType == JettType::UNKNOWN)
        inferredReturnType = JettType::VOID;
    found->second.returnType = inferredReturnType;

    variables = std::move(previousVariables);
    currentFunction = previousFunction;
    inferredReturnType = previousReturn;
    checkingFunction = previousChecking;
}

void TypeChecker::checkReturnStatement(const ReturnStmt* s)
{
    if (!checkingFunction || !currentFunction)
    {
        typeError(s, "'return' can only be used inside a function.");
        return;
    }

    JettType type = s->value ? checkExpression(s->value.get()) : JettType::VOID;
    if (type == JettType::UNKNOWN) return;

    if (inferredReturnType == JettType::UNKNOWN)
    {
        inferredReturnType = type;
        return;
    }

    if (!isAssignable(inferredReturnType, type))
        typeError(s, "Function '" + currentFunction->name + "' returns both " + typeToString(inferredReturnType) + " and " + typeToString(type) + ".");
}

void TypeChecker::checkVariableDeclaration(const VariableDeclarationStmt* s)
{
    if (variables.find(s->name) != variables.end())
    {
        typeError(s, "Variable '" + s->name + "' has already been declared.");
        return;
    }
    JettType actual = checkExpression(s->initializer.get());
    if (actual == JettType::UNKNOWN) return;
    if (s->declaredType.empty()) { variables[s->name] = actual; return; }
    JettType expected = typeFromName(s->declaredType);
    if (expected == JettType::UNKNOWN) { typeError(s, "Unknown type '" + s->declaredType + "'."); return; }
    if (!isAssignable(expected, actual))
    {
        typeError(s, "'" + s->name + "' expects " + typeToString(expected) + " but received " + typeToString(actual) + ".");
        return;
    }
    variables[s->name] = expected;
}

void TypeChecker::checkExpressionStatement(const ExpressionStmt* s) { checkExpression(s->expression.get()); }

JettType TypeChecker::checkExpression(const Expression* e)
{
    if (!e) { typeError(nullptr, "Invalid expression."); return JettType::UNKNOWN; }
    if (auto x = dynamic_cast<const LiteralExpr*>(e)) return checkLiteral(x);
    if (auto x = dynamic_cast<const ArrayLiteralExpr*>(e)) return checkArrayLiteral(x);
    if (auto x = dynamic_cast<const VariableExpr*>(e)) return checkVariable(x);
    if (auto x = dynamic_cast<const AssignmentExpr*>(e))
    {
        JettType value = checkExpression(x->value.get());
        auto it = variables.find(x->name);
        if (it == variables.end()) { typeError(x, "Variable '" + x->name + "' was not declared."); return JettType::UNKNOWN; }
        if (value != JettType::UNKNOWN && !isAssignable(it->second, value))
        { typeError(x, "Cannot assign " + typeToString(value) + " to " + typeToString(it->second) + "."); return JettType::UNKNOWN; }
        return it->second;
    }
    if (auto x = dynamic_cast<const IndexExpr*>(e)) return checkIndex(x);
    if (auto x = dynamic_cast<const IndexAssignmentExpr*>(e)) return checkIndexAssignment(x);
    if (auto x = dynamic_cast<const BinaryExpr*>(e)) return checkBinary(x);
    if (auto x = dynamic_cast<const UnaryExpr*>(e)) return checkUnary(x);
    if (auto x = dynamic_cast<const CallExpr*>(e)) return checkCall(x);
    typeError(e, "Unknown expression.");
    return JettType::UNKNOWN;
}

JettType TypeChecker::checkLiteral(const LiteralExpr* e)
{
    if (std::holds_alternative<long long>(e->value)) return JettType::INT;
    if (std::holds_alternative<double>(e->value)) return JettType::DOUB;
    if (std::holds_alternative<std::string>(e->value)) return JettType::STR;
    if (std::holds_alternative<bool>(e->value)) return JettType::BOOL;
    if (std::holds_alternative<char>(e->value)) return JettType::CHAR;
    return JettType::UNKNOWN;
}

JettType TypeChecker::checkArrayLiteral(const ArrayLiteralExpr* e)
{
    if (e->elements.empty()) { typeError(e, "Empty arrays are not supported yet."); return JettType::UNKNOWN; }
    JettType element = JettType::UNKNOWN;
    for (const auto& value : e->elements)
    {
        JettType current = checkExpression(value.get());
        if (current == JettType::UNKNOWN) return JettType::UNKNOWN;
        if (element == JettType::UNKNOWN) element = current;
        else if (element != current && !(isNumeric(element) && isNumeric(current)))
        { typeError(value.get(), "All array elements must have the same type."); return JettType::UNKNOWN; }
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

JettType TypeChecker::checkVariable(const VariableExpr* e)
{
    auto it = variables.find(e->name);
    if (it == variables.end()) { typeError(e, "Variable '" + e->name + "' was not declared."); return JettType::UNKNOWN; }
    return it->second;
}

JettType TypeChecker::checkIndex(const IndexExpr* e)
{
    JettType array = checkExpression(e->array.get());
    JettType index = checkExpression(e->index.get());
    if (index != JettType::UNKNOWN && index != JettType::INT) { typeError(e->index.get(), "Array index must be Int."); return JettType::UNKNOWN; }
    if (array == JettType::UNKNOWN) return JettType::UNKNOWN;
    if (!isArray(array)) { typeError(e->array.get(), "Only arrays can be indexed."); return JettType::UNKNOWN; }
    return arrayElementType(array);
}

JettType TypeChecker::checkIndexAssignment(const IndexAssignmentExpr* e)
{
    auto index = dynamic_cast<const IndexExpr*>(e->target.get());
    if (!index) { typeError(e, "Invalid array assignment target."); return JettType::UNKNOWN; }
    JettType array = checkExpression(index->array.get());
    JettType indexType = checkExpression(index->index.get());
    JettType value = checkExpression(e->value.get());
    if (indexType != JettType::UNKNOWN && indexType != JettType::INT) { typeError(index->index.get(), "Array index must be Int."); return JettType::UNKNOWN; }
    if (!isArray(array)) { typeError(index->array.get(), "Only arrays can be indexed."); return JettType::UNKNOWN; }
    JettType element = arrayElementType(array);
    if (value != JettType::UNKNOWN && !isAssignable(element, value)) { typeError(e, "Cannot assign " + typeToString(value) + " to array element of type " + typeToString(element) + "."); return JettType::UNKNOWN; }
    return element;
}

JettType TypeChecker::checkBinary(const BinaryExpr* e)
{
    JettType left = checkExpression(e->left.get());
    JettType right = checkExpression(e->right.get());
    if (left == JettType::UNKNOWN || right == JettType::UNKNOWN) return JettType::UNKNOWN;
    switch (e->op)
    {
        case TokenType::PLUS:
            if (left == JettType::STR && right == JettType::STR) return JettType::STR;
            [[fallthrough]];
        case TokenType::MINUS: case TokenType::STAR: case TokenType::SLASH: case TokenType::PERCENT:
            if (isNumeric(left) && isNumeric(right))
            {
                if (left == JettType::DOUB || right == JettType::DOUB) return JettType::DOUB;
                if (left == JettType::FL || right == JettType::FL) return JettType::FL;
                if (left == JettType::LONG || right == JettType::LONG) return JettType::LONG;
                return JettType::INT;
            }
            typeError(e, "Arithmetic operator cannot be used with " + typeToString(left) + " and " + typeToString(right) + "."); return JettType::UNKNOWN;
        case TokenType::GREATER: case TokenType::GREATER_EQUAL: case TokenType::LESS: case TokenType::LESS_EQUAL:
            if (isNumeric(left) && isNumeric(right)) return JettType::BOOL;
            typeError(e, "Comparison requires numeric values."); return JettType::UNKNOWN;
        case TokenType::EQUAL_EQUAL: case TokenType::NOT_EQUAL:
            if (left == right || (isNumeric(left) && isNumeric(right))) return JettType::BOOL;
            typeError(e, "Cannot compare " + typeToString(left) + " with " + typeToString(right) + "."); return JettType::UNKNOWN;
        case TokenType::AND: case TokenType::OR:
            if (left == JettType::BOOL && right == JettType::BOOL) return JettType::BOOL;
            typeError(e, "Logical operators require Bool values."); return JettType::UNKNOWN;
        default:
            typeError(e, "Unsupported binary operator."); return JettType::UNKNOWN;
    }
}

JettType TypeChecker::checkUnary(const UnaryExpr* e)
{
    JettType type = checkExpression(e->right.get());
    if (type == JettType::UNKNOWN) return type;
    if ((e->op == TokenType::PLUS || e->op == TokenType::MINUS) && isNumeric(type)) return type;
    if (e->op == TokenType::NOT && type == JettType::BOOL) return JettType::BOOL;
    typeError(e, "Invalid unary operator for type " + typeToString(type) + ".");
    return JettType::UNKNOWN;
}

JettType TypeChecker::checkCall(const CallExpr* e)
{
    auto variable = dynamic_cast<const VariableExpr*>(e->callee.get());
    if (!variable) { typeError(e, "Invalid function call."); return JettType::UNKNOWN; }
    const std::string& name = variable->name;

    if (name == "print")
    {
        for (const auto& argument : e->arguments) checkExpression(argument.get());
        return JettType::VOID;
    }
    if (name == "input")
    {
        if (e->arguments.size() > 1) { typeError(e, "input() accepts zero or one argument."); return JettType::UNKNOWN; }
        if (!e->arguments.empty()) { JettType t = checkExpression(e->arguments[0].get()); if (t != JettType::UNKNOWN && t != JettType::STR) typeError(e, "input() prompt must be Str."); }
        return JettType::STR;
    }
    if (name == "length")
    {
        if (e->arguments.size() != 1) { typeError(e, "length() expects exactly one argument."); return JettType::UNKNOWN; }
        JettType t = checkExpression(e->arguments[0].get());
        if (t != JettType::UNKNOWN && t != JettType::STR && !isArray(t)) typeError(e, "length() expects a Str or array.");
        return JettType::INT;
    }
    if (name == "upper" || name == "lower" || name == "trim")
    {
        if (e->arguments.size() != 1) { typeError(e, name + "() expects exactly one argument."); return JettType::UNKNOWN; }
        JettType t = checkExpression(e->arguments[0].get());
        if (t != JettType::UNKNOWN && t != JettType::STR) typeError(e, name + "() expects Str.");
        return JettType::STR;
    }
    if (name == "contains" || name == "startsWith" || name == "endsWith")
    {
        if (e->arguments.size() != 2) { typeError(e, name + "() expects two arguments."); return JettType::UNKNOWN; }
        JettType a = checkExpression(e->arguments[0].get());
        JettType b = checkExpression(e->arguments[1].get());
        if ((a != JettType::UNKNOWN && a != JettType::STR) || (b != JettType::UNKNOWN && b != JettType::STR)) typeError(e, name + "() expects two Str arguments.");
        return JettType::BOOL;
    }
    if (name == "substring")
    {
        if (e->arguments.size() != 3) { typeError(e, "substring() expects three arguments."); return JettType::UNKNOWN; }
        JettType text = checkExpression(e->arguments[0].get());
        JettType start = checkExpression(e->arguments[1].get());
        JettType length = checkExpression(e->arguments[2].get());
        if (text != JettType::UNKNOWN && text != JettType::STR) typeError(e, "substring() first argument must be Str.");
        if (start != JettType::UNKNOWN && start != JettType::INT) typeError(e, "substring() start must be Int.");
        if (length != JettType::UNKNOWN && length != JettType::INT) typeError(e, "substring() length must be Int.");
        return JettType::STR;
    }

    auto function = functions.find(name);
    if (function == functions.end()) { typeError(e, "Function '" + name + "' was not declared."); return JettType::UNKNOWN; }
    const auto& info = function->second;
    if (e->arguments.size() != info.parameterTypes.size())
    {
        typeError(e, "Function '" + name + "' expects " + std::to_string(info.parameterTypes.size()) + " argument(s), but got " + std::to_string(e->arguments.size()) + ".");
        return JettType::UNKNOWN;
    }
    for (std::size_t i = 0; i < e->arguments.size(); ++i)
    {
        JettType actual = checkExpression(e->arguments[i].get());
        JettType expected = info.parameterTypes[i];
        if (actual != JettType::UNKNOWN && expected != JettType::UNKNOWN && !isAssignable(expected, actual))
            typeError(e->arguments[i].get(), "Argument " + std::to_string(i + 1) + " of '" + name + "' expects " + typeToString(expected) + " but received " + typeToString(actual) + ".");
    }
    return info.returnType;
}

JettType TypeChecker::typeFromName(const std::string& name) const
{
    if (name == "Int") return JettType::INT; if (name == "Fl") return JettType::FL; if (name == "Doub") return JettType::DOUB;
    if (name == "Str") return JettType::STR; if (name == "Bool") return JettType::BOOL; if (name == "Char") return JettType::CHAR;
    if (name == "Byte") return JettType::BYTE; if (name == "Long") return JettType::LONG; if (name == "Void") return JettType::VOID;
    if (name == "Int[]") return JettType::INT_ARRAY; if (name == "Fl[]") return JettType::FL_ARRAY; if (name == "Doub[]") return JettType::DOUB_ARRAY;
    if (name == "Str[]") return JettType::STR_ARRAY; if (name == "Bool[]") return JettType::BOOL_ARRAY; if (name == "Char[]") return JettType::CHAR_ARRAY;
    if (name == "Byte[]") return JettType::BYTE_ARRAY; if (name == "Long[]") return JettType::LONG_ARRAY;
    return JettType::UNKNOWN;
}

JettType TypeChecker::arrayElementType(JettType type) const
{
    switch (type)
    {
        case JettType::INT_ARRAY: return JettType::INT; case JettType::FL_ARRAY: return JettType::FL; case JettType::DOUB_ARRAY: return JettType::DOUB;
        case JettType::STR_ARRAY: return JettType::STR; case JettType::BOOL_ARRAY: return JettType::BOOL; case JettType::CHAR_ARRAY: return JettType::CHAR;
        case JettType::BYTE_ARRAY: return JettType::BYTE; case JettType::LONG_ARRAY: return JettType::LONG; default: return JettType::UNKNOWN;
    }
}

bool TypeChecker::isArray(JettType type) const
{
    return type == JettType::INT_ARRAY || type == JettType::FL_ARRAY || type == JettType::DOUB_ARRAY || type == JettType::STR_ARRAY ||
           type == JettType::BOOL_ARRAY || type == JettType::CHAR_ARRAY || type == JettType::BYTE_ARRAY || type == JettType::LONG_ARRAY;
}

bool TypeChecker::isNumeric(JettType type) const
{
    return type == JettType::INT || type == JettType::FL || type == JettType::DOUB || type == JettType::BYTE || type == JettType::LONG;
}

bool TypeChecker::isAssignable(JettType expected, JettType actual) const
{
    return expected == actual || (isNumeric(expected) && isNumeric(actual));
}

std::string TypeChecker::typeToString(JettType type) const
{
    switch (type)
    {
        case JettType::INT: return "Int"; case JettType::FL: return "Fl"; case JettType::DOUB: return "Doub"; case JettType::STR: return "Str";
        case JettType::BOOL: return "Bool"; case JettType::CHAR: return "Char"; case JettType::BYTE: return "Byte"; case JettType::LONG: return "Long";
        case JettType::VOID: return "Void"; case JettType::INT_ARRAY: return "Int[]"; case JettType::FL_ARRAY: return "Fl[]"; case JettType::DOUB_ARRAY: return "Doub[]";
        case JettType::STR_ARRAY: return "Str[]"; case JettType::BOOL_ARRAY: return "Bool[]"; case JettType::CHAR_ARRAY: return "Char[]"; case JettType::BYTE_ARRAY: return "Byte[]"; case JettType::LONG_ARRAY: return "Long[]";
        default: return "Unknown";
    }
}

void TypeChecker::typeError(const ASTNode* node, const std::string& message)
{
    if (node) errors.push_back("Error at line " + std::to_string(node->line) + ", column " + std::to_string(node->column) + ": " + message);
    else errors.push_back("Error: " + message);
}
