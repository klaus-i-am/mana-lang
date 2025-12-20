# Mana Build System

This document describes how to build Mana projects.

## Quick Start

### Building the Compiler

```bash
# Windows
scripts\build.bat release

# Unix/Linux/macOS
./scripts/build.sh release
```

### Building a Project

```bash
# Compile a single file
mana_lang myfile.mana --emit-cpp

# Build the generated C++ code
cd build && cmake .. && cmake --build . --config Release
```

## Build Scripts

### Windows (build.bat)

```
build.bat [command]

Commands:
  debug     - Build compiler in debug mode
  release   - Build compiler in release mode (default)
  clean     - Remove all build artifacts
  test      - Build and run tests
  examples  - Build all example programs
  all       - Build compiler, examples, and run tests
  help      - Show help message
```

### Unix/Linux/macOS (build.sh)

```
./build.sh [command]

Commands:
  debug     - Build compiler in debug mode
  release   - Build compiler in release mode (default)
  clean     - Remove all build artifacts
  test      - Build and run tests
  examples  - Build all example programs
  all       - Build compiler, examples, and run tests
  help      - Show help message
```

## Project Configuration (mana.toml)

Mana projects can be configured using an `mana.toml` file:

```toml
[package]
name = "my_project"
version = "1.0.0"
authors = ["Your Name <email@example.com>"]
description = "My Mana project"

[build]
output = "build"
target = "executable"  # or "library"
optimize = true
debug_info = false

[dependencies]
# Local dependency
utils = { path = "../utils" }

# Future: package registry
# http = "1.2.0"

[[bin]]
name = "my_app"
path = "src/main.mana"

[[lib]]
name = "my_lib"
path = "src/lib.mana"
```

## Compiler Options

```
mana_lang [options] <source_file>

Options:
  --emit-cpp        Generate C++ code
  --output, -o      Output file path
  --debug           Enable debug output
  --test            Build and run tests
  --doc             Generate documentation
  --check           Type-check only (no code generation)
  -c                Compile to executable
  --version         Show version
  --help            Show help
```

## Directory Structure

Recommended project structure:

```
my_project/
├── mana.toml          # Project configuration
├── src/
│   ├── main.mana      # Main entry point
│   └── lib.mana       # Library code
├── tests/
│   └── test_main.mana # Test files
├── examples/
│   └── example.mana   # Example programs
├── docs/
│   └── README.md       # Documentation
└── build/              # Build output (generated)
```

## CMake Integration

Mana generates CMakeLists.txt for C++ compilation:

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_project)

set(CMAKE_CXX_STANDARD 17)

# Generated from mana.toml
add_executable(my_app
    src/main.cpp
    src/lib.cpp
)

# Runtime library
target_include_directories(my_app PRIVATE ${MANA_RUNTIME_DIR})
```

## Build Workflow

1. **Write Mana code** - Create `.mana` source files
2. **Compile to C++** - Run `mana_lang --emit-cpp`
3. **Build C++** - Use CMake or your C++ build system
4. **Run executable** - Execute the built binary

### Example Workflow

```bash
# 1. Create source file (vNext syntax - no semicolons, optional main return)
echo 'module main

fn main() {
    println("Hello!")
}' > hello.mana

# 2. Compile to C++
mana_lang hello.mana --emit-cpp

# 3. Build with CMake
mkdir build && cd build
cmake ..
cmake --build . --config Release

# 4. Run
./hello
```

## Multi-File Projects

For projects with multiple source files:

```bash
# Main file imports other modules
mana_lang main.mana --emit-cpp

# Compiler automatically resolves imports
# and generates all necessary C++ files
```

### Import Resolution

The compiler searches for imported modules in:
1. Current directory
2. `src/` subdirectory
3. Paths specified in `mana.toml`
4. Standard library path

## Testing

### Writing Tests

```mana
#[test]
fn test_addition() {
    assert_eq(1 + 1, 2)
}

#[test]
fn test_string_concat() {
    let s = "hello" + " world"
    assert_eq(s, "hello world")
}
```

### Running Tests

```bash
mana_lang tests/test_main.mana --test -c
./build/test_main
```

## Documentation Generation

Generate markdown documentation from doc comments:

```bash
mana_lang src/lib.mana --doc
# Creates: src/lib.md
```

### Doc Comment Syntax

```mana
/// Calculates the factorial of n.
///
/// # Arguments
/// * `n` - The number to calculate factorial for
///
/// # Returns
/// The factorial of n
///
/// # Example
/// ```mana
/// let result = factorial(5)
/// assert_eq(result, 120)
/// ```
fn factorial(n: int) -> int {
    if n <= 1 {
        return 1
    }
    return n * factorial(n - 1)
}
```

## IDE Integration

### VS Code

Install the Mana extension for:
- Syntax highlighting
- Code completion (via LSP)
- Error diagnostics
- Go to definition

LSP server location: `build/Release/mana-lsp`

### Visual Studio

Add `.mana` file association and configure external tools.

## Troubleshooting

### Common Issues

**Error: "Module not found"**
- Check import paths in `mana.toml`
- Ensure module file exists

**Error: "C++ compilation failed"**
- Check C++ compiler is installed
- Verify C++17 support

**Error: "Runtime not found"**
- Copy `mana_runtime.h` to include path
- Set `MANA_RUNTIME_DIR` environment variable

### Debug Mode

Enable verbose output:

```bash
mana_lang --debug myfile.mana --emit-cpp
```

This shows:
- Token stream
- AST dump
- Semantic analysis details
- Code generation steps

## vNext Syntax Support

The build system fully supports vNext syntax features:

| Feature | Example |
|---------|---------|
| Optional semicolons | `let x = 42` |
| Type aliases | `int` for `i32`, `float` for `f32` |
| Optional main return | `fn main() { }` |
| `when` expressions | `when x { 1 -> "one" _ -> "other" }` |
| `or` operator | `value or default` |
| `variant` keyword | `variant Color { Red, Green, Blue }` |
| Variadic println | `println("Value: ", x)` |

See [Language Specification](LANGUAGE_SPEC.md) for full syntax reference.
