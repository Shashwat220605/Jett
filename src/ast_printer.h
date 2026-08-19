#pragma once

#include "ast.h"

class ASTPrinter
{
public:
    static void print(const Program& program);

private:
    static void printStatement(
        const Statement* statement,
        int indent
    );

    static void printExpression(
        const Expression* expression,
        int indent
    );

    static void printIndent(int indent);
};