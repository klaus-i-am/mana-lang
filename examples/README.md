# Mana Language Examples

This directory contains example programs demonstrating Mana language features.

## Getting Started

### Building Examples

```bash
cd examples
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Running Examples

```bash
./hello          # Basic hello world
./generics       # Generic functions and types
./traits         # Trait definitions and implementations
```

## Example Categories

### Basics

| File | Description |
|------|-------------|
| `hello.mana` | Hello world program |
| `generics.mana` | Generic functions and structs |
| `compound.mana` | Compound assignment operators |
| `const_test.mana` | Constant declarations |

### Control Flow

| File | Description |
|------|-------------|
| `match.mana` | Basic pattern matching |
| `match_enum.mana` | Matching on enum variants |
| `match_or_test.mana` | Or-patterns in match |
| `guard_test.mana` | Match guards |
| `range_match_test.mana` | Range patterns |
| `iflet.mana` | If-let expressions |
| `whilelet.mana` | While-let loops |
| `forin.mana` | For-in loops |
| `loop_test.mana` | Loop with break |

### Data Structures

| File | Description |
|------|-------------|
| `tuples.mana` | Tuple creation and destructuring |
| `destructure.mana` | Pattern destructuring |
| `destructure_test.mana` | Advanced destructuring |
| `struct_destructure.mana` | Struct field destructuring |
| `hashmap_test.mana` | HashMap operations |
| `hashmap_test2.mana` | More HashMap examples |

### Functions and Closures

| File | Description |
|------|-------------|
| `closures.mana` | Closure syntax and capturing |
| `default_params.mana` | Default parameter values |
| `named_args.mana` | Named function arguments |
| `closure_improvements.mana` | Advanced closure features |

### Traits and Generics

| File | Description |
|------|-------------|
| `traits.mana` | Trait definitions |
| `traits_simple.mana` | Basic trait example |
| `trait_defaults.mana` | Traits with default methods |
| `dynamic_dispatch.mana` | Runtime polymorphism |
| `assoc_types.mana` | Associated types |
| `constraints.mana` | Generic constraints |
| `impl_simple_test.mana` | Basic impl blocks |

### Types

| File | Description |
|------|-------------|
| `type_aliases.mana` | Type alias declarations |
| `type_alias_generic.mana` | Generic type aliases |
| `adt_test.mana` | Algebraic data types |
| `inference.mana` | Type inference |
| `cast_test.mana` | Type casting |

### Strings

| File | Description |
|------|-------------|
| `fstrings.mana` | Format strings |
| `string_methods.mana` | String methods |
| `string_methods2.mana` | More string operations |
| `string_split_test.mana` | String splitting |
| `raw_string_test.mana` | Raw string literals |
| `multiline_string_test.mana` | Multi-line strings |

### Numbers and Math

| File | Description |
|------|-------------|
| `math_test.mana` | Math functions |
| `hex_binary_test.mana` | Hex/binary literals |
| `octal_test.mana` | Octal literals |
| `scientific_test.mana` | Scientific notation |
| `power_test.mana` | Exponentiation operator |
| `fmod_test.mana` | Floating-point modulo |

### Collections and Iterators

| File | Description |
|------|-------------|
| `vec_method_test.mana` | Vector methods |
| `vec_method_test2.mana` | More vector operations |
| `slice_test.mana` | Array/vector slicing |
| `array_fill_test.mana` | Array fill syntax |
| `iter_test.mana` | Iterator basics |
| `iter_methods.mana` | Iterator methods |
| `iter_destructure.mana` | Destructuring in iteration |

### Error Handling

| File | Description |
|------|-------------|
| `error_handling.mana` | Result type usage |
| `result_test.mana` | Result operations |
| `assert_test.mana` | Assertions |
| `none_test.mana` | Option/None handling |
| `optional_chaining.mana` | ?. operator |
| `null_coalesce.mana` | ?? operator |

### Advanced Features

| File | Description |
|------|-------------|
| `pattern_test.mana` | Complex patterns |
| `pattern_simple.mana` | Pattern basics |
| `binding_test.mana` | Pattern binding |
| `range_patterns.mana` | Range in patterns |
| `async_test.mana` | Async/await |
| `operator_test.mana` | Operator overloading |

### Modules

| File | Description |
|------|-------------|
| `modules.mana` | Module declaration |
| `lib.mana` | Library module |
| `multifile.mana` | Multi-file project |
| `import_test.mana` | Import statements |

### Memory and Safety

| File | Description |
|------|-------------|
| `memory_test.mana` | Memory operations |
| `memory_safety.mana` | Safety features |
| `ref_test.mana` | References |
| `mut_ref_test.mana` | Mutable references |

### Testing

| File | Description |
|------|-------------|
| `testing_example.mana` | Test framework usage |
| `exhaustive_test.mana` | Exhaustive testing |

### File I/O

| File | Description |
|------|-------------|
| `file_io.mana` | File read/write operations |

## Compilation Options

```bash
# Emit C++ code
mana_lang example.mana --emit-cpp

# Run tests
mana_lang example.mana --test -c

# Generate documentation
mana_lang example.mana --doc

# Debug mode (verbose output)
mana_lang example.mana --debug
```

## Adding New Examples

1. Create your `.mana` file in this directory
2. Add an entry in `CMakeLists.txt`
3. Run cmake to regenerate build files
4. Build and test your example

## Example Template

```mana
module main;

// Your code here

fn main() -> i32 {
    println("Example output");
    return 0;
}
```
