# Mana Language Tutorial

Welcome to Mana! This tutorial will guide you through the language from basics to advanced features.

## Getting Started

### Installation

Build the Mana compiler from source:

```bash
cd mana-lang
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Your First Program

Create a file `hello.mana`:

```mana
module main;

fn main() -> i32 {
    println("Hello, Mana!");
    return 0;
}
```

Compile and run:

```bash
mana_lang hello.mana --emit-cpp
cd build && cmake .. && cmake --build . --config Release
./hello
```

---

## Chapter 1: Basics

### 1.1 Variables

```mana
module main;

fn main() -> i32 {
    // Immutable by default
    let x = 42;
    let name = "Alice";

    // Mutable variables use 'mut'
    let mut counter = 0;
    counter = counter + 1;

    // Type annotations (optional with inference)
    let pi: f64 = 3.14159;
    let flag: bool = true;

    println("x = ", x, ", counter = ", counter);
    return 0;
}
```

### 1.2 Data Types

```mana
module main;

fn main() -> i32 {
    // Integers
    let i: i32 = 42;
    let big: i64 = 9999999999;
    let unsigned: u32 = 100;

    // Floating point
    let f: f32 = 3.14;
    let d: f64 = 2.71828;

    // Boolean
    let yes: bool = true;
    let no: bool = false;

    // Character
    let ch: char = 'A';

    // String
    let greeting: str = "Hello";

    // Arrays
    let nums: [i32; 5] = [1, 2, 3, 4, 5];
    let zeros: [i32; 10] = [0; 10];  // Fill with zeros

    return 0;
}
```

### 1.3 Functions

```mana
module main;

// Basic function
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

// Void function (no return value)
fn greet(name: str) {
    println("Hello, ", name, "!");
}

// Default parameters
fn connect(host: str, port: i32 = 8080) {
    println("Connecting to ", host, ":", port);
}

fn main() -> i32 {
    let sum = add(3, 4);
    println("3 + 4 = ", sum);

    greet("World");

    connect("localhost");        // Uses default port
    connect("example.com", 443); // Custom port

    return 0;
}
```

### 1.4 Control Flow

```mana
module main;

fn main() -> i32 {
    let x = 10;

    // If-else
    if x > 0 {
        println("positive");
    } else if x < 0 {
        println("negative");
    } else {
        println("zero");
    }

    // If as expression
    let abs = if x >= 0 { x } else { -x };

    // While loop
    let mut i = 0;
    while i < 5 {
        println(f"i = {i}");
        i = i + 1;
    }

    // For loop with range
    for j in 0..5 {
        println(f"j = {j}");
    }

    // Loop with break
    let mut count = 0;
    loop {
        count = count + 1;
        if count >= 3 {
            break;
        }
    }

    return 0;
}
```

---

## Chapter 2: Structured Data

### 2.1 Structs

```mana
module main;

struct Point {
    x: f32,
    y: f32,
}

struct Player {
    name: str,
    health: i32,
    position: Point,
}

fn main() -> i32 {
    // Create struct instances
    let origin = Point { x: 0.0, y: 0.0 };
    let p = Point { x: 3.0, y: 4.0 };

    // Access fields
    println(f"Point: ({p.x}, {p.y})");

    // Nested structs
    let player = Player {
        name: "Hero",
        health: 100,
        position: Point { x: 10.0, y: 20.0 }
    };

    println(f"{player.name} at ({player.position.x}, {player.position.y})");

    return 0;
}
```

### 2.2 Methods (impl blocks)

```mana
module main;

struct Rectangle {
    width: f32,
    height: f32,
}

impl Rectangle {
    // Constructor (associated function)
    fn new(w: f32, h: f32) -> Rectangle {
        return Rectangle { width: w, height: h };
    }

    // Method (takes self)
    fn area(self) -> f32 {
        return self.width * self.height;
    }

    fn perimeter(self) -> f32 {
        return 2.0 * (self.width + self.height);
    }

    // Mutable method
    fn scale(mut self, factor: f32) {
        self.width = self.width * factor;
        self.height = self.height * factor;
    }
}

fn main() -> i32 {
    let rect = Rectangle::new(10.0, 5.0);
    println(f"Area: {rect.area()}");
    println(f"Perimeter: {rect.perimeter()}");

    let mut r2 = Rectangle::new(3.0, 4.0);
    r2.scale(2.0);
    println(f"Scaled area: {r2.area()}");

    return 0;
}
```

### 2.3 Enums

```mana
module main;

// Simple enum
enum Direction {
    North,
    South,
    East,
    West,
}

// Enum with data (Algebraic Data Type)
enum Shape {
    Circle(f32),              // radius
    Rectangle(f32, f32),      // width, height
    Point,
}

fn describe_shape(s: Shape) -> str {
    return match s {
        Shape::Circle(r) => f"Circle with radius {r}",
        Shape::Rectangle(w, h) => f"Rectangle {w}x{h}",
        Shape::Point => "A point"
    };
}

fn main() -> i32 {
    let dir = Direction::North;
    let circle = Shape::Circle(5.0);
    let rect = Shape::Rectangle(10.0, 20.0);

    println(describe_shape(circle));
    println(describe_shape(rect));

    return 0;
}
```

---

## Chapter 3: Collections

### 3.1 Vectors

```mana
module main;

fn main() -> i32 {
    // Create vector
    let mut numbers = vec![1, 2, 3, 4, 5];

    // Add elements
    numbers.push(6);
    numbers.push(7);

    // Access elements
    let first = numbers[0];
    let len = numbers.len();
    println(f"First: {first}, Length: {len}");

    // Iterate
    for num in numbers {
        println(f"Number: {num}");
    }

    // Vector methods
    let mut v = vec![3, 1, 4, 1, 5];
    v.sort();
    println(f"Sorted: {v}");

    return 0;
}
```

### 3.2 HashMaps

```mana
module main;

fn main() -> i32 {
    let mut scores = HashMap::new();

    // Insert
    scores.insert("Alice", 95);
    scores.insert("Bob", 87);
    scores.insert("Charlie", 92);

    // Lookup
    if let some(score) = scores.get("Alice") {
        println(f"Alice's score: {score}");
    }

    // Check existence
    if scores.contains_key("Bob") {
        println("Bob is in the map");
    }

    // Iterate
    for (name, score) in scores {
        println(f"{name}: {score}");
    }

    return 0;
}
```

### 3.3 Tuples

```mana
module main;

fn divide(a: i32, b: i32) -> (i32, i32) {
    return (a / b, a % b);  // quotient and remainder
}

fn main() -> i32 {
    // Create tuple
    let point = (10, 20, 30);

    // Access by index
    let x = point.0;
    let y = point.1;

    // Destructuring
    let (a, b, c) = point;
    println(f"a={a}, b={b}, c={c}");

    // Function returning tuple
    let (quot, rem) = divide(17, 5);
    println(f"17 / 5 = {quot} remainder {rem}");

    return 0;
}
```

---

## Chapter 4: Pattern Matching

### 4.1 Match Expressions

```mana
module main;

fn main() -> i32 {
    let x = 5;

    // Basic match
    let result = match x {
        0 => "zero",
        1 => "one",
        2 | 3 => "two or three",
        4..=10 => "four to ten",
        _ => "something else"
    };
    println(result);

    // Match with guards
    let score = 85;
    let grade = match score {
        s if s >= 90 => "A",
        s if s >= 80 => "B",
        s if s >= 70 => "C",
        s if s >= 60 => "D",
        _ => "F"
    };
    println(f"Grade: {grade}");

    return 0;
}
```

### 4.2 Destructuring

```mana
module main;

struct Point { x: i32, y: i32 }

fn main() -> i32 {
    // Tuple destructuring
    let (a, b) = (1, 2);

    // Struct destructuring
    let p = Point { x: 10, y: 20 };
    let Point { x, y } = p;
    println(f"x={x}, y={y}");

    // In match
    let point = Point { x: 0, y: 5 };
    match point {
        Point { x: 0, y } => println(f"On y-axis at {y}"),
        Point { x, y: 0 } => println(f"On x-axis at {x}"),
        Point { x, y } => println(f"At ({x}, {y})")
    }

    return 0;
}
```

### 4.3 Option and Result

```mana
module main;

fn find_user(id: i32) -> Option<str> {
    if id == 1 {
        return some("Alice");
    } else if id == 2 {
        return some("Bob");
    }
    return none;
}

fn divide(a: f64, b: f64) -> Result<f64, str> {
    if b == 0.0 {
        return err("division by zero");
    }
    return ok(a / b);
}

fn main() -> i32 {
    // Option handling
    match find_user(1) {
        some(name) => println(f"Found: {name}"),
        none => println("User not found")
    }

    // If-let for simpler cases
    if let some(name) = find_user(2) {
        println(f"User 2 is {name}");
    }

    // Result handling
    match divide(10.0, 2.0) {
        ok(result) => println(f"Result: {result}"),
        err(msg) => println(f"Error: {msg}")
    }

    return 0;
}
```

---

## Chapter 5: Generics and Traits

### 5.1 Generic Functions

```mana
module main;

generic<T>
fn identity(x: T) -> T {
    return x;
}

generic<T>
fn swap(a: T, b: T) -> (T, T) {
    return (b, a);
}

fn main() -> i32 {
    let x = identity(42);
    let s = identity("hello");

    let (a, b) = swap(1, 2);
    println(f"Swapped: {a}, {b}");

    return 0;
}
```

### 5.2 Generic Structs

```mana
module main;

generic<T>
struct Pair {
    first: T,
    second: T,
}

generic<T>
impl Pair<T> {
    fn new(a: T, b: T) -> Pair<T> {
        return Pair { first: a, second: b };
    }

    fn swap(self) -> Pair<T> {
        return Pair { first: self.second, second: self.first };
    }
}

fn main() -> i32 {
    let p = Pair::new(1, 2);
    println(f"({p.first}, {p.second})");

    let swapped = p.swap();
    println(f"Swapped: ({swapped.first}, {swapped.second})");

    return 0;
}
```

### 5.3 Traits

```mana
module main;

trait Printable {
    fn print(self);
}

trait Describable {
    fn describe(self) -> str;
}

struct Dog {
    name: str,
    age: i32,
}

impl Printable for Dog {
    fn print(self) {
        println(f"Dog: {self.name}");
    }
}

impl Describable for Dog {
    fn describe(self) -> str {
        return f"{self.name}, {self.age} years old";
    }
}

fn main() -> i32 {
    let dog = Dog { name: "Buddy", age: 3 };
    dog.print();
    println(dog.describe());

    return 0;
}
```

### 5.4 Trait Bounds

```mana
module main;

trait Display {
    fn to_string(self) -> str;
}

generic<T> where T: Display
fn print_item(item: T) {
    println(item.to_string());
}

fn main() -> i32 {
    // Works with any type implementing Display
    return 0;
}
```

---

## Chapter 6: Error Handling

### 6.1 Result Type

```mana
module main;

fn read_number(s: str) -> Result<i32, str> {
    // Simplified - in reality would parse
    if s == "42" {
        return ok(42);
    }
    return err(f"Cannot parse '{s}' as number");
}

fn main() -> i32 {
    let result = read_number("42");

    match result {
        ok(n) => println(f"Got number: {n}"),
        err(e) => println(f"Error: {e}")
    }

    return 0;
}
```

### 6.2 Error Propagation (?)

```mana
module main;

fn get_config_value() -> Result<i32, str> {
    // ... might fail
    return ok(100);
}

fn calculate() -> Result<i32, str> {
    let value = get_config_value()?;  // Propagates error if any
    return ok(value * 2);
}

fn main() -> i32 {
    match calculate() {
        ok(result) => println(f"Result: {result}"),
        err(e) => println(f"Failed: {e}")
    }
    return 0;
}
```

### 6.3 Assertions

```mana
module main;

fn main() -> i32 {
    let x = 5;

    assert(x > 0);
    assert(x > 0, "x must be positive");

    assert_eq(2 + 2, 4);
    assert_ne(1, 2);

    println("All assertions passed!");
    return 0;
}
```

---

## Chapter 7: Iterators

### 7.1 Basic Iteration

```mana
module main;

fn main() -> i32 {
    let numbers = vec![1, 2, 3, 4, 5];

    // For loop
    for n in numbers {
        println(f"{n}");
    }

    // Range iteration
    for i in 0..5 {
        println(f"Index: {i}");
    }

    // With enumerate
    let words = vec!["hello", "world"];
    for (i, word) in words.enumerate() {
        println(f"{i}: {word}");
    }

    return 0;
}
```

### 7.2 Iterator Methods

```mana
module main;

fn main() -> i32 {
    let numbers = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10];

    // Map
    let doubled = numbers.map(|x| x * 2);

    // Filter
    let evens = numbers.filter(|x| x % 2 == 0);

    // Fold (reduce)
    let sum = numbers.fold(0, |acc, x| acc + x);
    println(f"Sum: {sum}");

    // Find
    if let some(n) = numbers.find(|x| x > 5) {
        println(f"First > 5: {n}");
    }

    // All/Any
    let all_positive = numbers.all(|x| x > 0);
    let has_ten = numbers.any(|x| x == 10);

    return 0;
}
```

---

## Chapter 8: Closures

### 8.1 Basic Closures

```mana
module main;

fn main() -> i32 {
    // Simple closure
    let add = |a, b| a + b;
    println(f"3 + 4 = {add(3, 4)}");

    // With type annotations
    let multiply = |x: i32, y: i32| -> i32 { x * y };

    // Multi-statement closure
    let process = |x| {
        let doubled = x * 2;
        let squared = doubled * doubled;
        return squared;
    };

    println(f"process(3) = {process(3)}");

    return 0;
}
```

### 8.2 Capturing Variables

```mana
module main;

fn main() -> i32 {
    let factor = 10;

    // Closure captures 'factor' from environment
    let scale = |x| x * factor;

    println(f"scale(5) = {scale(5)}");

    return 0;
}
```

---

## Chapter 9: Modules

### 9.1 Single File Modules

```mana
// math.mana
module math;

pub fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

pub fn multiply(a: i32, b: i32) -> i32 {
    return a * b;
}

fn internal_helper() {
    // Not exported (no 'pub')
}
```

### 9.2 Importing Modules

```mana
// main.mana
module main;

import math;

fn main() -> i32 {
    let sum = math::add(3, 4);
    let product = math::multiply(5, 6);

    println(f"Sum: {sum}");
    println(f"Product: {product}");

    return 0;
}
```

### 9.3 Visibility

```mana
module example;

// Public struct
pub struct Point {
    pub x: f32,   // Public field
    pub y: f32,   // Public field
    id: i32,      // Private field
}

// Public function
pub fn create_point(x: f32, y: f32) -> Point {
    return Point { x: x, y: y, id: next_id() };
}

// Private function
fn next_id() -> i32 {
    // ...
    return 0;
}
```

---

## Chapter 10: Testing

### 10.1 Writing Tests

```mana
module math_tests;

fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

#[test]
fn test_add_positive() {
    assert_eq(add(2, 3), 5);
}

#[test]
fn test_add_negative() {
    assert_eq(add(-1, 1), 0);
}

#[test]
fn test_add_zero() {
    assert_eq(add(5, 0), 5);
}

fn main() -> i32 {
    return 0;
}
```

### 10.2 Running Tests

```bash
mana_lang test_file.mana --test -c
./test_file
```

---

## Appendix: Quick Reference

### Type Conversion

```mana
let i = 42;
let f = i as f64;      // i32 to f64
let u = i as u32;      // i32 to u32
let c = 65 as char;    // i32 to char
```

### String Formatting

```mana
let name = "World";
let count = 42;

// Variadic print - pass multiple arguments
println("Hello, ", name, "!");
println("Count: ", count);

// Multiple values in one call
println("a = ", a, ", b = ", b, ", sum = ", a + b);

// String concatenation
let msg = "Hello, " + name + "!";
let info = "Count: " + to_string(count);
```

### Common Operations

```mana
// Math
let abs_val = abs(-5);
let max_val = max(a, b);
let min_val = min(a, b);
let sqrt_val = sqrt(16.0);

// Strings
let len = str.len();
let upper = str.to_upper();
let trimmed = str.trim();
let parts = str.split(",");

// Vectors
let len = vec.len();
vec.push(item);
let item = vec.pop();
vec.sort();
```

---

*Happy coding with Mana!*
