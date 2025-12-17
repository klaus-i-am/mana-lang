# Mana Language v1.0.0

We're excited to announce the first stable release of **Mana**, a modern systems programming language designed for game engines and performance-critical applications.

## Highlights

- **Modern Syntax** - Clean, expressive syntax inspired by Rust and Swift
- **C++ Backend** - Compiles to readable, efficient C++ code
- **Type Safety** - Strong static typing with powerful type inference
- **Zero-Cost Abstractions** - High-level features without runtime overhead

## Language Features

### Core Language
- Structs, enums, and algebraic data types (ADTs)
- Traits with default methods and associated types
- Generics with `where` constraints
- Pattern matching with guards and destructuring
- Closures with environment capture
- Async/await support

### Type System
- Primitive types: `i8`-`i64`, `u8`-`u64`, `f32`, `f64`, `bool`, `char`, `str`
- Compound types: arrays, tuples, vectors, hashmaps
- Optional types with `Option<T>` and `?`/`??` operators
- Result types for error handling with `?` propagation
- Type aliases and type inference

### Standard Library
- **Collections**: `Vec`, `HashMap`, `HashSet`, `Deque`
- **Strings**: formatting, splitting, searching, case conversion
- **File I/O**: read, write, append, path operations
- **Math**: trigonometry, rounding, min/max, random
- **Time**: timestamps and sleep

## Tooling

- **Compiler** (`mana_lang`) - Full compilation pipeline
- **LSP Server** (`mana-lsp`) - IDE integration
- **Debug Adapter** (`mana-debug`) - Debugging support
- **Package Manager** - `init`, `build`, `run`, `test` commands
- **Documentation Generator** - Generate docs from `///` comments
- **Test Framework** - `#[test]` attribute for unit tests

## Optimizations

- Dead code elimination
- Inline candidate identification
- Constant folding (via C++ backend)

## Documentation

- [Language Specification](docs/LANGUAGE_SPEC.md) - Complete language reference
- [Tutorial](docs/TUTORIAL.md) - Step-by-step guide from basics to advanced
- [Build System](docs/BUILD_SYSTEM.md) - Build and project configuration
- [Examples](examples/README.md) - 100+ example programs

## Building from Source

```bash
git clone https://github.com/klaus-i-am/mana-lang.git
cd mana-lang
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Quick Start

```mana
module main;

fn main() -> i32 {
    println("Hello, Mana!");
    return 0;
}
```

```bash
mana_lang hello.mana --emit-cpp
cd build && cmake .. && cmake --build . --config Release
./hello
```

## What's Next

- Package registry for dependency management
- More standard library modules
- Performance optimizations
- IDE plugins

---

Thank you for trying Mana! We welcome feedback and contributions.
