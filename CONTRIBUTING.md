# Contributing to Mana

Thank you for your interest in contributing to Mana! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Documentation](#documentation)

## Code of Conduct

Please read our [Code of Conduct](CODE_OF_CONDUCT.md) before contributing. We are committed to providing a welcoming and inclusive environment for everyone.

## Getting Started

### Finding Issues to Work On

- Look for issues labeled `good first issue` for beginner-friendly tasks
- Issues labeled `help wanted` are open for community contributions
- Check the [roadmap](docs/ROADMAP.md) for planned features
- Feel free to propose new features by opening an issue first

### Reporting Bugs

When reporting bugs, please include:

1. **Mana version** (`mana --version`)
2. **Operating system** and version
3. **Minimal reproduction** - smallest code example that triggers the bug
4. **Expected behavior** vs **actual behavior**
5. **Full error message** including stack traces if applicable

### Feature Requests

Open an issue with the `enhancement` label and include:

1. **Problem description** - what problem does this solve?
2. **Proposed solution** - how should it work?
3. **Alternatives considered** - other approaches you thought of
4. **Additional context** - examples from other languages, use cases

## Development Setup

### Prerequisites

- **CMake** 3.16 or later
- **C++17 compiler**:
  - Windows: Visual Studio 2022 or MinGW-w64
  - macOS: Xcode 14+ or Clang 14+
  - Linux: GCC 10+ or Clang 14+
- **Git**

### Building from Source

```bash
# Clone the repository
git clone https://github.com/mana-lang/mana.git
cd mana

# Windows
scripts\build.bat release

# macOS / Linux
./scripts/build.sh release
```

### Build Targets

```bash
# Build configurations
scripts\build.bat debug      # Debug build with symbols
scripts\build.bat release    # Optimized release build
scripts\build.bat test       # Build and run tests
scripts\build.bat examples   # Build example programs
scripts\build.bat all        # Build everything
scripts\build.bat clean      # Clean build artifacts
```

### Running Tests

```bash
# Build and run all tests
scripts\build.bat test

# Run tests directly
build\Release\mana_tests.exe

# Run specific test
build\Release\mana_tests.exe --gtest_filter=ParserTest.*
```

## Making Changes

### Branch Naming

Use descriptive branch names:

- `feature/description` - new features
- `fix/description` - bug fixes
- `docs/description` - documentation changes
- `refactor/description` - code refactoring
- `test/description` - test additions/changes

### Commit Messages

Follow conventional commit format:

```
type(scope): brief description

Longer description if needed.

Fixes #123
```

Types:
- `feat` - new feature
- `fix` - bug fix
- `docs` - documentation
- `refactor` - code refactoring
- `test` - adding/updating tests
- `chore` - build, CI, dependencies

Examples:
```
feat(parser): add support for match expressions
fix(semantic): resolve type inference for generics
docs(tutorial): add chapter on error handling
refactor(lexer): simplify token classification
test(codegen): add tests for defer statement
```

## Pull Request Process

1. **Fork** the repository and create your branch from `main`
2. **Make changes** following our coding standards
3. **Add tests** for new functionality
4. **Update documentation** if needed
5. **Run tests** to ensure nothing is broken
6. **Open PR** with a clear description

### PR Description Template

```markdown
## Summary
Brief description of changes.

## Changes
- List of specific changes made

## Testing
How were these changes tested?

## Related Issues
Fixes #123
```

### Review Process

1. PRs require at least one approving review
2. CI must pass (build, tests, linting)
3. Address reviewer feedback promptly
4. Squash commits if requested

## Coding Standards

### C++ Style Guide

We follow a consistent C++ style:

```cpp
namespace mana::frontend {

    // Use 4-space indentation
    class Parser {
    public:
        // Public interface first
        explicit Parser(Lexer& lexer);
        std::unique_ptr<AstModule> parse();

    private:
        // Private implementation
        Lexer& lexer_;
        Token current_token_;

        void advance();
        bool expect(TokenKind kind);
    };

    // Function definitions
    void Parser::advance() {
        current_token_ = lexer_.next_token();
    }

} // namespace mana::frontend
```

### Key Conventions

- **Naming**:
  - Classes: `PascalCase`
  - Functions: `snake_case`
  - Variables: `snake_case`
  - Constants: `UPPER_CASE`
  - Private members: trailing underscore (`member_`)

- **Headers**:
  - Use `#pragma once`
  - Include what you use
  - Forward declare when possible

- **Modern C++**:
  - Prefer `unique_ptr` over raw pointers
  - Use `auto` when type is obvious
  - Use `const` liberally
  - Prefer range-based for loops

### AST Modifications

When adding new AST nodes, update ALL of these files:

1. `AstNodes.h` - Add to `NodeKind` enum
2. `AstStatements.h` / `AstExpressions.h` - Add node struct
3. `Parser.cpp` - Add parsing logic
4. `AstPrinter.cpp` - Add debug printing
5. `Semantic.cpp` - Add type checking
6. `CppEmitter.cpp` - Add code generation
7. Update IR passes in `middle/` if applicable

## Testing

### Test Categories

- **Unit tests** - Test individual components
- **Integration tests** - Test compiler pipeline
- **E2E tests** - Compile and run Mana programs

### Writing Tests

```cpp
#include <gtest/gtest.h>
#include "Parser.h"

TEST(ParserTest, ParseSimpleFunction) {
    std::string source = R"(
        fn add(a: i32, b: i32) -> i32 {
            return a + b;
        }
    )";

    Lexer lexer(source);
    Parser parser(lexer);
    auto ast = parser.parse();

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->decls.size(), 1);
}
```

### Test File Organization

```
tests/
  mana_tests/
    lexer_tests.cpp
    parser_tests.cpp
    semantic_tests.cpp
    codegen_tests.cpp
  e2e/
    run_examples.cpp
```

## Documentation

### Code Documentation

Use `///` for documentation comments:

```cpp
/// Parses a function declaration.
///
/// Grammar:
///   fn IDENTIFIER ( params ) -> TYPE { body }
///
/// Returns nullptr on parse error.
std::unique_ptr<AstFuncDecl> parse_function();
```

### User Documentation

- Update `docs/` for user-facing changes
- Add examples to `examples/` for new features
- Update `CHANGELOG.md` for notable changes

### Documentation Style

- Use clear, concise language
- Include code examples
- Keep examples minimal but complete
- Test all code examples

## Questions?

- Open a [Discussion](https://github.com/mana-lang/mana/discussions) for questions
- Join our [Discord](https://discord.gg/mana-lang) community
- Check existing issues before opening new ones

Thank you for contributing to Mana!
