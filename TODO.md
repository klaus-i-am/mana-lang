# Mana Language TODO

This document tracks remaining work for the Mana language project.

---

## Tooling

- [ ] REPL improvements (history, autocomplete, multiline)
- [ ] Formatter (`mana fmt`) - auto-format Mana source files
- [ ] Linter rules expansion - more static analysis checks
- [ ] Test runner enhancements - parallel execution, coverage reports
- [ ] CI/CD pipeline - GitHub Actions for build/test
- [ ] Benchmarking tool - performance testing framework
- [ ] Documentation generator - generate API docs from source
- [ ] Watch mode - auto-rebuild on file changes
- [ ] Error recovery - better error messages with suggestions

---

## Stdlib Additions

- [ ] JSON parsing/serialization
- [ ] Date/time utilities
- [ ] Regex support
- [ ] Crypto (hashing: SHA256, MD5; encryption: AES)
- [ ] Compression (zip/gzip)
- [ ] Environment variables
- [ ] Command-line argument parsing
- [ ] Logging framework
- [ ] UUID generation
- [ ] Base64 encoding/decoding

---

## Language Features

- [ ] Trait/interface improvements
- [ ] Generics constraints (`where T: Trait`)
- [ ] Macro system
- [ ] Module visibility refinements
- [ ] Default function parameters
- [ ] Named function arguments
- [ ] String interpolation (`f"Hello {name}"`)
- [ ] Destructuring in let bindings
- [ ] Guard clauses in match
- [ ] Type inference improvements

---

## IDE/Editor Support

- [ ] VS Code debugging launch config
- [ ] More syntax highlighting refinements
- [ ] Code actions (quick fixes)
- [ ] Rename symbol
- [ ] Find all references
- [ ] Code folding regions
- [ ] Bracket pair colorization
- [ ] Semantic tokens
- [ ] Inlay hints (type annotations)
- [ ] Snippet completions

---

## Documentation

- [ ] Website/docs updates
- [ ] More example projects
- [ ] Tutorial series
- [ ] API reference generation
- [ ] Language specification
- [ ] Contributing guide
- [ ] Changelog maintenance

---

## Graphics/Game Dev

- [ ] Mouse input support
- [ ] Audio playback
- [ ] Sprite batching
- [ ] Tilemap support
- [ ] Simple physics
- [ ] Scene graph
- [ ] Asset loading (images, sounds)
- [ ] Font rendering

---

## Completed

- [x] Networking stdlib (TCP/UDP, HTTP client) - v1.2.4
- [x] Async runtime (ThreadPool, Task, Channel) - v1.2.4
- [x] Package registry support - v1.2.4
- [x] Debugger enhancements (source mapping, variables) - v1.2.4
- [x] Graphics/texture support - v1.2.4
- [x] VS Code auto-indent fix - v1.2.4
- [x] LSP hover/go-to-definition - v1.2.4
- [x] Pattern matching with exhaustiveness - v1.2.3
- [x] Result<T,E> and Option<T> types - v1.2.3
- [x] Math stdlib functions - v1.2.4
- [x] File I/O functions - v1.2.3

---

*Last updated: 2025-12-22*
