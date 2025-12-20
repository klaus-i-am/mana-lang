# Advanced Patterns in Mana

This guide covers idiomatic patterns and best practices for writing clean, efficient Mana code.

## Table of Contents

- [Builder Pattern](#builder-pattern)
- [Error Handling Strategies](#error-handling-strategies)
- [State Machines](#state-machines)
- [Functional Pipelines](#functional-pipelines)
- [Resource Management](#resource-management)
- [Type-Safe IDs](#type-safe-ids)
- [Configuration Pattern](#configuration-pattern)
- [Event Systems](#event-systems)

---

## Builder Pattern

For complex objects with many optional parameters, use the builder pattern:

```mana
struct HttpRequest {
    method: string,
    url: string,
    headers: Vec<(string, string)>,
    body: Option<string>,
    timeout_ms: int,
}

struct HttpRequestBuilder {
    method: string,
    url: string,
    headers: Vec<(string, string)>,
    body: Option<string>,
    timeout_ms: int,
}

impl HttpRequestBuilder {
    fn new(url: string) -> HttpRequestBuilder {
        return HttpRequestBuilder {
            method: "GET",
            url: url,
            headers: vec![],
            body: none,
            timeout_ms: 30000,
        }
    }

    fn method(mut self, m: string) -> HttpRequestBuilder {
        self.method = m
        return self
    }

    fn header(mut self, key: string, value: string) -> HttpRequestBuilder {
        self.headers.push((key, value))
        return self
    }

    fn body(mut self, b: string) -> HttpRequestBuilder {
        self.body = some(b)
        return self
    }

    fn timeout(mut self, ms: int) -> HttpRequestBuilder {
        self.timeout_ms = ms
        return self
    }

    fn build(self) -> HttpRequest {
        return HttpRequest {
            method: self.method,
            url: self.url,
            headers: self.headers,
            body: self.body,
            timeout_ms: self.timeout_ms,
        }
    }
}

// Usage
fn main() {
    let request = HttpRequestBuilder::new("https://api.example.com/users")
        .method("POST")
        .header("Content-Type", "application/json")
        .header("Authorization", "Bearer token123")
        .body("{\"name\": \"Alice\"}")
        .timeout(5000)
        .build()
}
```

---

## Error Handling Strategies

### Custom Error Types

```mana
enum AppError {
    NotFound(string),
    InvalidInput(string),
    NetworkError(string),
    DatabaseError(string),
}

impl AppError {
    fn message(self) -> string {
        return when self {
            AppError::NotFound(s) -> "Not found: " + s
            AppError::InvalidInput(s) -> "Invalid input: " + s
            AppError::NetworkError(s) -> "Network error: " + s
            AppError::DatabaseError(s) -> "Database error: " + s
        }
    }
}

type AppResult<T> = Result<T, AppError>

fn find_user(id: int) -> AppResult<User> {
    if id <= 0 {
        return err(AppError::InvalidInput("ID must be positive"))
    }

    // ... database lookup
    return err(AppError::NotFound("User " + id.to_string()))
}
```

### Early Return with `or` Operator

```mana
fn process_order(order_id: int) -> AppResult<Receipt> {
    // Using the `or` operator for clean error handling
    let order = get_order(order_id) or return err(AppError::NotFound("order"))
    let user = get_user(order.user_id) or return err(AppError::NotFound("user"))
    let payment = process_payment(order, user) or return err(AppError::NetworkError("payment failed"))
    let receipt = generate_receipt(order, payment) or return err(AppError::DatabaseError("receipt generation"))

    return ok(receipt)
}

// Or with ? propagation
fn process_order_short(order_id: int) -> AppResult<Receipt> {
    let order = get_order(order_id)?
    let user = get_user(order.user_id)?
    let payment = process_payment(order, user)?
    let receipt = generate_receipt(order, payment)?

    return ok(receipt)
}
```

### Fallback Values with `or`

```mana
fn get_config_or_default() -> Config {
    // Use `or` for simple defaults
    let port_str = env::get("PORT") or "8080"
    let port = port_str.parse_int() or 8080

    // Use `or` with blocks for computed defaults
    let db_url = env::get("DATABASE_URL") or {
        println("Using default database")
        "sqlite://local.db"
    }

    return Config { port: port, db_url: db_url }
}
```

---

## State Machines

Model explicit states to prevent invalid operations:

```mana
// Connection states
variant ConnectionState {
    Disconnected,
    Connecting,
    Connected(Socket),
    Error(string),
}

struct Connection {
    state: ConnectionState,
    host: string,
    port: int,
}

impl Connection {
    fn new(host: string, port: int) -> Connection {
        return Connection {
            state: ConnectionState::Disconnected,
            host: host,
            port: port,
        }
    }

    fn connect(mut self) -> Result<(), string> {
        // Can only connect from Disconnected state
        when self.state {
            ConnectionState::Disconnected -> {
                self.state = ConnectionState::Connecting

                when Socket::connect(self.host, self.port) {
                    ok(socket) -> {
                        self.state = ConnectionState::Connected(socket)
                        return ok(())
                    }
                    err(e) -> {
                        self.state = ConnectionState::Error(e)
                        return err(e)
                    }
                }
            }
            ConnectionState::Connected(_) -> {
                return err("Already connected")
            }
            ConnectionState::Connecting -> {
                return err("Connection in progress")
            }
            ConnectionState::Error(e) -> {
                return err("Previous error: " + e)
            }
        }
    }

    fn send(self, data: string) -> Result<(), string> {
        when self.state {
            ConnectionState::Connected(socket) -> {
                return socket.send(data)
            }
            _ -> {
                return err("Not connected")
            }
        }
    }

    fn is_connected(self) -> bool {
        return when self.state {
            ConnectionState::Connected(_) -> true
            _ -> false
        }
    }
}
```

---

## Functional Pipelines

Chain operations for readable data transformations:

```mana
struct Order {
    id: int,
    customer: string,
    total: float,
    status: string,
}

fn get_high_value_customers(orders: Vec<Order>) -> Vec<string> {
    return orders
        .filter(|o| o.status == "completed")
        .filter(|o| o.total > 100.0)
        .map(|o| o.customer)
        .collect::<HashSet<string>>()  // Deduplicate
        .into_iter()
        .collect::<Vec<string>>()
}

// Multi-step processing with intermediate variables for clarity
fn process_data(items: Vec<RawItem>) -> Vec<ProcessedItem> {
    let valid_items = items.filter(|i| i.is_valid())
    let transformed = valid_items.map(|i| transform(i))
    let sorted = transformed.sorted_by(|a, b| a.priority.cmp(b.priority))
    return sorted.collect()
}

// Fold for aggregation
fn calculate_stats(numbers: Vec<float>) -> Stats {
    let (sum, count, min, max) = numbers.fold(
        (0.0, 0, float::MAX, float::MIN),
        |(sum, count, min, max), n| {
            return (
                sum + n,
                count + 1,
                if n < min { n } else { min },
                if n > max { n } else { max },
            )
        }
    )

    return Stats {
        sum: sum,
        count: count,
        min: min,
        max: max,
        avg: if count > 0 { sum / count as float } else { 0.0 },
    }
}
```

---

## Resource Management

### RAII with Defer

```mana
fn read_file_safely(path: string) -> Result<string, string> {
    let file = File::open(path)?
    defer file.close()  // Guaranteed cleanup

    let content = file.read_all()?
    return ok(content)
}

fn with_transaction(db: Database, work: fn() -> Result<(), string>) -> Result<(), string> {
    db.begin_transaction()?

    // Rollback on failure, commit on success
    defer {
        if result.is_err() {
            db.rollback()
        }
    }

    let result = work()

    if result.is_ok() {
        db.commit()?
    }

    return result
}
```

### Resource Wrapper Pattern

```mana
struct ManagedResource<T> {
    value: T,
    cleanup: fn(T),
}

impl<T> ManagedResource<T> {
    fn new(value: T, cleanup: fn(T)) -> ManagedResource<T> {
        return ManagedResource {
            value: value,
            cleanup: cleanup,
        }
    }

    fn get(self) -> T {
        return self.value
    }
}

impl<T> Drop for ManagedResource<T> {
    fn drop(self) {
        (self.cleanup)(self.value)
    }
}

// Usage
fn main() {
    let buffer = ManagedResource::new(
        allocate_buffer(1024),
        |b| free_buffer(b)
    )

    // Use buffer.get()...
    // Automatically freed when buffer goes out of scope
}
```

---

## Type-Safe IDs

Prevent mixing up different ID types:

```mana
// Newtype pattern for type-safe IDs
struct UserId(i64)
struct OrderId(i64)
struct ProductId(i64)

impl UserId {
    fn new(id: i64) -> UserId {
        return UserId(id)
    }

    fn value(self) -> i64 {
        return self.0
    }
}

impl OrderId {
    fn new(id: i64) -> OrderId {
        return OrderId(id)
    }

    fn value(self) -> i64 {
        return self.0
    }
}

// Now these can't be accidentally swapped
fn get_user(id: UserId) -> Option<User> {
    // ...
}

fn get_order(id: OrderId) -> Option<Order> {
    // ...
}

fn main() {
    let user_id = UserId::new(123)
    let order_id = OrderId::new(456)

    get_user(user_id)    // OK
    // get_user(order_id) // Compile error! Type mismatch
}
```

---

## Configuration Pattern

Layered configuration with defaults, files, and environment:

```mana
struct Config {
    host: string,
    port: int,
    debug: bool,
    database_url: string,
    max_connections: int,
}

impl Config {
    fn defaults() -> Config {
        return Config {
            host: "localhost",
            port: 8080,
            debug: false,
            database_url: "sqlite://app.db",
            max_connections: 10,
        }
    }

    fn from_file(path: string) -> Result<Config, string> {
        let content = fs::read_string(path)?
        // Parse TOML/JSON/etc
        return parse_config(content)
    }

    fn from_env(mut self) -> Config {
        if let some(host) = env::get("APP_HOST") {
            self.host = host
        }
        if let some(port) = env::get("APP_PORT") {
            self.port = port.parse_int() or self.port
        }
        if let some(_) = env::get("APP_DEBUG") {
            self.debug = true
        }
        if let some(url) = env::get("DATABASE_URL") {
            self.database_url = url
        }
        return self
    }

    /// Load config: defaults -> file -> environment
    fn load() -> Config {
        let mut config = Config::defaults()

        // Try config file (optional)
        if let ok(file_config) = Config::from_file("config.toml") {
            config = file_config
        }

        // Environment overrides everything
        config = config.from_env()

        return config
    }
}
```

---

## Event Systems

### Simple Observer Pattern

```mana
type EventHandler<T> = fn(T)

struct EventEmitter<T> {
    handlers: Vec<EventHandler<T>>,
}

impl<T> EventEmitter<T> {
    fn new() -> EventEmitter<T> {
        return EventEmitter { handlers: vec![] }
    }

    fn on(mut self, handler: EventHandler<T>) {
        self.handlers.push(handler)
    }

    fn emit(self, event: T) {
        for handler in self.handlers {
            handler(event.clone())
        }
    }
}

// Usage
struct UserCreated {
    id: int,
    name: string,
}

fn main() {
    let mut emitter = EventEmitter::new()

    emitter.on(|e: UserCreated| {
        println("Welcome email sent to ", e.name)
    })

    emitter.on(|e: UserCreated| {
        println("User ", e.id, " added to analytics")
    })

    // Trigger event
    emitter.emit(UserCreated { id: 1, name: "Alice" })
}
```

### Command Pattern

```mana
trait Command {
    fn execute(self)
    fn undo(self)
}

struct CommandHistory {
    executed: Vec<Box<dyn Command>>,
}

impl CommandHistory {
    fn new() -> CommandHistory {
        return CommandHistory { executed: vec![] }
    }

    fn execute(mut self, cmd: Box<dyn Command>) {
        cmd.execute()
        self.executed.push(cmd)
    }

    fn undo(mut self) {
        if let some(cmd) = self.executed.pop() {
            cmd.undo()
        }
    }
}

// Example command
struct InsertTextCommand {
    document: Document,
    position: int,
    text: string,
}

impl Command for InsertTextCommand {
    fn execute(self) {
        self.document.insert(self.position, self.text)
    }

    fn undo(self) {
        self.document.delete(self.position, self.text.len())
    }
}
```

---

## Best Practices Summary

1. **Prefer `Option` over nullable values** - Make absence explicit
2. **Use `Result` for recoverable errors** - Don't panic on expected failures
3. **Use `or` for clean error handling** - `value or default` and `result or return err`
4. **Favor composition over inheritance** - Use traits and composition
5. **Make invalid states unrepresentable** - Use enums/variants for state machines
6. **Keep functions small and focused** - Single responsibility
7. **Use meaningful names** - Code is read more than written
8. **Defer for cleanup** - Guarantee resource release
9. **Leverage type system** - Newtypes prevent bugs at compile time
10. **Chain iterators** - Functional style for transformations
11. **Document public APIs** - Help future maintainers (including yourself)

---

*See also: [Language Reference](LANGUAGE_SPEC.md) | [Standard Library](STDLIB.md)*
