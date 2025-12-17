# Module: doc_test

## Type Aliases

### `pub` `type Coord`

```mana
pub type Coord = i32;
```

A type alias for integer coordinates

---

## Structs

### `pub` `struct Point`

```mana
pub struct Point {
    x: f64,
    y: f64,
}
```

A simple 2D point structure
Holds x and y coordinates

**Fields:**

| Name | Type | Default |
|------|------|--------|
| `x` | `f64` | - |
| `y` | `f64` | - |

---

### `pub` `struct Vector`

```mana
pub struct Vector {
    origin: Point,
    dx: f64,
    dy: f64,
}
```

A 2D vector with origin and direction

**Fields:**

| Name | Type | Default |
|------|------|--------|
| `origin` | `Point` | - |
| `dx` | `f64` | - |
| `dy` | `f64` | - |

---

## Enums

### `pub` `enum Shape`

```mana
pub enum Shape {
    Circle(f64),
    Rectangle(f64, f64),
    Triangle { a: f64, b: f64, c: f64 },
}
```

Represents different shapes
Used for geometry calculations

**Variants:**

| Name | Data |
|------|------|
| `Circle` | tuple(f64) |
| `Rectangle` | tuple(f64, f64) |
| `Triangle` | struct |

---

### `pub` `enum Color`

```mana
pub enum Color {
    Red,
    Green,
    Blue,
    Rgba { r: i32, g: i32, b: i32, a: i32 },
}
```

Colors with optional alpha

**Variants:**

| Name | Data |
|------|------|
| `Red` | - |
| `Green` | - |
| `Blue` | - |
| `Rgba` | struct |

---

## Traits

### `pub` `trait Display`

```mana
pub trait Display {
    fn to_string() -> String
}
```

Trait for objects that can be displayed

**Methods:**

| Name | Signature | Default |
|------|-----------|--------|
| `to_string` | `fn() -> String` | - |

---

## Functions

### `pub` `fn add`

```mana
pub fn add(a: i32, b: i32) -> i32
```

Adds two numbers and returns the result
This function supports any numeric types

**Parameters:**

| Name | Type | Default |
|------|------|--------|
| `a` | `i32` | - |
| `b` | `i32` | - |

**Returns:** `i32`

---

### `pub` `fn distance`

```mana
pub fn distance(p1: Point, p2: Point) -> f64
```

Calculates the distance between two points

**Parameters:**

| Name | Type | Default |
|------|------|--------|
| `p1` | `Point` | - |
| `p2` | `Point` | - |

**Returns:** `f64`

---

### `fn main`

```mana
fn main() -> i32
```

Main entry point

**Returns:** `i32`

---

