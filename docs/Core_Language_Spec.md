# Jett Core Language Specification

## Purpose

This document defines the correctness rules for the Jett core language. It is the reference for the lexer, parser, type checker, interpreter, examples, and regression tests.

## 1. Primitive Types

Jett recognizes these primitive types:

- `Int`
- `Fl`
- `Doub`
- `Str`
- `Bool`
- `Char`
- `Byte`
- `Long`
- `Void`

Arrays use the corresponding `Type[]` form.

## 2. Declaration

Explicit declaration:

```jett
Int count is 10;
Str name is "Jett";
Bool enabled is true;
```

Type-inferred declaration:

```jett
count is 10;
name is "Jett";
```

A declared variable must receive a value compatible with its declared type.

## 3. Assignment

Assignment is an expression and must preserve the declared variable type.

```jett
Int x is 10;
x = 20;
```

This is invalid:

```jett
Int x is 10;
x = "twenty";
```

Array element assignment follows the array element type.

## 4. Expressions

Operator precedence, from lowest to highest:

1. assignment
2. logical OR `||`
3. logical AND `&&`
4. equality `==`, `!=`
5. comparison `<`, `<=`, `>`, `>=`
6. bitwise OR `|`
7. bitwise XOR `^`
8. bitwise AND `&`
9. shifts `<<`, `>>`
10. addition/subtraction `+`, `-`
11. multiplication/division/modulo `*`, `/`, `%`
12. unary `!`, unary `+`, unary `-`, and future prefix operators
13. calls and indexing

The parser must preserve this ordering so expressions such as `a + b * c` evaluate as `a + (b * c)`.

## 5. Boolean Logic

`&&`, `||`, and `!` operate on `Bool` values. Conditions in `if`, `while`, and `for` must evaluate to `Bool`.

## 6. Numeric Operators

Arithmetic operators require numeric operands. Numeric result promotion follows:

`Doub` > `Fl` > `Long` > `Int`

A numeric operation must not silently accept `Str`, `Bool`, arrays, or `Void`.

## 7. Arrays

Array literals must contain compatible element types.

```jett
Int[] numbers is [10, 20, 30];
print(numbers[0]);
numbers[1] = 99;
```

Indexes must be `Int`. Runtime indexes must be checked for bounds before access.

## 8. Functions

Functions have typed parameters:

```jett
fn add(Int a, Int b) {
    return a + b;
}
```

Calls must provide exactly the declared number of arguments, and every argument must be compatible with its parameter type.

Functions may call themselves recursively.

`return` is valid only inside a function.

## 9. Control Flow

`break` and `continue` are valid only inside loops.

```jett
while condition {
    if something {
        continue;
    }

    if done {
        break;
    }
}
```

The implementation must not allow these statements to escape their loop context.

## 10. Runtime Errors

The interpreter must report controlled Jett errors for:

- division or modulo by zero
- undefined variables
- invalid function calls
- wrong argument count
- invalid array indexes
- indexing non-arrays
- assigning incompatible values
- unsupported operators
- invalid control-flow statements

Errors should identify the relevant source location whenever the AST node provides one.

## 11. Scope

Blocks and functions must not accidentally leak implementation state into unrelated execution contexts. Function parameters and local variables belong to the function invocation. Nested block scope should be treated consistently across declarations, assignments, type checking, and interpretation.

## 12. Regression Policy

Every language bug should receive a `.jett` regression test before or together with the fix. The `tests/` directory contains both valid programs and intentionally invalid programs.

Core tests currently cover:

- declarations and arrays
- operators
- logical expressions
- control flow
- recursion
- strings and characters
- invalid assignments
- invalid types
- invalid syntax
- invalid function arguments
- invalid array indexes
- division by zero
- invalid `break` / `continue`

## 13. Implementation Pipeline

```text
Source
  -> Lexer
  -> Tokens
  -> Recursive-descent Parser
  -> AST
  -> Type Checker
  -> Tree-walk Interpreter
  -> Output / Jett Error
```

Any new core-language feature must be considered across all affected stages rather than implemented in only one stage.

Copyright © 2026 Shashwat Ghadge. All rights reserved.
