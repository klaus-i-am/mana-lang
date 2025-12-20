# Mana Error Message Catalog

This document explains all error messages produced by the Mana compiler, with examples and suggested fixes.

## Table of Contents

- [Parser Errors](#parser-errors)
- [Type Errors](#type-errors)
- [Semantic Errors](#semantic-errors)
- [Module Errors](#module-errors)
- [Pattern Matching Errors](#pattern-matching-errors)
- [Function Errors](#function-errors)
- [Trait Errors](#trait-errors)

---

## Parser Errors

### unexpected top-level declaration

**When:** The compiler encountered something it doesn't recognize at the module level.

```mana
module main

42  // Error: unexpected top-level declaration
```

**Fix:** Only declarations are allowed at the top level: `fn`, `struct`, `enum`, `variant`, `trait`, `impl`, `const`, `type`, `import`.

```mana
module main

fn main() {
    println("42")
}
```

---

### 'self' must be first parameter

**When:** A method has `self` in a position other than the first parameter.

```mana
impl Player {
    fn update(delta: float, self) {  // Error
        // ...
    }
}
```

**Fix:** Move `self` to the first position.

```mana
impl Player {
    fn update(self, delta: float) {
        // ...
    }
}
```

---

### expected expression

**When:** The parser expected an expression but found something else.

```mana
let x =   // Error: expected expression
```

**Fix:** Provide a valid expression.

```mana
let x = 42
```

---

### expected '{' after generic type

**When:** A generic struct literal is missing its opening brace.

```mana
let p = Pair<int>   // Error: expected '{'
```

**Fix:** Add the struct body.

```mana
let p = Pair<int> { first: 1, second: 2 }
```

---

### expected '|' after 'move' for closure

**When:** A `move` closure is missing the parameter delimiters.

```mana
let f = move x { x + 1 }  // Error
```

**Fix:** Use the pipe syntax for parameters.

```mana
let f = move |x| { x + 1 }
```

---

### expected pattern in match arm

**When:** A match arm is missing its pattern.

```mana
match x {
    => println("oops"),  // Error: expected pattern
}
```

**Fix:** Add a pattern before the arrow.

```mana
match x {
    0 => println("zero"),
    _ => println("other"),
}

// Or use when syntax
when x {
    0 -> println("zero")
    _ -> println("other")
}
```

---

### expected number after range operator

**When:** A range pattern is incomplete.

```mana
match x {
    1.. => println("one or more"),  // Error in some contexts
}
```

**Fix:** Complete the range.

```mana
when x {
    1..=10 -> println("one to ten")
    _ -> println("other")
}
```

---

### function call on non-identifier not supported

**When:** Trying to call a function on a complex expression that isn't supported.

```mana
(get_func())()  // Error: function call on non-identifier not supported
```

**Fix:** Store the function in a variable first.

```mana
let f = get_func()
f()
```

---

## Type Errors

### type mismatch in variable initialization

**When:** The initializer type doesn't match the declared variable type.

```mana
let x: int = "hello"  // Error: expected int, got str
```

**Fix:** Either change the type annotation or the value.

```mana
let x: int = 42
// or
let x: str = "hello"
```

---

### type mismatch in assignment

**When:** Assigning a value of the wrong type to a variable.

```mana
let mut x: int = 10
x = "hello"  // Error: expected int, got str
```

**Fix:** Assign a value of the correct type.

```mana
let mut x: int = 10
x = 20
```

---

### cannot assign to immutable variable

**When:** Trying to modify a variable declared without `mut`.

```mana
let x = 10
x = 20  // Error: cannot assign to immutable variable 'x'
```

**Fix:** Declare the variable as mutable.

```mana
let mut x = 10
x = 20
```

---

### if condition must be bool

**When:** The condition in an `if` statement isn't a boolean.

```mana
if 42 {  // Error: if condition must be bool, got int
    println("oops")
}
```

**Fix:** Use a boolean expression.

```mana
if 42 > 0 {
    println("positive")
}
```

---

### while condition must be bool

**When:** The condition in a `while` loop isn't a boolean.

```mana
while 1 {  // Error: while condition must be bool, got int
    // ...
}
```

**Fix:** Use a boolean expression.

```mana
while true {
    // ...
}
// or
while counter > 0 {
    // ...
}
```

---

### return type mismatch

**When:** The returned value doesn't match the function's declared return type.

```mana
fn get_number() -> int {
    return "hello"  // Error: expected int, got str
}
```

**Fix:** Return the correct type.

```mana
fn get_number() -> int {
    return 42
}
```

---

### invalid binary operator operands

**When:** An operator is used with incompatible types.

```mana
let x = "hello" + 42  // Error: cannot apply '+' to str and int
```

**Fix:** Use compatible types or convert them.

```mana
let x = "hello" + "world"  // String concatenation
// or use variadic print
println("hello", 42)
```

---

### cannot dereference non-pointer type

**When:** Using `*` on a value that isn't a pointer.

```mana
let x = 42
let y = *x  // Error: cannot dereference non-pointer type int
```

**Fix:** Only dereference pointers.

```mana
let x = 42
let ptr = &x
let y = *ptr  // OK
```

---

### left/right operand of '&&'/'||' must be bool

**When:** Using logical operators with non-boolean types.

```mana
let x = 1 && 2  // Error: left operand of '&&' must be bool
```

**Fix:** Use boolean expressions.

```mana
let x = true && false
// or
let x = (a > 0) && (b > 0)
```

---

### tuple index on non-tuple type

**When:** Using `.0`, `.1`, etc. on something that isn't a tuple.

```mana
let x = 42
let y = x.0  // Error: tuple index on non-tuple type int
```

**Fix:** Only use tuple indexing on tuples.

```mana
let x = (1, 2, 3)
let y = x.0  // OK: y = 1
```

---

### tuple index out of bounds

**When:** Accessing a tuple element that doesn't exist.

```mana
let x = (1, 2)
let y = x.5  // Error: tuple index out of bounds: index 5 on tuple with 2 elements
```

**Fix:** Use a valid index.

```mana
let x = (1, 2)
let y = x.1  // OK: y = 2
```

---

### array elements have inconsistent types

**When:** Array elements have different types.

```mana
let arr = [1, "hello", 3.14]  // Error: inconsistent types
```

**Fix:** Make all elements the same type.

```mana
let arr = [1, 2, 3]  // All int
// or use a tuple for mixed types
let mixed = (1, "hello", 3.14)
```

---

## Semantic Errors

### break outside loop

**When:** Using `break` outside of a loop.

```mana
fn main() {
    break  // Error: break outside loop
}
```

**Fix:** Only use `break` inside loops.

```mana
fn main() {
    loop {
        break  // OK
    }
}
```

---

### continue outside loop

**When:** Using `continue` outside of a loop.

```mana
fn main() {
    continue  // Error: continue outside loop
}
```

**Fix:** Only use `continue` inside loops.

```mana
fn main() {
    for i in 0..10 {
        if i == 5 {
            continue  // OK
        }
    }
}
```

---

### 'self' used outside of method

**When:** Using `self` in a function that isn't a method.

```mana
fn standalone() {
    println(self.name)  // Error: 'self' used outside of method
}
```

**Fix:** Only use `self` inside `impl` blocks.

```mana
impl Player {
    fn print_name(self) {
        println(self.name)  // OK
    }
}
```

---

### unknown struct member

**When:** Accessing a field that doesn't exist on a struct.

```mana
struct Point { x: float, y: float }

let p = Point { x: 1.0, y: 2.0 }
println(p.z)  // Error: unknown struct member 'z' on type Point
```

**Fix:** Use an existing field name.

```mana
println(p.x)  // OK
```

---

### unknown struct field (in initialization)

**When:** Initializing a struct with a field that doesn't exist.

```mana
struct Point { x: float, y: float }

let p = Point { x: 1.0, z: 2.0 }  // Error: unknown struct field
```

**Fix:** Use the correct field names.

```mana
let p = Point { x: 1.0, y: 2.0 }
```

---

### too many initializers for struct

**When:** Providing more fields than the struct has.

```mana
struct Point { x: float, y: float }

let p = Point { x: 1.0, y: 2.0, z: 3.0 }  // Error: too many initializers
```

**Fix:** Only initialize existing fields.

```mana
let p = Point { x: 1.0, y: 2.0 }
```

---

### type alias already defined

**When:** Defining a type alias with a name that's already used.

```mana
type Id = int
type Id = i64  // Error: type alias already defined: Id
```

**Fix:** Use a different name.

```mana
type Id32 = int
type Id64 = i64
```

---

### impl for unknown type

**When:** Implementing methods for a type that doesn't exist.

```mana
impl NonExistent {  // Error: impl for unknown type
    fn foo(self) {}
}
```

**Fix:** Define the type first.

```mana
struct MyType {}

impl MyType {
    fn foo(self) {}
}
```

---

### impl for unknown trait

**When:** Implementing a trait that doesn't exist.

```mana
impl NonExistentTrait for MyType {  // Error: impl for unknown trait
    fn foo(self) {}
}
```

**Fix:** Define the trait first.

```mana
trait MyTrait {
    fn foo(self)
}

impl MyTrait for MyType {
    fn foo(self) {}
}
```

---

### default value type mismatch for field

**When:** A struct field's default value doesn't match its type.

```mana
struct Config {
    port: int = "8080",  // Error: default value type mismatch
}
```

**Fix:** Use the correct type for the default value.

```mana
struct Config {
    port: int = 8080,
}
```

---

## Module Errors

### circular module dependency

**When:** Two or more modules depend on each other in a cycle.

```mana
// a.mana
module a
import b

// b.mana
module b
import a  // Error: circular module dependency
```

**Fix:** Restructure your modules to break the cycle. Extract shared code into a third module.

```mana
// shared.mana
module shared
pub struct Common {}

// a.mana
module a
import shared

// b.mana
module b
import shared
```

---

### cannot open file

**When:** The compiler can't find or read an imported file.

```mana
import nonexistent  // Error: cannot open file: nonexistent.mana
```

**Fix:** Ensure the file exists and the path is correct.

---

### failed to parse

**When:** An imported module has syntax errors.

**Fix:** Fix the syntax errors in the imported module first.

---

### 'X' not found in module

**When:** Importing something that doesn't exist in a module.

```mana
import math::{add, nonexistent}  // Error: 'nonexistent' not found in module 'math'
```

**Fix:** Only import items that exist in the module.

```mana
import math::{add, multiply}
```

---

### 'X' is private in module

**When:** Trying to use a non-public item from another module.

```mana
// lib.mana
module lib
fn internal() {}  // Not public

// main.mana
module main
import lib
lib::internal()  // Error: 'internal' is private in module 'lib'
```

**Fix:** Either make the item public or don't import it.

```mana
// lib.mana
module lib
pub fn internal() {}  // Now public
```

---

## Pattern Matching Errors

### match arms have different types

**When:** The branches of a match expression return different types.

```mana
let result = match x {
    0 => "zero",    // str
    _ => 42,        // int  Error!
}
```

**Fix:** Make all arms return the same type.

```mana
let result = match x {
    0 => "zero",
    _ => "other",
}

// Or with when syntax
let result = when x {
    0 -> "zero"
    _ -> "other"
}
```

---

### unknown enum variant

**When:** Using a variant that doesn't exist on an enum.

```mana
enum Color { Red, Green, Blue }

let c = Color::Yellow  // Error: unknown variant 'Yellow' for enum 'Color'
```

**Fix:** Use an existing variant.

```mana
let c = Color::Red
```

---

### wrong number of arguments for enum variant

**When:** Providing the wrong number of values to an enum variant.

```mana
enum Shape {
    Circle(float),           // Takes 1 value
    Rectangle(float, float), // Takes 2 values
}

let s = Shape::Circle(1.0, 2.0)  // Error: expected 1, got 2
```

**Fix:** Provide the correct number of arguments.

```mana
let s = Shape::Circle(5.0)
```

---

### unit variant takes no arguments

**When:** Passing arguments to an enum variant that doesn't take any.

```mana
enum Status { Active, Inactive }

let s = Status::Active(true)  // Error: unit variant 'Active' takes no arguments
```

**Fix:** Don't pass arguments to unit variants.

```mana
let s = Status::Active
```

---

## Function Errors

### function expects N arguments, got M

**When:** Calling a function with the wrong number of arguments.

```mana
fn add(a: int, b: int) -> int {
    return a + b
}

let x = add(1)  // Error: expects 2 arguments, got 1
```

**Fix:** Provide the correct number of arguments.

```mana
let x = add(1, 2)
```

---

### unknown parameter name

**When:** Using named arguments with an invalid parameter name.

```mana
fn greet(name: str) {
    println("Hello, ", name, "!")
}

greet(user: "Alice")  // Error: unknown parameter name 'user'
```

**Fix:** Use the correct parameter name.

```mana
greet(name: "Alice")
```

---

### duplicate argument for parameter

**When:** Providing the same named argument twice.

```mana
greet(name: "Alice", name: "Bob")  // Error: duplicate argument for parameter 'name'
```

**Fix:** Only provide each argument once.

```mana
greet(name: "Alice")
```

---

## Trait Errors

### type does not implement trait

**When:** Using a type where a trait bound is required but not implemented.

```mana
generic<T> where T: Display
fn print_item(item: T) {
    println(item.to_string())
}

print_item(MyType{})  // Error: 'MyType' does not implement trait 'Display'
```

**Fix:** Implement the required trait.

```mana
impl Display for MyType {
    fn to_string(self) -> str {
        return "MyType"
    }
}
```

---

### missing associated type in trait impl

**When:** A trait implementation is missing a required associated type.

```mana
trait Container {
    type Item
    fn get(self) -> Self::Item
}

impl Container for Box {  // Error: missing associated type 'Item'
    fn get(self) -> int { return 0 }
}
```

**Fix:** Provide all associated types.

```mana
impl Container for Box {
    type Item = int
    fn get(self) -> Self::Item { return 0 }
}
```

---

### where clause references unknown type parameter

**When:** A where clause uses a type parameter that doesn't exist.

```mana
generic<T> where U: Display  // Error: unknown type parameter 'U'
fn foo(x: T) {}
```

**Fix:** Only reference declared type parameters.

```mana
generic<T> where T: Display
fn foo(x: T) {}
```

---

### where clause references unknown trait

**When:** A where clause references a trait that doesn't exist.

```mana
generic<T> where T: NonExistent  // Error: unknown trait 'NonExistent'
fn foo(x: T) {}
```

**Fix:** Use an existing trait or define it first.

```mana
trait MyTrait {}

generic<T> where T: MyTrait
fn foo(x: T) {}
```

---

## See Also

- [Language Specification](LANGUAGE_SPEC.md) - Full language syntax
- [Tutorial](TUTORIAL.md) - Getting started guide
- [Standard Library](STDLIB.md) - Built-in types and functions
