# Jett

![Jett Banner](assets/jett-banner.png)

**FAST • SIMPLE • POWERFUL**

Jett is a lightweight interpreted programming language built from scratch in C++.

## Copyright

Copyright © 2026 Shashwat Ghadge. All rights reserved.

The Jett source code, name, logos, artwork, and branding are proprietary unless a separate license explicitly says otherwise. See `LICENSE` for the project license terms.

## Branding Assets

The `assests/` directory contains Jett's official branding assets.

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
- `if`, `else`, `else if`
- `while` and `for` loops
- `break` and `continue`
- Functions, parameters, and `return`
- Arrays and indexing
- `print()` and `input()`
- String utilities
- `--help` and `--version`
- `.jett` source files

## Quick Start

### Build

```powershell
msbuild .\\build\\jett.vcxproj /p:Configuration=Debug
```

### Run

```powershell
.\\build\\Debug\\jett.exe .\\examples\\hello.jett
```

### CLI

```powershell
.\\build\\Debug\\jett.exe --version
.\\build\\Debug\\jett.exe --help
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

## Version

**Jett 1.0.0**

## License

Jett is proprietary software. Copyright © 2026 Shashwat Ghadge. All rights reserved. See `LICENSE` and `COPYRIGHT.md`.

---

**Jett**  
`FAST • SIMPLE • POWERFUL`
