# Mana Language - Polish & Tooling Roadmap

## Documentation

- [x] Language reference (all syntax, types, operators)
- [x] Standard library API docs
- [x] More tutorials/guides
- [x] Error message catalog with explanations
- [x] Contributing guide
- [ ] Changelog

### Existing Docs
- [x] README.md
- [x] TUTORIAL.md
- [x] BUILD_SYSTEM.md
- [x] INSTALLATION.md
- [x] RELEASE_NOTES_v1.0.0.md
- [x] STDLIB.md (Standard Library API Reference)

---

## Tooling

### Existing
- [x] LSP server (`mana-lsp`)
- [x] VS Code extension (`vscode-mana/`)
- [x] Package manager (`mana pkg`)
- [x] Debug build (`mana-debug`)

### TODO
- [x] Code formatter (`mana fmt`)
  - Consistent indentation
  - Brace style
  - Line length limits
- [x] Linter (`mana lint`)
  - Unused variables
  - Unreachable code
  - Style warnings
- [x] REPL (`mana repl`)
  - Interactive expression evaluation
  - History
  - Tab completion
- [ ] Debugger support
  - Breakpoints
  - Step through
  - Variable inspection
  - Integration with VS Code
- [ ] Playground
  - Web-based editor
  - Run in browser (WASM?)
  - Share snippets

---

## Build & Distribution

### Existing
- [x] CMake build system
- [x] Windows installer (`installer/`)
- [x] Build scripts (`scripts/build.bat`, `scripts/build.sh`)

### TODO
- [ ] Cross-compilation
  - Linux target from Windows
  - macOS target
  - ARM support
- [x] Release automation
  - GitHub Actions CI/CD
  - Automated builds
  - Artifact uploads
- [ ] Version management
  - Semantic versioning
  - Version in compiler output
  - Deprecation warnings
- [ ] Package registry
  - Central package index
  - Package publishing
  - Dependency resolution

---

## Testing

### Existing
- [x] Unit tests (41 tests passing)
- [x] Lexer tests
- [x] Parser tests
- [x] Semantic analysis tests
- [x] Code generation tests
- [x] Integration tests

### TODO
- [ ] Increase test coverage
  - Edge cases
  - Error paths
  - All language features
- [ ] Benchmark suite
  - Compile time benchmarks
  - Runtime performance tests
  - Comparison with other languages
- [ ] Fuzzing
  - Parser robustness
  - Crash detection
  - Invalid input handling
- [x] End-to-end tests
  - Full programs (30 test files covering all features)
  - Expected output verification
  - E2E test infrastructure with compile/run/verify pipeline

---

## Error Handling & Diagnostics

- [x] Better error messages
  - Show source code context
  - Underline the error location
  - Color-coded output
- [ ] Error recovery
  - Continue parsing after errors
  - Report multiple errors at once
- [x] Suggestions
  - "Did you mean X?"
  - Typo detection
  - Missing import suggestions
- [x] Warnings
  - Unused variables
  - Shadowed variables
  - Implicit type conversions
- [x] Notes
  - "Note: X was defined here"
  - Related information

---

## Performance

- [ ] Compile time profiling
  - Time spent in each phase
  - Identify bottlenecks
- [ ] Optimization passes
  - Dead code elimination
  - Constant folding
  - Inlining
- [ ] Incremental compilation
  - Only recompile changed files
  - Dependency tracking
  - Cache intermediate results
- [ ] Parallel compilation
  - Multi-threaded parsing
  - Parallel code generation

---

## Code Quality

- [ ] Refactor compiler internals
  - Clean up AST hierarchy
  - Consistent naming
  - Better separation of concerns
- [ ] Code comments
  - Document complex algorithms
  - Explain design decisions
- [ ] Architecture docs
  - Compiler pipeline overview
  - Module responsibilities

---

## Community

- [ ] Issue templates
- [ ] PR templates
- [ ] Code of conduct
- [ ] Discord/community chat
- [ ] Example projects
- [ ] Showcase/gallery

---

## Priority Order

1. ~~**High** - Error messages (user experience)~~ DONE
2. ~~**High** - Code formatter (consistency)~~ DONE
3. **Medium** - More tests (stability)
4. **Medium** - Language reference docs
5. ~~**Medium** - Linter~~ DONE
6. ~~**Low** - REPL~~ DONE
7. **Low** - Debugger
8. **Low** - Playground
