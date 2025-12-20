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
module main

fn main() {
    println("Hello, Mana!")
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
module main

fn main() {
    // Immutable by default
    let x = 42
    let name = "Alice"

    // Mutable variables use 'mut'
    let mut counter = 0
    counter = counter + 1

    // Type annotations (optional with inference)
    let pi: f64 = 3.14159
    let flag: bool = true

    // Use 'int' and 'float' aliases for convenience
    let count: int = 100
    let temp: float = 98.6

    println("x = ", x, ", counter = ", counter)
}
```

### 1.2 Data Types

```mana
module main

fn main() {
    // Integers (int is alias for i32)
    let i: int = 42
    let big: i64 = 9999999999
    let unsigned: u32 = 100

    // Floating point (float is alias for f32)
    let f: float = 3.14
    let d: f64 = 2.71828

    // Boolean
    let yes: bool = true
    let no: bool = false

    // Character
    let ch: char = 'A'

    // String
    let greeting: string = "Hello"

    // Arrays
    let nums: [int; 5] = [1, 2, 3, 4, 5]
    let zeros: [int; 10] = [0; 10]  // Fill with zeros
}
```

### 1.3 Functions

```mana
module main

// Basic function
fn add(a: int, b: int) -> int {
    return a + b
}

// Void function (no return value)
fn greet(name: str) {
    println("Hello, ", name, "!")
}

// Default parameters
fn connect(host: str, port: int = 8080) {
    println("Connecting to ", host, ":", port)
}

fn main() {
    let sum = add(3, 4)
    println("3 + 4 = ", sum)

    greet("World")

    connect("localhost")        // Uses default port
    connect("example.com", 443) // Custom port
}
```

### 1.4 Control Flow

```mana
module main

fn main() {
    let x = 10

    // If-else
    if x > 0 {
        println("positive")
    } else if x < 0 {
        println("negative")
    } else {
        println("zero")
    }

    // If as expression
    let abs = if x >= 0 { x } else { -x }

    // While loop
    let mut i = 0
    while i < 5 {
        println("i = ", i)
        i = i + 1
    }

    // For loop with range
    for j in 0..5 {
        println("j = ", j)
    }

    // C-style for loop (semicolons required in header)
    for k = 0; k < 5; k++ {
        println("k = ", k)
    }

    // Loop with break
    let mut count = 0
    loop {
        count = count + 1
        if count >= 3 {
            break
        }
    }
}
```

---

## Chapter 2: Structured Data

### 2.1 Structs

```mana
module main

struct Point {
    x: float,
    y: float,
}

struct Player {
    name: str,
    health: int,
    position: Point,
}

fn main() {
    // Create struct instances
    let origin = Point { x: 0.0, y: 0.0 }
    let p = Point { x: 3.0, y: 4.0 }

    // Access fields
    println("Point: (", p.x, ", ", p.y, ")")

    // Nested structs
    let player = Player {
        name: "Hero",
        health: 100,
        position: Point { x: 10.0, y: 20.0 }
    }

    println(player.name, " at (", player.position.x, ", ", player.position.y, ")")
}
```

### 2.2 Methods (impl blocks)

```mana
module main

struct Rectangle {
    width: float,
    height: float,
}

impl Rectangle {
    // Constructor (associated function)
    fn new(w: float, h: float) -> Rectangle {
        return Rectangle { width: w, height: h }
    }

    // Method (takes self)
    fn area(self) -> float {
        return self.width * self.height
    }

    fn perimeter(self) -> float {
        return 2.0 * (self.width + self.height)
    }

    // Mutable method
    fn scale(mut self, factor: float) {
        self.width = self.width * factor
        self.height = self.height * factor
    }
}

fn main() {
    let rect = Rectangle::new(10.0, 5.0)
    println("Area: ", rect.area())
    println("Perimeter: ", rect.perimeter())

    let mut r2 = Rectangle::new(3.0, 4.0)
    r2.scale(2.0)
    println("Scaled area: ", r2.area())
}
```

### 2.3 Enums and Variants

```mana
module main

// Simple enum
enum Direction {
    North,
    South,
    East,
    West,
}

// 'variant' is a synonym for enum
variant Color {
    Red,
    Green,
    Blue,
}

// Enum with data (Algebraic Data Type)
enum Shape {
    Circle(float),              // radius
    Rectangle(float, float),    // width, height
    Point,
}

fn describe_shape(s: Shape) -> str {
    // Use 'when' with -> arrows (alternative to match/=>)
    return when s {
        Shape::Circle(r) -> "Circle with radius " + r
        Shape::Rectangle(w, h) -> "Rectangle " + w + "x" + h
        Shape::Point -> "A point"
    }
}

fn main() {
    let dir = Direction::North
    let color = Color::Red
    let circle = Shape::Circle(5.0)
    let rect = Shape::Rectangle(10.0, 20.0)

    println(describe_shape(circle))
    println(describe_shape(rect))
}
```

---

## Chapter 3: Collections

### 3.1 Vectors

```mana
module main

fn main() {
    // Create vector
    let mut numbers = vec![1, 2, 3, 4, 5]

    // Add elements
    numbers.push(6)
    numbers.push(7)

    // Access elements
    let first = numbers[0]
    let len = numbers.len()
    println("First: ", first, ", Length: ", len)

    // Iterate
    for num in numbers {
        println("Number: ", num)
    }

    // Vector methods
    let mut v = vec![3, 1, 4, 1, 5]
    v.sort()
    println("Sorted: ", v)
}
```

### 3.2 HashMaps

```mana
module main

fn main() {
    let mut scores = HashMap::new()

    // Insert
    scores.insert("Alice", 95)
    scores.insert("Bob", 87)
    scores.insert("Charlie", 92)

    // Lookup
    if let some(score) = scores.get("Alice") {
        println("Alice's score: ", score)
    }

    // Check existence
    if scores.contains_key("Bob") {
        println("Bob is in the map")
    }

    // Iterate
    for (name, score) in scores {
        println(name, ": ", score)
    }
}
```

### 3.3 Tuples

```mana
module main

fn divide(a: int, b: int) -> (int, int) {
    return (a / b, a % b)  // quotient and remainder
}

fn main() {
    // Create tuple
    let point = (10, 20, 30)

    // Access by index
    let x = point.0
    let y = point.1

    // Destructuring
    let (a, b, c) = point
    println("a=", a, ", b=", b, ", c=", c)

    // Function returning tuple
    let (quot, rem) = divide(17, 5)
    println("17 / 5 = ", quot, " remainder ", rem)
}
```

---

## Chapter 4: Pattern Matching

### 4.1 Match and When Expressions

```mana
module main

fn main() {
    let x = 5

    // Traditional match with =>
    let result = match x {
        0 => "zero",
        1 => "one",
        2 | 3 => "two or three",
        4..=10 => "four to ten",
        _ => "something else"
    }
    println(result)

    // Alternative: when with -> arrows
    let result2 = when x {
        0 -> "zero"
        1 -> "one"
        2 | 3 -> "two or three"
        4..=10 -> "four to ten"
        _ -> "something else"
    }

    // Match with guards
    let score = 85
    let grade = when score {
        s if s >= 90 -> "A"
        s if s >= 80 -> "B"
        s if s >= 70 -> "C"
        s if s >= 60 -> "D"
        _ -> "F"
    }
    println("Grade: ", grade)
}
```

### 4.2 Destructuring

```mana
module main

struct Point { x: int, y: int }

fn main() {
    // Tuple destructuring
    let (a, b) = (1, 2)

    // Struct destructuring
    let p = Point { x: 10, y: 20 }
    let Point { x, y } = p
    println("x=", x, ", y=", y)

    // In when expression
    let point = Point { x: 0, y: 5 }
    when point {
        Point { x: 0, y } -> println("On y-axis at ", y)
        Point { x, y: 0 } -> println("On x-axis at ", x)
        Point { x, y } -> println("At (", x, ", ", y, ")")
    }
}
```

### 4.3 Option and Result

```mana
module main

fn find_user(id: int) -> Option<str> {
    if id == 1 {
        return some("Alice")
    } else if id == 2 {
        return some("Bob")
    }
    return none
}

fn divide(a: f64, b: f64) -> Result<f64, str> {
    if b == 0.0 {
        return Err("division by zero")
    }
    return Ok(a / b)
}

fn main() {
    // Option handling with when
    when find_user(1) {
        some(name) -> println("Found: ", name)
        none -> println("User not found")
    }

    // If-let for simpler cases
    if let some(name) = find_user(2) {
        println("User 2 is ", name)
    }

    // Result handling
    when divide(10.0, 2.0) {
        Ok(result) -> println("Result: ", result)
        Err(msg) -> println("Error: ", msg)
    }
}
```

---

## Chapter 5: Generics and Traits

### 5.1 Generic Functions

```mana
module main

generic<T>
fn identity(x: T) -> T {
    return x
}

generic<T>
fn swap(a: T, b: T) -> (T, T) {
    return (b, a)
}

fn main() {
    let x = identity(42)
    let s = identity("hello")

    let (a, b) = swap(1, 2)
    println("Swapped: ", a, ", ", b)
}
```

### 5.2 Generic Structs

```mana
module main

generic<T>
struct Pair {
    first: T,
    second: T,
}

generic<T>
impl Pair<T> {
    fn new(a: T, b: T) -> Pair<T> {
        return Pair { first: a, second: b }
    }

    fn swap(self) -> Pair<T> {
        return Pair { first: self.second, second: self.first }
    }
}

fn main() {
    let p = Pair::new(1, 2)
    println("(", p.first, ", ", p.second, ")")

    let swapped = p.swap()
    println("Swapped: (", swapped.first, ", ", swapped.second, ")")
}
```

### 5.3 Traits

```mana
module main

trait Printable {
    fn print(self)
}

trait Describable {
    fn describe(self) -> str
}

struct Dog {
    name: str,
    age: int,
}

impl Printable for Dog {
    fn print(self) {
        println("Dog: ", self.name)
    }
}

impl Describable for Dog {
    fn describe(self) -> str {
        return self.name + ", " + self.age + " years old"
    }
}

fn main() {
    let dog = Dog { name: "Buddy", age: 3 }
    dog.print()
    println(dog.describe())
}
```

### 5.4 Trait Bounds

```mana
module main

trait Display {
    fn to_string(self) -> str
}

generic<T> where T: Display
fn print_item(item: T) {
    println(item.to_string())
}

fn main() {
    // Works with any type implementing Display
}
```

---

## Chapter 6: Error Handling

### 6.1 Result Type

```mana
module main

fn read_number(s: str) -> Result<int, str> {
    // Simplified - in reality would parse
    if s == "42" {
        return Ok(42)
    }
    return Err("Cannot parse '" + s + "' as number")
}

fn main() {
    let result = read_number("42")

    when result {
        Ok(n) -> println("Got number: ", n)
        Err(e) -> println("Error: ", e)
    }
}
```

### 6.2 Error Propagation with ?

```mana
module main

fn get_config_value() -> Result<int, str> {
    // ... might fail
    return Ok(100)
}

fn calculate() -> Result<int, str> {
    let value = get_config_value()?  // Propagates error if any
    return Ok(value * 2)
}

fn main() {
    when calculate() {
        Ok(result) -> println("Result: ", result)
        Err(e) -> println("Failed: ", e)
    }
}
```

### 6.3 Error Propagation with or

```mana
module main

fn get_value() -> Result<int, str> {
    return Err("not found")
}

// or return - unwrap Ok or early return on Err
fn get_value_or_default() -> int {
    let value = get_value() or return 0
    return value
}

// or { block } - unwrap Ok or execute block on Err
fn get_value_with_handler() -> int {
    let value = get_value() or {
        println("Error occurred, using default")
        return -1
    }
    return value
}

fn main() {
    let a = get_value_or_default()
    println("Value (or default): ", a)

    let b = get_value_with_handler()
    println("Value (with handler): ", b)
}
```

### 6.4 Assertions

```mana
module main

fn main() {
    let x = 5

    assert(x > 0)
    assert(x > 0, "x must be positive")

    assert_eq(2 + 2, 4)
    assert_ne(1, 2)

    println("All assertions passed!")
}
```

---

## Chapter 7: Iterators

### 7.1 Basic Iteration

```mana
module main

fn main() {
    let numbers = vec![1, 2, 3, 4, 5]

    // For loop
    for n in numbers {
        println(n)
    }

    // Range iteration
    for i in 0..5 {
        println("Index: ", i)
    }

    // With enumerate
    let words = vec!["hello", "world"]
    for (i, word) in words.enumerate() {
        println(i, ": ", word)
    }
}
```

### 7.2 Iterator Methods

```mana
module main

fn main() {
    let numbers = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

    // Map
    let doubled = numbers.map(|x| x * 2)

    // Filter
    let evens = numbers.filter(|x| x % 2 == 0)

    // Fold (reduce)
    let sum = numbers.fold(0, |acc, x| acc + x)
    println("Sum: ", sum)

    // Find
    if let some(n) = numbers.find(|x| x > 5) {
        println("First > 5: ", n)
    }

    // All/Any
    let all_positive = numbers.all(|x| x > 0)
    let has_ten = numbers.any(|x| x == 10)
}
```

---

## Chapter 8: Closures

### 8.1 Basic Closures

```mana
module main

fn main() {
    // Simple closure
    let add = |a, b| a + b
    println("3 + 4 = ", add(3, 4))

    // With type annotations
    let multiply = |x: int, y: int| -> int { x * y }

    // Multi-statement closure
    let process = |x| {
        let doubled = x * 2
        let squared = doubled * doubled
        return squared
    }

    println("process(3) = ", process(3))
}
```

### 8.2 Capturing Variables

```mana
module main

fn main() {
    let factor = 10

    // Closure captures 'factor' from environment
    let scale = |x| x * factor

    println("scale(5) = ", scale(5))
}
```

---

## Chapter 9: Modules

### 9.1 Single File Modules

```mana
// math.mana
module math

pub fn add(a: int, b: int) -> int {
    return a + b
}

pub fn multiply(a: int, b: int) -> int {
    return a * b
}

fn internal_helper() {
    // Not exported (no 'pub')
}
```

### 9.2 Importing Modules

```mana
// main.mana
module main

import math

fn main() {
    let sum = math::add(3, 4)
    let product = math::multiply(5, 6)

    println("Sum: ", sum)
    println("Product: ", product)
}
```

### 9.3 Visibility

```mana
module example

// Public struct
pub struct Point {
    pub x: float,   // Public field
    pub y: float,   // Public field
    id: int,        // Private field
}

// Public function
pub fn create_point(x: float, y: float) -> Point {
    return Point { x: x, y: y, id: next_id() }
}

// Private function
fn next_id() -> int {
    // ...
    return 0
}
```

---

## Chapter 10: Testing

### 10.1 Writing Tests

```mana
module math_tests

fn add(a: int, b: int) -> int {
    return a + b
}

#[test]
fn test_add_positive() {
    assert_eq(add(2, 3), 5)
}

#[test]
fn test_add_negative() {
    assert_eq(add(-1, 1), 0)
}

#[test]
fn test_add_zero() {
    assert_eq(add(5, 0), 5)
}

fn main() {
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
let i = 42
let f = i as f64      // int to f64
let u = i as u32      // int to u32
let c = 65 as char    // int to char
```

### Printing (Variadic)

```mana
let name = "World"
let count = 42

// Variadic print - pass multiple arguments
println("Hello, ", name, "!")
println("Count: ", count)

// Multiple values in one call
println("a = ", a, ", b = ", b, ", sum = ", a + b)

// No newline
print("Enter value: ")
```

### Common Operations

```mana
// Math
let abs_val = abs(-5)
let max_val = max(a, b)
let min_val = min(a, b)
let sqrt_val = sqrt(16.0)

// Strings
let len = str.len()
let upper = str.to_upper()
let trimmed = str.trim()
let parts = str.split(",")

// Vectors
let len = vec.len()
vec.push(item)
let item = vec.pop()
vec.sort()
```

### vNext Syntax Summary

| Feature | Description |
|---------|-------------|
| Semicolons | Optional for all statements |
| `int`, `float` | Type aliases for `i32`, `f32` |
| `when ... { x -> y }` | Alternative to `match ... { x => y }` |
| `x or return 0` | Unwrap Result or early return |
| `variant` | Synonym for `enum` |
| `fn main()` | No return type needed |
| `println(a, b, c)` | Variadic print |

---

*Happy coding with Mana!*
