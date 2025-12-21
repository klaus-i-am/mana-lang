# Changelog

All notable changes to the Mana programming language will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.4] - 2024-12-20

### Added

- **`or` with default value**: Use `expr or default_value` to unwrap Result/Option with a fallback
  ```mana
  let result = divide(10.0, 0.0) or 0.0  // Returns 0.0 on error
  ```

- **For-in loops with arrays**: Iterate directly over array literals
  ```mana
  let nums = [1, 2, 3, 4, 5]
  for n in nums {
      println(n)
  }
  ```

### Fixed

- Array type emission in C++ backend: `[5]i32` now correctly emits as `std::array<int32_t, 5>`

### Changed

- Updated playground examples:
  - `error_handling.mana`: Now demonstrates `or` with default value
  - `iterators.mana`: New example showing for-in with ranges and arrays
  - Removed `loops.mana` (functionality merged into iterators)

---

## [1.0.0] - 2024-12-20

### Added

#### Language Features
- **Core Type System**: Static typing with type inference, generics, and traits
- **Ownership Model**: Rust-inspired ownership with borrowing and lifetimes
- **Pattern Matching**: Exhaustive pattern matching with `match` expressions
- **Error Handling**: `Result<T, E>` and `Option<T>` types with `?` operator
- **Modules**: File-based module system with `pub` visibility
- **Structs & Enums**: User-defined types with methods and associated functions
- **Traits**: Interface abstraction with default implementations
- **Generics**: Parametric polymorphism with trait bounds
- **Closures**: Anonymous functions with environment capture
- **Iterators**: Lazy iteration with combinators (`map`, `filter`, `fold`)

#### Compiler
- **C++17 Backend**: Compiles to human-readable C++17 code
- **Incremental Compilation**: Only recompile changed modules
- **Cross-Platform**: Windows, Linux, and macOS support
- **Helpful Errors**: Rust-style error messages with suggestions

#### Tooling
- **Language Server (LSP)**: Full IDE support with `mana-lsp`
- **Debug Adapter (DAP)**: Debugging support with `mana-debug`
- **Formatter**: `mana fmt` for consistent code style
- **Linter**: `mana lint` for catching common issues
- **REPL**: Interactive mode with `mana repl`
- **Package Manager**: Dependency management with `mana pkg`
- **Test Runner**: Built-in testing with `#[test]` attribute
- **Documentation Generator**: `mana doc` for API docs

#### Standard Library
- Core types: `i8`-`i64`, `u8`-`u64`, `f32`, `f64`, `bool`, `char`, `String`
- Collections: `Vec<T>`, `HashMap<K,V>`, `HashSet<T>`
- I/O: File operations, stdin/stdout
- Concurrency: Threads, channels, mutexes

#### Editor Support
- **VS Code Extension**: Syntax highlighting, LSP integration, snippets

### Infrastructure
- GitHub Actions CI/CD for automated builds
- Cross-platform release binaries with checksums
- Installation scripts for all platforms

## [Unreleased]

### Planned
- WebAssembly backend
- Async/await support
- Macro system
- Package registry

---

[1.2.4]: https://github.com/klaus-i-am/mana-lang/releases/tag/v1.2.4
[1.0.0]: https://github.com/klaus-i-am/mana-lang/releases/tag/v1.0.0
[Unreleased]: https://github.com/klaus-i-am/mana-lang/compare/v1.2.4...HEAD
