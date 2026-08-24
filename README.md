# Jett

![Jett Banner](assets/jett-banner.png)

**FAST • SIMPLE • POWERFUL**

Jett is a lightweight interpreted programming language built from scratch in C++.

> [!WARNING]
> **Project status: Incomplete / Experimental**
>
> Jett is an ongoing, unfinished project and should not be considered a production-ready programming language. This project was primarily an attempt to explore what it takes to design and build a programming language in a professional, structured way, from the lexer and parser through type checking, interpretation, testing, documentation, and tooling.
>
> Some features are implemented and functional, while other parts are still experimental or incomplete. The repository is kept as a record of the project, its architecture, experiments, and continued learning.

## What's new in 1.1

Jett now includes a richer command-line workflow for inspecting and validating programs before execution.

```text
jett <file.jett>            Run a program
jett --check <file.jett>    Type-check without executing
jett --tokens <file.jett>   Inspect lexer output
jett --ast <file.jett>      Inspect the parsed AST
jett --help                 Show CLI help
jett --version              Show version
```

## Project Goals

The goal of Jett was not simply to make another toy interpreter. It was an attempt to approach language development as a real software-engineering project, including:

- A structured compiler/interpreter pipeline
- A dedicated lexer and parser
- An Abstract Syntax Tree (AST)
- Static type checking
- A tree-walk interpreter
- Automated testing
- Cross-platform CMake builds
- GitHub Actions CI
- Documentation and architecture guides
- VS Code language tooling
- A dedicated syntax and file format

The project is incomplete, but it represents an exploration of how a programming language can be designed, implemented, documented, tested, and maintained in a professional way.

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
- CLI inspection and validation modes
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

### Validate without running

```powershell
.\\build\\Release\\jett.exe --check .\\examples\\functions.jett
```

### Inspect tokens

```powershell
.\\build\\Release\\jett.exe --tokens .\\examples\\hello.jett
```

### Inspect the AST

```powershell
.\\build\\Release\\jett.exe --ast .\\examples\\functions.jett
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

**Jett 1.1.0**

## License

Jett is proprietary software. Copyright © 2026 Shashwat Ghadge. All rights reserved. See `LICENSE` and `COPYRIGHT.md`.

---

**Jett**  
`FAST • SIMPLE • POWERFUL`
