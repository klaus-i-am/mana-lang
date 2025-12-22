# Mana

A modern, readable programming language that compiles to C++. Designed for game engines and systems programming with a focus on clarity and safety.

```mana
module main

fn main() {
    println("Hello, Mana!")

    // Clean, expressive syntax
    for i in 1..=5 {
        println("Count: ", i)
    }
}
```

## Features

- **Clean syntax** - No semicolons required, type inference, expressive control flow
- **Compiles to C++17** - Zero-cost abstractions, full C++ interop
- **Modern type system** - Generics, traits, enums with data, Option/Result types
- **Pattern matching** - Destructuring, guards, exhaustiveness checking
- **Memory safe** - RAII, defer statements, no null by default
- **Great tooling** - LSP, VS Code extension, formatter, linter, REPL

## Quick Example

```mana
module demo

struct Point {
    x: f32,
    y: f32
}

impl Point {
    fn distance(self, other: Point) -> f32 {
        let dx = self.x - other.x
        let dy = self.y - other.y
        sqrt(dx * dx + dy * dy)
    }
}

enum Shape {
    Circle { center: Point, radius: f32 },
    Rectangle { origin: Point, width: f32, height: f32 }
}

fn area(shape: Shape) -> f32 {
    match shape {
        Circle { radius, .. } => 3.14159 * radius * radius,
        Rectangle { width, height, .. } => width * height
    }
}

fn main() {
    let shapes = [
        Shape::Circle { center: Point { x: 0.0, y: 0.0 }, radius: 5.0 },
        Shape::Rectangle { origin: Point { x: 0.0, y: 0.0 }, width: 10.0, height: 20.0 }
    ]

    for shape in shapes {
        println("Area: ", area(shape))
    }
}
```

## Installation

### From Source

```bash
git clone https://github.com/klaus-i-am/mana-lang.git
cd mana-lang
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Prerequisites

Mana compiles to C++17. You need a C++ compiler:

- **Windows**: Visual Studio 2022 or MinGW-w64
- **macOS**: `xcode-select --install`
- **Linux**: `sudo apt install g++` or equivalent

## Quick Start

```bash
mana new my-project      # Create a new project
cd my-project
mana build               # Compile to executable
mana run                 # Build and run
```

Or manually:

```mana
// hello.mana
module hello

fn main() -> i32 {
    println("Hello, Mana!")
    return 0
}
```

```bash
mana_lang hello.mana -c          # Generates hello.cpp
g++ -std=c++17 hello.cpp -o hello
./hello
```

See [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) for full setup instructions.

## Usage

```bash
# Compile to C++
mana_lang hello.mana -c

# Compile and emit to stdout
mana_lang hello.mana --emit-cpp

# Run tests
mana_lang test

# Format code
mana_lang fmt hello.mana

# Start REPL
mana_lang repl
```

## Language Highlights

### Variables and Types

```mana
let x = 42              // Immutable, type inferred
let mut counter = 0     // Mutable
let pi: f64 = 3.14159   // Explicit type

// Primitive types: i8, i16, i32, i64, u8, u16, u32, u64, f32, f64, bool, char, string
```

### Control Flow

```mana
// If expressions
let max = if a > b { a } else { b }

// Pattern matching
match value {
    0 => println("zero"),
    1..=9 => println("single digit"),
    n if n < 0 => println("negative"),
    _ => println("other")
}

// Loops
for i in 0..10 { }           // Range
for item in collection { }    // Iterator
while condition { }
loop { break }
```

### Functions and Closures

```mana
fn greet(name: string, times: i32 = 1) {
    for _ in 0..times {
        println("Hello, ", name, "!")
    }
}

let double = |x: i32| x * 2
let sum = numbers.fold(0, |acc, n| acc + n)
```

### Structs and Traits

```mana
struct Vec2 { x: f32, y: f32 }

trait Drawable {
    fn draw(self) -> void
}

impl Drawable for Vec2 {
    fn draw(self) {
        println("Point at (", self.x, ", ", self.y, ")")
    }
}
```

### Error Handling

```mana
fn divide(a: f32, b: f32) -> Option<f32> {
    if b == 0.0 { return none }
    some(a / b)
}

fn read_file(path: string) -> Result<string, Error> {
    // ...
}

// Use with pattern matching or ? operator
let result = divide(10.0, 2.0) or 0.0  // Default value
```

### Async/Await

```mana
async fn fetch_data(url: string) -> Result<string, Error> {
    let response = await http_get(url)?
    ok(response.body)
}
```

## Tooling

| Tool | Command | Description |
|------|---------|-------------|
| Compiler | `mana_lang` | Compile .mana to C++ |
| LSP | `mana-lsp` | Language server for IDEs |
| Formatter | `mana_lang fmt` | Auto-format code |
| Linter | `mana_lang lint` | Static analysis |
| REPL | `mana_lang repl` | Interactive mode |
| Package Manager | `mana_lang pkg` | Dependency management |

### VS Code Extension

Install the Mana extension from the `editors/vscode-mana` folder for:
- Syntax highlighting
- Error diagnostics
- Code completion
- Go to definition
- Hover documentation

## Documentation

- [Getting Started](docs/GETTING_STARTED.md) - Create your first project
- [VS Code Setup](docs/VSCODE_SETUP.md) - IDE configuration
- [Language Tutorial](docs/TUTORIAL.md) - Learn Mana step by step
- [Language Reference](docs/LANGUAGE_REFERENCE.md) - Complete syntax reference
- [Standard Library](docs/STDLIB.md) - Built-in functions and types
- [Build System](docs/BUILD_SYSTEM.md) - Project configuration
- [Error Catalog](docs/ERROR_CATALOG.md) - Error messages explained

## Examples

The `examples/` directory contains many sample programs:

- `hello.mana` - Hello world
- `forin.mana` - For loops and ranges
- `match_option.mana` - Pattern matching
- `closures.mana` - Lambda functions
- `async_test.mana` - Async/await
- `graphics/` - OpenGL graphics demos

## Contributing

Contributions are welcome! Please read the contributing guidelines before submitting PRs.

## License

MIT License - see LICENSE file for details.

---

**Mana** - Write code that reads like prose, runs like C++.
