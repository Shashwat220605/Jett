# Jett Core Regression Tests

These `.jett` files form the language-level regression suite.

## Valid programs

- `core_scope_types.jett`
- `operators.jett`
- `logical.jett`
- `control_flow.jett`
- `functions_recursion.jett`
- `strings_chars.jett`

## Expected failures

- `invalid_type.jett`
- `invalid_assignment.jett`
- `invalid_function_args.jett`
- `invalid_syntax_extra.jett`
- `invalid_array_index.jett`
- `invalid_division.jett`
- `invalid_break.jett`
- `invalid_continue.jett`

Expected-failure programs are intentionally invalid. A future test harness should assert both the non-zero exit status and the diagnostic category/message.

## Core implementation contract

A change to the lexer, parser, AST, type checker, or interpreter should run this suite. If a bug is discovered, add a minimal `.jett` reproducer here before fixing it so the bug cannot silently return later.
