# Jett

![Jett Banner](assets/jett-banner.png)

**FAST • SIMPLE • POWERFUL**

Jett is a lightweight interpreted programming language built from scratch in C++.

## Copyright

Copyright © 2026 Shashwat Ghadge. All rights reserved.

The Jett source code, name, logos, artwork, and branding are proprietary unless a separate license explicitly says otherwise. See `LICENSE` for the project license terms.

## Branding Assets

The `assets/` directory contains Jett's official branding assets.

- `jett-logo.png` - main logo
- `jett-wordmark.png` - wordmark
- `jett-banner.png` - README/GitHub banner
- `tagline.png` - brand tagline
- `jett-mark.png` - compact Jett mark
- `jett-jett.png` - `.jett` / VS Code file icon

## Features

- Lexer and recursive-descent parser
- Abstract Syntax Tree (AST)
- Static type checking
- Tree-walk interpreter
- Variables and assignments
- Arithmetic, comparisons, and boolean logic
- Bitwise operators and shifts
- `if`, `else`, `else if`
- `while` and `for` loops
- `break` and `continue`
- Functions, parameters, and `return`
- Arrays and indexing
- `print()` and `input()`
- String utilities
- `--help` and `--version`
- `.jett` source files
- VS Code language support with syntax highlighting and Jett file icons

## Quick Start

### Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

### Run

```powershell
.\\build\\Release\\jett.exe .\\examples\\hello.jett
```

### CLI

```powershell
.\\build\\Release\\jett.exe --version
.\\build\\Release\\jett.exe --help
```

## Hello Jett

```jett
print("Hello Jett");
```

## Variables

```jett
Int x is 10;
Int y is 5;

print(x + y);
```

## Conditions

```jett
Int x is 10;
Int y is 5;

if x > y {
    print("x is greater");
} else {
    print("y is greater");
}
```

## Loops

```jett
Int i is 0;

while i < 5 {
    print(i);
    i = i + 1;
}
```

## Functions

```jett
fn add(Int a, Int b) {
    return a + b;
}

print(add(10, 20));
```

## Arrays

```jett
Int[] numbers is [10, 20, 30];

print(numbers[0]);
numbers[1] = 99;
print(numbers[1]);
```

## Input

```jett
Str name is input("Enter your name: ");
print("Hello " + name);
```

## Project Architecture

Jett processes source code through a clear pipeline:

```text
.jett source
    ↓
Lexer → Tokens
    ↓
Parser → AST
    ↓
Type Checker
    ↓
Interpreter
    ↓
Program output
```

The C++ implementation lives in `src/`. Examples are in `examples/`, validation programs are in `tests/`, and VS Code support is in `vscode-jett/`.

For a detailed explanation of every repository file and how the components interact, see `docs/Jett_Architecture_Guide.pdf`.

## Testing

Jett uses CTest for automated validation. After building:

```powershell
ctest --test-dir build --output-on-failure -C Release
```

The GitHub Actions workflow also builds Jett on Ubuntu and Windows and runs the test suite.

## Version

**Jett 1.0.0**

## License

Jett is proprietary software. Copyright © 2026 Shashwat Ghadge. All rights reserved. See `LICENSE` and `COPYRIGHT.md`.

---

**Jett**  
`FAST • SIMPLE • POWERFUL`
