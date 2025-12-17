# Mana Language Specification v1.0

## Table of Contents

1. [Introduction](#1-introduction)
2. [Lexical Elements](#2-lexical-elements)
3. [Types](#3-types)
4. [Expressions](#4-expressions)
5. [Statements](#5-statements)
6. [Functions](#6-functions)
7. [Structs and Enums](#7-structs-and-enums)
8. [Traits](#8-traits)
9. [Pattern Matching](#9-pattern-matching)
10. [Modules and Imports](#10-modules-and-imports)
11. [Error Handling](#11-error-handling)
12. [Memory Management](#12-memory-management)
13. [Standard Library](#13-standard-library)
14. [Async Programming](#14-async-programming)
15. [Interoperability](#15-interoperability)

---

## 1. Introduction

Mana is a modern systems programming language designed for game engines and performance-critical applications. It compiles to C++ for maximum portability and performance.

### 1.1 Design Goals

- **Zero-cost abstractions**: High-level features compile to efficient code
- **Memory safety**: Ownership and borrowing prevent common memory errors
- **Expressiveness**: Modern syntax with pattern matching, traits, and generics
- **C++ interoperability**: Seamless integration with existing C++ codebases
- **Fast compilation**: Incremental compilation and parallel builds

### 1.2 Hello World

```mana
module main;

fn main() -> i32 {
    println("Hello, Mana!");
    return 0;
}
```

---

## 2. Lexical Elements

### 2.1 Keywords

```
module    import    export    pub       fn        return
struct    enum      trait     impl      generic   where
let       mut       const     if        else      match
while     for       in        break     continue  loop
true      false     none      self      Self      async
await     try       ref       ptr       borrow    unsafe
defer     scope     test      type      as        is
```

### 2.2 Identifiers

Identifiers start with a letter or underscore, followed by letters, digits, or underscores:

```
identifier = [a-zA-Z_][a-zA-Z0-9_]*
```

### 2.3 Literals

#### Integer Literals

```mana
let decimal = 42;
let hex = 0xFF;
let octal = 0o77;
let binary = 0b1010;
let with_sep = 1_000_000;
```

#### Floating-Point Literals

```mana
let f1 = 3.14;
let f2 = 1.0e-10;
let f3 = 2.5E+3;
```

#### String Literals

```mana
let simple = "hello";
let escaped = "line1\nline2";
let raw = r#"no \n escaping"#;
let multiline = """
    multi-line
    string
""";
```

#### Character Literals

```mana
let ch = 'a';
let newline = '\n';
let unicode = '\u{1F600}';
```

### 2.4 Comments

```mana
// Single-line comment

/* Multi-line
   comment */

/// Documentation comment for items
```

### 2.5 Operators

```
+   -   *   /   %   **          // Arithmetic
==  !=  <   >   <=  >=          // Comparison
&&  ||  !                        // Logical
&   |   ^   ~   <<  >>          // Bitwise
=   +=  -=  *=  /=  %=          // Assignment
..  ..=                          // Range
?.  ??                           // Optional chaining/coalescing
->  =>                           // Arrow operators
```

---

## 3. Types

### 3.1 Primitive Types

| Type | Description | Size |
|------|-------------|------|
| `i8`, `i16`, `i32`, `i64` | Signed integers | 1, 2, 4, 8 bytes |
| `u8`, `u16`, `u32`, `u64` | Unsigned integers | 1, 2, 4, 8 bytes |
| `f32`, `f64` | Floating-point | 4, 8 bytes |
| `bool` | Boolean | 1 byte |
| `char` | Unicode character | 4 bytes |
| `str` | String slice | pointer + length |

### 3.2 Compound Types

```mana
// Arrays
let arr: [i32; 5] = [1, 2, 3, 4, 5];
let filled: [i32; 10] = [0; 10];  // Array fill syntax

// Tuples
let tuple: (i32, str, bool) = (42, "hello", true);
let (x, y, z) = tuple;  // Destructuring

// Vectors
let vec: Vec<i32> = Vec::new();
let vec2 = vec![1, 2, 3];

// HashMaps
let map: HashMap<str, i32> = HashMap::new();
```

### 3.3 Optional Types

```mana
let maybe: Option<i32> = some(42);
let nothing: Option<i32> = none;

// Optional chaining
let value = obj?.field?.method();

// Null coalescing
let result = maybe ?? 0;
```

### 3.4 Result Types

```mana
let ok: Result<i32, str> = ok(42);
let err: Result<i32, str> = err("failed");

// Error propagation
fn read_file(path: str) -> Result<str, Error> {
    let file = File::open(path)?;
    let content = file.read_to_string()?;
    return ok(content);
}
```

### 3.5 Type Aliases

```mana
type Point = (f32, f32);
type Callback = fn(i32) -> bool;
type StringResult = Result<str, Error>;
```

---

## 4. Expressions

### 4.1 Arithmetic Expressions

```mana
let sum = a + b;
let diff = a - b;
let prod = a * b;
let quot = a / b;
let rem = a % b;
let power = a ** 2;  // Exponentiation
```

### 4.2 Comparison Expressions

```mana
let eq = a == b;
let ne = a != b;
let lt = a < b;
let gt = a > b;
let le = a <= b;
let ge = a >= b;
```

### 4.3 Logical Expressions

```mana
let and = a && b;
let or = a || b;
let not = !a;
```

### 4.4 If Expressions

```mana
let max = if a > b { a } else { b };
```

### 4.5 Match Expressions

```mana
let result = match value {
    0 => "zero",
    1 | 2 => "one or two",
    3..=9 => "three to nine",
    n if n < 0 => "negative",
    _ => "other"
};
```

### 4.6 Range Expressions

```mana
let inclusive = 1..=10;   // 1 to 10 inclusive
let exclusive = 1..10;    // 1 to 9
let from = 5..;           // 5 to infinity
let to = ..10;            // 0 to 9
```

### 4.7 Lambda Expressions

```mana
let add = |a, b| a + b;
let square = |x: i32| -> i32 { x * x };
let closure = |x| x + captured_var;  // Captures environment
```

### 4.8 Call Expressions

```mana
// Regular call
let result = func(arg1, arg2);

// Named arguments
let point = Point::new(x: 10, y: 20);

// Method call
let len = string.len();
```

### 4.9 Cast Expressions

```mana
let i = 42;
let f = i as f64;
let u = i as u32;
```

---

## 5. Statements

### 5.1 Variable Declarations

```mana
let immutable = 42;
let mut mutable = 0;
let typed: i32 = 100;
const CONSTANT: i32 = 1000;
```

### 5.2 Assignment

```mana
x = 10;
x += 5;
x -= 3;
x *= 2;
x /= 4;
```

### 5.3 If Statement

```mana
if condition {
    // then branch
} else if other_condition {
    // else-if branch
} else {
    // else branch
}
```

### 5.4 If-Let Statement

```mana
if let some(x) = optional {
    println(f"Got value: {x}");
}

if let ok(value) = result {
    use(value);
} else {
    handle_error();
}
```

### 5.5 Match Statement

```mana
match expr {
    Pattern1 => statement1,
    Pattern2 => {
        multiple_statements();
    },
    _ => default_case
}
```

### 5.6 While Loop

```mana
while condition {
    // body
}

// While-let
while let some(item) = iterator.next() {
    process(item);
}
```

### 5.7 For Loop

```mana
for i in 0..10 {
    println(i);
}

for item in collection {
    process(item);
}

for (index, value) in collection.enumerate() {
    println(f"{index}: {value}");
}
```

### 5.8 Loop Statement

```mana
loop {
    if done {
        break;
    }
    continue;
}
```

### 5.9 Return Statement

```mana
return value;
return;  // For void functions
```

### 5.10 Break and Continue

```mana
break;        // Exit loop
continue;     // Skip to next iteration
```

### 5.11 Defer Statement

```mana
fn process_file() {
    let file = open("data.txt");
    defer file.close();

    // file.close() called automatically when function exits
}
```

---

## 6. Functions

### 6.1 Function Declaration

```mana
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

fn greet(name: str) {
    println(f"Hello, {name}!");
}
```

### 6.2 Default Parameters

```mana
fn connect(host: str, port: i32 = 8080, timeout: i32 = 30) {
    // ...
}

// Call with defaults
connect("localhost");
connect("localhost", 9000);
connect("localhost", port: 9000, timeout: 60);
```

### 6.3 Named Arguments

```mana
fn create_window(title: str, width: i32, height: i32, fullscreen: bool) {
    // ...
}

create_window(
    title: "My App",
    width: 800,
    height: 600,
    fullscreen: false
);
```

### 6.4 Generic Functions

```mana
generic<T>
fn identity(x: T) -> T {
    return x;
}

generic<T, U>
fn pair(first: T, second: U) -> (T, U) {
    return (first, second);
}
```

### 6.5 Constrained Generics

```mana
generic<T> where T: Display + Clone
fn print_twice(x: T) {
    println(x.to_string());
    let copy = x.clone();
    println(copy.to_string());
}
```

### 6.6 Test Functions

```mana
#[test]
fn test_addition() {
    assert(1 + 1 == 2);
    assert_eq(2 + 2, 4);
}
```

---

## 7. Structs and Enums

### 7.1 Struct Declaration

```mana
struct Point {
    x: f32,
    y: f32,
}

pub struct Player {
    pub name: str,
    health: i32,
    position: Point,
}
```

### 7.2 Struct with Defaults

```mana
struct Config {
    host: str = "localhost",
    port: i32 = 8080,
    debug: bool = false,
}

let config = Config { debug: true };  // Uses defaults for host and port
```

### 7.3 Struct Methods (impl blocks)

```mana
impl Point {
    fn new(x: f32, y: f32) -> Point {
        return Point { x, y };
    }

    fn distance(self, other: Point) -> f32 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        return (dx*dx + dy*dy).sqrt();
    }

    fn translate(mut self, dx: f32, dy: f32) {
        self.x += dx;
        self.y += dy;
    }
}
```

### 7.4 Static Methods

```mana
impl Math {
    fn pi() -> f64 {
        return 3.14159265358979;
    }

    fn max(a: i32, b: i32) -> i32 {
        return if a > b { a } else { b };
    }
}

let pi = Math::pi();
```

### 7.5 Enum Declaration

```mana
enum Direction {
    North,
    South,
    East,
    West,
}
```

### 7.6 Algebraic Data Types (ADT Enums)

```mana
enum Option<T> {
    Some(T),
    None,
}

enum Result<T, E> {
    Ok(T),
    Err(E),
}

enum Message {
    Quit,
    Move { x: i32, y: i32 },
    Write(str),
    Color(i32, i32, i32),
}
```

---

## 8. Traits

### 8.1 Trait Declaration

```mana
trait Display {
    fn to_string(self) -> str;
}

trait Clone {
    fn clone(self) -> Self;
}
```

### 8.2 Trait with Default Methods

```mana
trait Iterator<T> {
    fn next(mut self) -> Option<T>;

    fn count(mut self) -> i32 {
        let mut c = 0;
        while let some(_) = self.next() {
            c += 1;
        }
        return c;
    }
}
```

### 8.3 Trait Implementation

```mana
impl Display for Point {
    fn to_string(self) -> str {
        return f"({self.x}, {self.y})";
    }
}
```

### 8.4 Associated Types

```mana
trait Container {
    type Item;

    fn get(self, index: i32) -> Self::Item;
    fn len(self) -> i32;
}

impl Container for Vec<i32> {
    type Item = i32;

    fn get(self, index: i32) -> i32 {
        return self[index];
    }

    fn len(self) -> i32 {
        return self.length();
    }
}
```

### 8.5 Operator Overloading

```mana
trait Add<Rhs = Self> {
    type Output;
    fn add(self, rhs: Rhs) -> Self::Output;
}

impl Add for Point {
    type Output = Point;

    fn add(self, rhs: Point) -> Point {
        return Point {
            x: self.x + rhs.x,
            y: self.y + rhs.y
        };
    }
}

let p3 = p1 + p2;  // Uses Add::add
```

---

## 9. Pattern Matching

### 9.1 Literal Patterns

```mana
match x {
    0 => "zero",
    1 => "one",
    _ => "other"
}
```

### 9.2 Variable Binding

```mana
match value {
    n => println(f"Got {n}")
}
```

### 9.3 Tuple Patterns

```mana
match point {
    (0, 0) => "origin",
    (x, 0) => f"on x-axis at {x}",
    (0, y) => f"on y-axis at {y}",
    (x, y) => f"at ({x}, {y})"
}
```

### 9.4 Struct Patterns

```mana
match player {
    Player { health: 0, .. } => "dead",
    Player { health: h, .. } if h < 20 => "critical",
    Player { name, health, .. } => f"{name}: {health}hp"
}
```

### 9.5 Enum Patterns

```mana
match option {
    some(x) => use(x),
    none => default()
}

match result {
    ok(value) => success(value),
    err(e) => handle_error(e)
}
```

### 9.6 Range Patterns

```mana
match score {
    0..=59 => "F",
    60..=69 => "D",
    70..=79 => "C",
    80..=89 => "B",
    90..=100 => "A",
    _ => "invalid"
}
```

### 9.7 Or Patterns

```mana
match direction {
    North | South => "vertical",
    East | West => "horizontal"
}
```

### 9.8 Guards

```mana
match value {
    n if n < 0 => "negative",
    n if n > 0 => "positive",
    _ => "zero"
}
```

---

## 10. Modules and Imports

### 10.1 Module Declaration

```mana
module math;

pub fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

fn private_helper() {
    // Not exported
}
```

### 10.2 Import Statement

```mana
import math;
import std::collections::HashMap;
import graphics::{Window, Canvas, Color};
```

### 10.3 Visibility

- `pub` - Public, accessible from other modules
- No modifier - Private to the current module

```mana
pub struct PublicStruct {
    pub field1: i32,    // Public field
    field2: i32,        // Private field
}
```

### 10.4 Multi-File Projects

```mana
// lib.mana
module mylib;

pub fn exported_function() { }

// main.mana
module main;
import mylib;

fn main() -> i32 {
    mylib::exported_function();
    return 0;
}
```

---

## 11. Error Handling

### 11.1 Result Type

```mana
fn divide(a: f64, b: f64) -> Result<f64, str> {
    if b == 0.0 {
        return err("division by zero");
    }
    return ok(a / b);
}
```

### 11.2 Error Propagation

```mana
fn process() -> Result<i32, Error> {
    let value = get_value()?;  // Propagates error
    let result = compute(value)?;
    return ok(result);
}
```

### 11.3 Try Expression

```mana
let result = try {
    let file = File::open("data.txt")?;
    let content = file.read_to_string()?;
    parse(content)?
};
```

### 11.4 Assert

```mana
assert(condition);
assert(condition, "custom message");
assert_eq(a, b);
assert_ne(a, b);
```

---

## 12. Memory Management

### 12.1 Ownership

Each value has a single owner. When the owner goes out of scope, the value is dropped.

```mana
fn example() {
    let s = String::from("hello");  // s owns the string
    take_ownership(s);               // Ownership moved to function
    // s is no longer valid here
}
```

### 12.2 References

```mana
fn example() {
    let s = String::from("hello");
    let len = calculate_length(&s);  // Borrow s
    println(s);  // s is still valid
}

fn calculate_length(s: &str) -> i32 {
    return s.len();
}
```

### 12.3 Mutable References

```mana
fn example() {
    let mut s = String::from("hello");
    change(&mut s);
}

fn change(s: &mut str) {
    s.push_str(", world");
}
```

### 12.4 Defer for Cleanup

```mana
fn process_file(path: str) -> Result<(), Error> {
    let file = File::open(path)?;
    defer file.close();

    let buffer = allocate(1024);
    defer free(buffer);

    // Work with file and buffer
    // Cleanup happens automatically in reverse order
    return ok(());
}
```

---

## 13. Standard Library

### 13.1 String Operations

```mana
let s = "hello world";
let len = s.len();
let upper = s.to_upper();
let lower = s.to_lower();
let trimmed = s.trim();
let contains = s.contains("world");
let replaced = s.replace("world", "mana");
let parts = s.split(" ");
let starts = s.starts_with("hello");
let ends = s.ends_with("world");
let sub = s.substring(0, 5);
```

### 13.2 Vector Operations

```mana
let mut v = vec![1, 2, 3];
v.push(4);
let popped = v.pop();
let first = v[0];
let len = v.len();
let empty = v.is_empty();
v.clear();
v.insert(0, 10);
v.remove(1);
let reversed = v.reverse();
let sorted = v.sort();
```

### 13.3 HashMap Operations

```mana
let mut map = HashMap::new();
map.insert("key", 42);
let value = map.get("key");
let has = map.contains_key("key");
map.remove("key");
let len = map.len();
let keys = map.keys();
let values = map.values();
```

### 13.4 Iterator Methods

```mana
let numbers = vec![1, 2, 3, 4, 5];

let doubled = numbers.map(|x| x * 2);
let evens = numbers.filter(|x| x % 2 == 0);
let sum = numbers.fold(0, |acc, x| acc + x);
let found = numbers.find(|x| x > 3);
let any_big = numbers.any(|x| x > 10);
let all_pos = numbers.all(|x| x > 0);
```

### 13.5 File I/O

```mana
// Reading
let content = read_file("data.txt")?;
let lines = read_lines("data.txt")?;
let exists = file_exists("data.txt");

// Writing
write_file("output.txt", content)?;
append_file("log.txt", message)?;

// File operations
let file = File::open("data.txt")?;
let content = file.read_to_string()?;
file.close();
```

### 13.6 Math Functions

```mana
let abs_val = abs(-42);
let minimum = min(a, b);
let maximum = max(a, b);
let clamped = clamp(value, 0, 100);
let sq = sqrt(16.0);
let sn = sin(angle);
let cs = cos(angle);
let tn = tan(angle);
let pw = pow(2.0, 10.0);
let lg = log(100.0);
let fl = floor(3.7);
let cl = ceil(3.2);
let rn = round(3.5);
```

### 13.7 Random

```mana
let r = random();           // 0.0 to 1.0
let n = random_int(1, 100); // 1 to 100
```

### 13.8 Time

```mana
let now = time_now();       // Unix timestamp
sleep(1000);                // Sleep 1 second
```

---

## 14. Async Programming

### 14.1 Async Functions

```mana
async fn fetch_data(url: str) -> Result<str, Error> {
    let response = await http_get(url)?;
    return ok(response.body);
}
```

### 14.2 Await Expression

```mana
async fn process() {
    let data = await fetch_data("https://api.example.com")?;
    let parsed = parse(data)?;
    await save_to_db(parsed)?;
}
```

---

## 15. Interoperability

### 15.1 C++ Integration

Mana compiles to C++ and can interoperate with C++ code:

```mana
// Declare external C++ function
extern fn cpp_function(x: i32) -> i32;

// Use in Mana
fn main() -> i32 {
    let result = cpp_function(42);
    return result;
}
```

### 15.2 Generated C++ Code

Mana generates clean, readable C++ code:

```mana
// Mana
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}
```

```cpp
// Generated C++
int32_t add(int32_t a, int32_t b) {
    return a + b;
}
```

---

## Appendix A: Grammar Summary

```ebnf
program         = { module_decl | import_decl | declaration } ;
module_decl     = "module" identifier ";" ;
import_decl     = "import" import_path ";" ;
declaration     = func_decl | struct_decl | enum_decl | trait_decl | impl_decl ;

func_decl       = [ "pub" ] [ "async" ] [ generics ] "fn" identifier
                  "(" [ params ] ")" [ "->" type ] [ where_clause ] block ;

struct_decl     = [ "pub" ] "struct" identifier [ generics ] "{" { field } "}" ;
enum_decl       = [ "pub" ] "enum" identifier [ generics ] "{" { variant } "}" ;
trait_decl      = [ "pub" ] "trait" identifier [ generics ] "{" { trait_item } "}" ;
impl_decl       = "impl" [ generics ] type [ "for" type ] "{" { impl_item } "}" ;

statement       = var_decl | assignment | expr_stmt | if_stmt | match_stmt
                | while_stmt | for_stmt | return_stmt | break_stmt | continue_stmt ;

expression      = literal | identifier | binary_expr | unary_expr | call_expr
                | field_expr | index_expr | lambda_expr | if_expr | match_expr ;
```

---

## Appendix B: Operator Precedence

| Precedence | Operators | Associativity |
|------------|-----------|---------------|
| 1 (lowest) | `=` `+=` `-=` `*=` `/=` | Right |
| 2 | `||` | Left |
| 3 | `&&` | Left |
| 4 | `==` `!=` | Left |
| 5 | `<` `>` `<=` `>=` | Left |
| 6 | `|` | Left |
| 7 | `^` | Left |
| 8 | `&` | Left |
| 9 | `<<` `>>` | Left |
| 10 | `+` `-` | Left |
| 11 | `*` `/` `%` | Left |
| 12 | `**` | Right |
| 13 (highest) | `!` `-` (unary) | Right |

---

## Appendix C: Reserved for Future Use

The following keywords are reserved for potential future features:

```
macro    derive    move    copy    box    dyn
yield    union     static  virtual final  override
```

---

*Document Version: 1.0*
*Last Updated: December 2025*
