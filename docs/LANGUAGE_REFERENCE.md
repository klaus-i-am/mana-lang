# Mana Language Reference

This document provides a complete reference for the Mana programming language syntax, types, and semantics.

---

## Table of Contents

1. [Lexical Structure](#1-lexical-structure)
2. [Types](#2-types)
3. [Expressions](#3-expressions)
4. [Statements](#4-statements)
5. [Declarations](#5-declarations)
6. [Patterns](#6-patterns)
7. [Modules](#7-modules)
8. [Attributes](#8-attributes)
9. [Built-in Functions](#9-built-in-functions)
10. [Operators](#10-operators)

---

## 1. Lexical Structure

### 1.1 Comments

```mana
// Single-line comment

/* Multi-line
   comment */

/// Documentation comment (attached to following item)
```

### 1.2 Identifiers

Identifiers start with a letter or underscore, followed by letters, digits, or underscores.

```
identifier = [a-zA-Z_][a-zA-Z0-9_]*
```

Reserved keywords cannot be used as identifiers.

### 1.3 Keywords

| Category | Keywords |
|----------|----------|
| Declarations | `fn`, `struct`, `enum`, `trait`, `impl`, `type`, `const`, `static` |
| Control Flow | `if`, `else`, `match`, `while`, `for`, `loop`, `break`, `continue`, `return` |
| Variables | `let`, `mut` |
| Modules | `module`, `import`, `use`, `from`, `pub` |
| Types | `self`, `Self`, `dyn` |
| Values | `true`, `false`, `none` |
| Memory | `move` |
| Async | `async`, `await` |
| Other | `in`, `as`, `where`, `defer`, `scope` |

### 1.4 Literals

#### Integer Literals

```mana
42          // Decimal
0x2A        // Hexadecimal
0o52        // Octal
0b101010    // Binary
1_000_000   // With separators
```

#### Floating-Point Literals

```mana
3.14
2.5e10
1.0e-5
3.14_159   // With separators
```

#### String Literals

```mana
"hello"                    // Regular string
"line1\nline2"             // With escape sequences
r"no\escape"               // Raw string (no escape processing)
"""
Multi-line
string
"""                        // Multi-line string
f"Hello, {name}!"          // Interpolated string (f-string)
```

#### Escape Sequences

| Escape | Meaning |
|--------|---------|
| `\n` | Newline |
| `\r` | Carriage return |
| `\t` | Tab |
| `\\` | Backslash |
| `\"` | Double quote |
| `\'` | Single quote |
| `\0` | Null character |
| `\xHH` | Hex byte |
| `\u{HHHH}` | Unicode code point |

#### Character Literals

```mana
'a'
'\n'
'\u{1F600}'
```

#### Boolean Literals

```mana
true
false
```

#### None Literal

```mana
none    // Represents absence of value (Option type)
```

### 1.5 Operators and Punctuation

#### Arithmetic Operators

| Operator | Description |
|----------|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division |
| `%` | Modulo |
| `**` | Power |

#### Compound Assignment

| Operator | Equivalent |
|----------|------------|
| `+=` | `a = a + b` |
| `-=` | `a = a - b` |
| `*=` | `a = a * b` |
| `/=` | `a = a / b` |
| `%=` | `a = a % b` |
| `**=` | `a = a ** b` |

#### Increment/Decrement

| Operator | Description |
|----------|-------------|
| `++` | Increment |
| `--` | Decrement |

#### Comparison Operators

| Operator | Description |
|----------|-------------|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `<=` | Less than or equal |
| `>` | Greater than |
| `>=` | Greater than or equal |

#### Logical Operators

| Operator | Description |
|----------|-------------|
| `&&` | Logical AND |
| `\|\|` | Logical OR |
| `!` | Logical NOT |

#### Bitwise Operators

| Operator | Description |
|----------|-------------|
| `&` | Bitwise AND |
| `\|` | Bitwise OR |
| `^` | Bitwise XOR |
| `~` | Bitwise NOT |
| `<<` | Left shift |
| `>>` | Right shift |

#### Bitwise Compound Assignment

| Operator | Equivalent |
|----------|------------|
| `&=` | `a = a & b` |
| `\|=` | `a = a \| b` |
| `^=` | `a = a ^ b` |
| `<<=` | `a = a << b` |
| `>>=` | `a = a >> b` |

#### Special Operators

| Operator | Description |
|----------|-------------|
| `?` | Error propagation |
| `?.` | Optional chaining |
| `??` | Null coalescing |
| `..` | Exclusive range |
| `..=` | Inclusive range |
| `as` | Type cast |

#### Punctuation

| Symbol | Description |
|--------|-------------|
| `(` `)` | Parentheses |
| `{` `}` | Braces |
| `[` `]` | Brackets |
| `,` | Comma |
| `;` | Semicolon |
| `:` | Colon (type annotation) |
| `::` | Path separator |
| `.` | Member access |
| `->` | Return type arrow |
| `=>` | Match arm arrow |
| `=` | Assignment |
| `_` | Wildcard/discard |
| `#` | Attribute prefix |

---

## 2. Types

### 2.1 Primitive Types

| Type | Description | Size |
|------|-------------|------|
| `i32` | Signed 32-bit integer | 4 bytes |
| `i64` | Signed 64-bit integer | 8 bytes |
| `u32` | Unsigned 32-bit integer | 4 bytes |
| `u64` | Unsigned 64-bit integer | 8 bytes |
| `f32` | 32-bit floating point | 4 bytes |
| `f64` | 64-bit floating point | 8 bytes |
| `bool` | Boolean | 1 byte |
| `char` | Unicode character | 4 bytes |
| `string` | UTF-8 string | varies |
| `void` | No value | 0 bytes |

### 2.2 Compound Types

#### Arrays

Fixed-size contiguous collection of elements.

```mana
[N]T        // Array of N elements of type T

let arr: [5]i32 = [1, 2, 3, 4, 5];
let zeros: [10]i32 = [0; 10];     // Initialize all to 0
```

#### Tuples

Fixed-size heterogeneous collection.

```mana
(T1, T2, ...)

let point: (i32, i32) = (10, 20);
let mixed: (i32, string, bool) = (42, "hello", true);

// Access by index
let x = point.0;
let y = point.1;
```

#### Slices

View into a contiguous sequence.

```mana
let arr = [1, 2, 3, 4, 5];
let slice = arr[1..4];    // [2, 3, 4]
```

### 2.3 Reference Types

#### Immutable Reference

```mana
&T

fn print_value(x: &i32) {
    println(f"{x}");
}
```

#### Mutable Reference

```mana
&mut T

fn increment(x: &mut i32) {
    *x = *x + 1;
}
```

#### Pointer

```mana
*T

let ptr: *i32 = &value;
```

### 2.4 User-Defined Types

#### Struct

```mana
struct Name {
    field1: Type1,
    field2: Type2,
}
```

#### Enum

```mana
// Simple enum
enum Direction {
    North,
    South,
    East,
    West,
}

// Enum with data
enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

#### Type Alias

```mana
type Name = ExistingType;

type Point = (f32, f32);
type StringMap = HashMap<string, string>;
```

### 2.5 Function Types

```mana
fn(T1, T2) -> R

let add: fn(i32, i32) -> i32 = |a, b| a + b;
```

### 2.6 Generic Types

```mana
struct Container<T> {
    value: T,
}

fn convert<T, U>(x: T) -> U { ... }
```

### 2.7 Option and Result

```mana
// Option - represents optional value
Option<T> = Some(T) | None

let maybe: Option<i32> = some(42);
let nothing: Option<i32> = none;

// Result - represents success or failure
Result<T, E> = Ok(T) | Err(E)

let success: Result<i32, string> = ok(42);
let failure: Result<i32, string> = err("failed");
```

### 2.8 Dynamic Types

```mana
dyn Trait    // Trait object (dynamic dispatch)

fn print_all(items: Vec<dyn Display>) { ... }
```

---

## 3. Expressions

### 3.1 Literal Expressions

```mana
42              // Integer
3.14            // Float
"hello"         // String
'a'             // Char
true            // Bool
none            // None
```

### 3.2 Identifier Expressions

```mana
variable_name
function_name
Type::associated_function
module::item
```

### 3.3 Operator Expressions

#### Unary Expressions

```mana
-x          // Negation
!x          // Logical NOT
~x          // Bitwise NOT
*ptr        // Dereference
&x          // Reference
&mut x      // Mutable reference
```

#### Binary Expressions

```mana
a + b       // Arithmetic
a == b      // Comparison
a && b      // Logical
a & b       // Bitwise
a..b        // Range
```

### 3.4 Call Expressions

```mana
function(arg1, arg2)
object.method(arg1)
Type::associated_function(arg1)
```

### 3.5 Index Expressions

```mana
array[index]
array[start..end]       // Slice
```

### 3.6 Member Access

```mana
object.field
object.method()
tuple.0
object?.field           // Optional chaining
```

### 3.7 Struct Literals

```mana
Point { x: 10, y: 20 }
Point { x, y }              // Shorthand when variable name matches field
Point { x: 10, ..default }  // Struct update syntax
```

### 3.8 Array Literals

```mana
[1, 2, 3, 4, 5]
[0; 10]                     // [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
```

### 3.9 Tuple Literals

```mana
(1, 2)
(a, b, c)
()                          // Unit tuple
```

### 3.10 Range Expressions

```mana
start..end              // Exclusive: [start, end)
start..=end             // Inclusive: [start, end]
..end                   // From beginning
start..                 // To end
..                      // Full range
```

### 3.11 Closure Expressions

```mana
|params| expr
|params| { statements }
|x: i32, y: i32| -> i32 { x + y }

// Capture modes
|x| x + captured        // Borrow by default
move |x| x + captured   // Move captured variables
```

### 3.12 If Expressions

```mana
if condition { expr1 } else { expr2 }

let max = if a > b { a } else { b };
```

### 3.13 Match Expressions

```mana
match value {
    pattern1 => expr1,
    pattern2 => expr2,
    _ => default_expr,
}

let name = match status {
    0 => "ok",
    1 => "warning",
    _ => "error",
};
```

### 3.14 Cast Expressions

```mana
expr as Type

let f = 42 as f64;
let i = 3.14 as i32;
```

### 3.15 Try Expressions

```mana
expr?       // Propagate error if Result is Err or Option is None
```

### 3.16 Null Coalescing

```mana
expr ?? default     // Use default if expr is none
```

### 3.17 Await Expressions

```mana
future.await        // Await async result
```

---

## 4. Statements

### 4.1 Variable Declaration

```mana
let name = value;                   // Immutable, type inferred
let name: Type = value;             // Explicit type
let mut name = value;               // Mutable
const NAME: Type = value;           // Compile-time constant
```

### 4.2 Assignment

```mana
variable = value;
object.field = value;
array[index] = value;

// Compound assignment
x += 1;
x -= 1;
x *= 2;
```

### 4.3 Expression Statements

```mana
function_call();
object.method();
expr;
```

### 4.4 Block Statements

```mana
{
    statement1;
    statement2;
    expr        // Last expression is block's value (no semicolon)
}
```

### 4.5 If Statements

```mana
if condition {
    // then branch
}

if condition {
    // then branch
} else {
    // else branch
}

if condition1 {
    // ...
} else if condition2 {
    // ...
} else {
    // ...
}
```

### 4.6 While Loops

```mana
while condition {
    // body
}
```

### 4.7 For Loops

```mana
// C-style for loop
for init; condition; update {
    // body
}

for let i = 0; i < 10; i++ {
    println(f"{i}");
}

// For-in loop (iteration)
for item in collection {
    // body
}

for i in 0..10 {
    println(f"{i}");
}

for (index, value) in collection.enumerate() {
    // body
}
```

### 4.8 Loop Statement

```mana
loop {
    // infinite loop, use break to exit
    if condition {
        break;
    }
}

// Loop with value
let result = loop {
    if done {
        break value;
    }
};
```

### 4.9 Break and Continue

```mana
break;              // Exit innermost loop
break value;        // Exit with value (for loop expressions)
continue;           // Skip to next iteration
```

### 4.10 Return Statement

```mana
return;             // Return void
return value;       // Return with value
```

### 4.11 Match Statement

```mana
match value {
    pattern1 => {
        // statements
    },
    pattern2 => expr,
    _ => {
        // default
    },
}
```

### 4.12 Defer Statement

```mana
defer {
    // Executed when scope exits (LIFO order)
}

fn example() {
    let file = open("data.txt");
    defer { file.close(); }

    // Use file...
    // file.close() called automatically when function returns
}
```

### 4.13 Scope Statement

```mana
scope {
    // Creates a new scope
    // Variables declared here are dropped at end
}
```

---

## 5. Declarations

### 5.1 Module Declaration

```mana
module name;

// Must be first declaration in file
module game::engine;    // Nested module path
```

### 5.2 Import Declaration

```mana
// Import module (access via module::item)
import module_name;
import module::submodule;

// Import from file path
import "path/to/file.mana";

// Use declaration - import specific items into scope
use module::item;
use module::{ item1, item2 };    // Selective import
use module::*;                    // Import all public items
use module::item as alias;        // Import with alias
```

### 5.3 Function Declaration

```mana
fn name(param1: Type1, param2: Type2) -> ReturnType {
    // body
}

// No return type (void)
fn greet(name: string) {
    println(f"Hello, {name}!");
}

// Public function
pub fn public_function() { }

// Default parameters
fn connect(host: string, port: i32 = 8080) { }

// Generic function
fn identity<T>(x: T) -> T {
    return x;
}

// With trait bounds
fn print_item<T>(item: T) where T: Display {
    println(item.to_string());
}

// Async function
async fn fetch_data() -> Result<Data, Error> {
    // ...
}
```

### 5.4 Struct Declaration

```mana
struct Name {
    field1: Type1,
    field2: Type2,
}

// Public struct with mixed visibility
pub struct Player {
    pub name: string,       // Public field
    pub health: i32,        // Public field
    id: u64,                // Private field
}

// Generic struct
struct Container<T> {
    value: T,
}

// Tuple struct
struct Point(f32, f32);

// Unit struct
struct Marker;
```

### 5.5 Enum Declaration

```mana
enum Name {
    Variant1,
    Variant2,
    Variant3,
}

// With explicit values
enum Status {
    Ok = 0,
    Warning = 1,
    Error = 2,
}

// Algebraic data type (variants with data)
enum Message {
    Quit,
    Move { x: i32, y: i32 },
    Write(string),
    Color(i32, i32, i32),
}

// Generic enum
enum Result<T, E> {
    Ok(T),
    Err(E),
}
```

### 5.6 Impl Block

```mana
impl StructName {
    // Associated function (no self)
    fn new() -> StructName {
        return StructName { ... };
    }

    // Method (takes self)
    fn method(self) -> ReturnType {
        return self.field;
    }

    // Mutable method
    fn mutate(mut self) {
        self.field = new_value;
    }

    // Reference method
    fn borrow(&self) -> &Field {
        return &self.field;
    }
}

// Impl for generic type
impl<T> Container<T> {
    fn get(self) -> T {
        return self.value;
    }
}
```

### 5.7 Trait Declaration

```mana
trait TraitName {
    // Required method (no body)
    fn required_method(self) -> Type;

    // Provided method (has default implementation)
    fn provided_method(self) {
        // default implementation
    }

    // Associated type
    type Item;

    // Associated constant
    const MAX: i32;
}

// Trait with generic
trait Container<T> {
    fn get(self) -> T;
    fn set(mut self, value: T);
}
```

### 5.8 Trait Implementation

```mana
impl TraitName for StructName {
    fn required_method(self) -> Type {
        // implementation
    }
}

// Impl trait for generic type
impl<T> Display for Container<T> where T: Display {
    fn to_string(self) -> string {
        return f"Container({self.value})";
    }
}
```

### 5.9 Type Alias Declaration

```mana
type Alias = ExistingType;

type Point = (f32, f32);
type Result<T> = std::Result<T, Error>;
```

### 5.10 Global Variable Declaration

```mana
const PI: f64 = 3.14159265358979;

static mut COUNTER: i32 = 0;
```

---

## 6. Patterns

Patterns are used in `match` expressions, `let` bindings, and function parameters.

### 6.1 Literal Patterns

```mana
match value {
    0 => "zero",
    1 => "one",
    "hello" => "greeting",
    true => "yes",
    _ => "other",
}
```

### 6.2 Identifier Patterns

```mana
match value {
    x => use(x),    // Binds value to x
}

let x = 42;         // Binds 42 to x
```

### 6.3 Wildcard Pattern

```mana
match value {
    _ => "anything",    // Matches anything, doesn't bind
}

let (x, _) = pair;      // Ignore second element
```

### 6.4 Tuple Patterns

```mana
match tuple {
    (0, 0) => "origin",
    (x, 0) => f"on x-axis at {x}",
    (0, y) => f"on y-axis at {y}",
    (x, y) => f"at ({x}, {y})",
}

let (a, b, c) = triple;
```

### 6.5 Struct Patterns

```mana
match point {
    Point { x: 0, y: 0 } => "origin",
    Point { x, y: 0 } => f"on x-axis at {x}",
    Point { x: 0, y } => f"on y-axis at {y}",
    Point { x, y } => f"at ({x}, {y})",
}

let Point { x, y } = point;
```

### 6.6 Enum Patterns

```mana
match message {
    Message::Quit => "quit",
    Message::Move { x, y } => f"move to ({x}, {y})",
    Message::Write(text) => f"write: {text}",
    Message::Color(r, g, b) => f"color: rgb({r}, {g}, {b})",
}
```

### 6.7 Range Patterns

```mana
match value {
    0..=9 => "single digit",
    10..=99 => "two digits",
    100..=999 => "three digits",
    _ => "many digits",
}
```

### 6.8 Or Patterns

```mana
match value {
    0 | 1 => "zero or one",
    2 | 3 | 5 | 7 => "small prime",
    _ => "other",
}
```

### 6.9 Guard Patterns

```mana
match value {
    x if x < 0 => "negative",
    x if x > 0 => "positive",
    _ => "zero",
}
```

### 6.10 Option/Result Patterns

```mana
match option {
    some(x) => use(x),
    none => default(),
}

match result {
    ok(value) => use(value),
    err(e) => handle_error(e),
}

// If-let pattern
if let some(x) = option {
    use(x);
}

// While-let pattern
while let some(item) = iterator.next() {
    process(item);
}
```

---

## 7. Modules

### 7.1 Module Structure

```
project/
├── mana.toml           # Package manifest
├── src/
│   ├── main.mana       # Entry point
│   ├── lib.mana        # Library root
│   ├── utils.mana      # Module file
│   └── game/
│       ├── mod.mana    # Submodule root
│       ├── player.mana
│       └── enemy.mana
```

### 7.2 Module Declaration

```mana
// src/main.mana
module main;

// src/utils.mana
module utils;

// src/game/player.mana
module game::player;
```

### 7.3 Imports

```mana
// Import entire module (access via module::item)
import utils;
utils::helper();

// Import from file path
import "game/player.mana";

// Use declaration - import items into scope
use utils::helper;              // Import single item
use utils::{ helper, Config };  // Import multiple items
use utils::*;                   // Import all public items
use utils::helper as h;         // Import with alias
h();
```

### 7.4 Visibility

```mana
// Private by default
fn private_function() { }
struct PrivateStruct { }

// Public (visible outside module)
pub fn public_function() { }
pub struct PublicStruct {
    pub public_field: i32,
    private_field: i32,     // Still private
}
```

### 7.5 Re-exports

```mana
// In lib.mana
pub use utils::Config;      // Re-export Config
pub use game::*;            // Re-export all from game
```

---

## 8. Attributes

Attributes provide metadata about items.

### 8.1 Syntax

```mana
#[attribute]
#[attribute = "value"]
#[attribute(arg1, arg2)]
```

### 8.2 Common Attributes

| Attribute | Description | Status |
|-----------|-------------|--------|
| `#[test]` | Marks function as test | Implemented |
| `#[ignore]` | Skip this test | Implemented |
| `#[ignore = "reason"]` | Skip with reason | Implemented |
| `#[should_panic]` | Test should panic | Implemented |
| `#[tag("name")]` | Tag test for filtering | Implemented |
| `#[deprecated]` | Mark as deprecated | Planned |
| `#[inline]` | Suggest inlining | Planned |
| `#[derive(...)]` | Auto-derive traits | Planned |
| `#[cfg(...)]` | Conditional compilation | Planned |
| `#[allow(...)]` | Suppress warning | Planned |
| `#[deny(...)]` | Treat warning as error | Planned |

### 8.3 Examples

```mana
#[test]
fn test_addition() {
    assert_eq(2 + 2, 4);
}

#[test]
#[should_panic]
fn test_panic() {
    panic("expected panic");
}

#[test]
#[ignore = "not implemented yet"]
fn test_future_feature() {
    // ...
}

#[test]
#[tag("math")]
fn test_math_operations() {
    // tagged test for filtering with --tag math
}
```

---

## 9. Built-in Functions

### 9.1 I/O Functions

| Function | Description |
|----------|-------------|
| `print(msg)` | Print without newline |
| `println(msg)` | Print with newline |
| `input()` | Read line from stdin |
| `input(prompt)` | Print prompt, read line |

### 9.2 Type Conversion

| Function | Description |
|----------|-------------|
| `to_string(x)` | Convert to string |
| `parse_int(s)` | Parse string to i32 |
| `parse_float(s)` | Parse string to f64 |

### 9.3 Math Functions

| Function | Description |
|----------|-------------|
| `abs(x)` | Absolute value |
| `min(a, b)` | Minimum of two values |
| `max(a, b)` | Maximum of two values |
| `sqrt(x)` | Square root |
| `pow(base, exp)` | Power |
| `sin(x)`, `cos(x)`, `tan(x)` | Trigonometric |
| `floor(x)`, `ceil(x)`, `round(x)` | Rounding |
| `log(x)`, `log10(x)`, `log2(x)` | Logarithms |

### 9.4 Assertions

| Function | Description |
|----------|-------------|
| `assert(cond)` | Assert condition is true |
| `assert(cond, msg)` | Assert with message |
| `assert_eq(a, b)` | Assert equality |
| `assert_ne(a, b)` | Assert inequality |
| `panic(msg)` | Abort with message |
| `unreachable()` | Mark unreachable code |

### 9.5 Memory

| Function | Description |
|----------|-------------|
| `sizeof<T>()` | Size of type in bytes |
| `alignof<T>()` | Alignment of type |
| `drop(x)` | Explicitly drop value |

### 9.6 Debugging

| Function | Description |
|----------|-------------|
| `dbg(x)` | Debug print with location |
| `todo()` | Mark unfinished code |
| `todo(msg)` | Mark with message |

---

## 10. Operators

### 10.1 Operator Precedence (highest to lowest)

| Precedence | Operators | Associativity | Description |
|------------|-----------|---------------|-------------|
| 1 | `()` `[]` `.` `?.` `::` `.await` | Left | Call, index, member access |
| 2 | `?` (try) `as` | Left | Error propagation, cast |
| 3 | `!` `-` `~` `*` `&` `&mut` (unary) | Right | Unary operators |
| 4 | `**` | Right | Power/exponentiation |
| 5 | `*` `/` `%` | Left | Multiplicative |
| 6 | `+` `-` | Left | Additive |
| 7 | `<<` `>>` | Left | Bit shift |
| 8 | `<` `<=` `>` `>=` | Left | Relational comparison |
| 9 | `==` `!=` | Left | Equality comparison |
| 10 | `&` | Left | Bitwise AND |
| 11 | `^` | Left | Bitwise XOR |
| 12 | `\|` | Left | Bitwise OR |
| 13 | `&&` | Left | Logical AND |
| 14 | `\|\|` | Left | Logical OR |
| 15 | `??` | Left | Null coalescing |
| 16 | `..` `..=` | Left | Range |
| 17 | `=` `+=` `-=` `*=` `/=` `%=` etc. | Right | Assignment |

### 10.2 Operator Overloading

Operators can be overloaded by implementing traits:

| Operator | Trait |
|----------|-------|
| `+` | `Add` |
| `-` | `Sub` |
| `*` | `Mul` |
| `/` | `Div` |
| `%` | `Rem` |
| `-` (unary) | `Neg` |
| `!` | `Not` |
| `[]` | `Index` / `IndexMut` |
| `==` | `PartialEq` |
| `<` `<=` `>` `>=` | `PartialOrd` |

Example:

```mana
impl Add for Point {
    fn add(self, other: Point) -> Point {
        return Point {
            x: self.x + other.x,
            y: self.y + other.y,
        };
    }
}

let p3 = p1 + p2;  // Calls Point::add
```

---

## Appendix A: Grammar Summary

```ebnf
module      = "module" path ";" { import } { declaration }
path        = identifier { "::" identifier }
import      = "import" path ";"
            | "import" string_literal ";"
use         = "use" path [ "::" ( "*" | "{" ident_list "}" ) ] [ "as" identifier ] ";"

declaration = function_decl
            | struct_decl
            | enum_decl
            | trait_decl
            | impl_block
            | type_alias
            | const_decl
            | static_decl

function_decl = [ "pub" ] [ "async" ] "fn" identifier
                [ "<" type_params ">" ] "(" params ")" [ "->" type ]
                [ where_clause ] block

struct_decl = [ "pub" ] "struct" identifier [ "<" type_params ">" ]
              "{" field_list "}"

enum_decl = [ "pub" ] "enum" identifier [ "<" type_params ">" ]
            "{" variant_list "}"

trait_decl = [ "pub" ] "trait" identifier [ "<" type_params ">" ] "{" trait_items "}"

impl_block = "impl" [ "<" type_params ">" ] [ trait "for" ] type
             [ where_clause ] "{" impl_items "}"

const_decl = "const" identifier ":" type "=" expr ";"

static_decl = "static" [ "mut" ] identifier ":" type "=" expr ";"

statement = let_stmt | assign_stmt | expr_stmt | if_stmt | while_stmt
          | for_stmt | loop_stmt | match_stmt | return_stmt | break_stmt
          | continue_stmt | defer_stmt | block

expression = literal | identifier | unary_expr | binary_expr | call_expr
           | index_expr | member_expr | if_expr | match_expr | closure_expr
           | struct_expr | array_expr | tuple_expr | range_expr | cast_expr
```

---

## Appendix B: Standard Library Types

### Vec<T>
Dynamic array.

```mana
let v: Vec<i32> = vec![];
v.push(1);
v.pop();
v.len();
v.is_empty();
v.clear();
```

### HashMap<K, V>
Hash map collection.

```mana
let m: HashMap<string, i32> = HashMap::new();
m.insert("key", 42);
m.get("key");
m.remove("key");
m.contains_key("key");
```

### String
UTF-8 string type.

```mana
let s = String::new();
s.push_str("hello");
s.len();
s.is_empty();
s.trim();
s.split(",");
s.to_upper();
s.to_lower();
```

### Option<T>
Optional value.

```mana
some(value)     // Contains value
none            // No value

option.is_some()
option.is_none()
option.unwrap()
option.unwrap_or(default)
option.map(|x| x * 2)
```

### Result<T, E>
Success or error.

```mana
ok(value)       // Success
err(error)      // Failure

result.is_ok()
result.is_err()
result.unwrap()
result.unwrap_or(default)
result.map(|x| x * 2)
result.map_err(|e| new_error)
```

---

*This reference is for Mana v1.0.0*
