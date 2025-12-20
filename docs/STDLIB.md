# Mana Standard Library Reference

This document provides a comprehensive reference for all modules in the Mana standard library.

## Table of Contents

- [Core Types](#core-types)
- [Collections](#collections)
- [Networking](#networking)
- [Concurrency](#concurrency)
- [Encoding](#encoding)
- [Compression](#compression)
- [Database](#database)
- [XML](#xml)
- [JSON](#json)
- [Cryptography](#cryptography)
- [Regular Expressions](#regular-expressions)
- [File System](#file-system)
- [Date and Time](#date-and-time)
- [UUID](#uuid)
- [Environment](#environment)
- [Async/Await](#asyncawait)
- [Basic I/O](#basic-io)
- [String Functions](#string-functions)
- [Math Functions](#math-functions)
- [Math Types](#math-types)
- [Random](#random)
- [Time](#time)
- [Graphics (OpenGL)](#graphics-opengl)
- [Audio](#audio)

---

## Core Types

### Option<T>

Represents an optional value that may or may not be present.

```mana
let maybe_value: Option<i32> = Some(42)
let nothing: Option<i32> = None

if maybe_value.is_some() {
    println(f"Value: {maybe_value.unwrap()}")
}

// Safe unwrapping with default
let value = maybe_value.unwrap_or(0)
```

**Methods:**
- `is_some() -> bool` - Check if value is present
- `is_none() -> bool` - Check if value is absent
- `unwrap() -> T` - Get value (panics if None)
- `unwrap_or(default: T) -> T` - Get value or default
- `map(fn: T -> U) -> Option<U>` - Transform value if present

### Result<T, E>

Represents either success (Ok) or failure (Err).

```mana
fn divide(a: i32, b: i32) -> Result<i32, str> {
    if b == 0 {
        return Err("Division by zero")
    }
    return Ok(a / b)
}

match divide(10, 2) {
    Ok(result) => println(f"Result: {result}"),
    Err(e) => println(f"Error: {e}"),
}

// Error propagation with ?
fn calculate() -> Result<i32, str> {
    let x = divide(10, 2)?
    return Ok(x * 2)
}
```

**Methods:**
- `is_ok() -> bool` - Check if result is Ok
- `is_err() -> bool` - Check if result is Err
- `unwrap() -> T` - Get Ok value (panics if Err)
- `unwrap_err() -> E` - Get Err value (panics if Ok)
- `unwrap_or(default: T) -> T` - Get value or default
- `map(fn: T -> U) -> Result<U, E>` - Transform Ok value

---

## Collections

### Vec<T>

Dynamic array with contiguous storage.

```mana
let mut items: Vec<i32> = Vec::new()
items.push(1)
items.push(2)
items.push(3)

for item in items {
    println(f"{item}")
}

let first = items[0]          // Access by index
let len = items.len()         // Get length
let popped = items.pop()      // Remove last element
```

**Methods:**
- `new() -> Vec<T>` - Create empty vector
- `push(value: T)` - Add element to end
- `pop() -> Option<T>` - Remove and return last element
- `len() -> i64` - Get number of elements
- `is_empty() -> bool` - Check if empty
- `clear()` - Remove all elements
- `contains(value: T) -> bool` - Check if contains value

### HashMap<K, V>

Hash map for key-value storage.

```mana
let mut scores: HashMap<str, i32> = HashMap::new()
scores.insert("Alice", 100)
scores.insert("Bob", 85)

if let Some(score) = scores.get("Alice") {
    println(f"Alice's score: {score}")
}

for (name, score) in scores {
    println(f"{name}: {score}")
}
```

**Methods:**
- `new() -> HashMap<K, V>` - Create empty map
- `insert(key: K, value: V)` - Insert or update entry
- `get(key: K) -> Option<V>` - Get value by key
- `remove(key: K) -> Option<V>` - Remove and return entry
- `contains_key(key: K) -> bool` - Check if key exists
- `len() -> i64` - Get number of entries
- `keys() -> Vec<K>` - Get all keys
- `values() -> Vec<V>` - Get all values

### BinaryHeap<T>

Priority queue (max-heap by default).

```mana
let mut heap: BinaryHeap<i32> = BinaryHeap::new()
heap.push(3)
heap.push(1)
heap.push(4)

while !heap.is_empty() {
    println(f"{heap.pop()}")  // 4, 3, 1
}

// Min-heap
let mut min_heap: MinHeap<i32> = MinHeap::new()
```

---

## Networking

### HTTP/HTTPS

Full HTTP client with TLS support.

```mana
// GET request
let response = http::get("https://api.example.com/data")
if response.is_ok() {
    let res = response.unwrap()
    println(f"Status: {res.status}")
    println(f"Body: {res.body}")
}

// POST with JSON
let body = json::stringify(data)
let response = http::post("https://api.example.com/users", body, "application/json")

// PUT and DELETE
http::put(url, body, content_type)
http::del(url)
```

**Response fields:**
- `status: i32` - HTTP status code
- `status_text: str` - Status message
- `body: str` - Response body
- `headers: HashMap<str, str>` - Response headers

### WebSocket

Real-time bidirectional communication.

```mana
// Connect to WebSocket server
let ws = websocket::connect("wss://echo.websocket.org")
if ws.is_err() {
    println(f"Failed: {ws.unwrap_err()}")
    return
}

let socket = ws.unwrap()

// Send message
socket.send("Hello, WebSocket!")

// Receive message (blocking)
let msg = socket.receive()
if msg.is_ok() {
    let message = msg.unwrap()
    match message.type {
        MessageType::Text => println(f"Text: {message.data}"),
        MessageType::Binary => println("Binary data received"),
        MessageType::Close => println("Connection closed"),
    }
}

// Close connection
socket.close()
```

**WebSocket class methods:**
- `connect(url: str) -> Result<bool, str>` - Connect to server
- `send(message: str) -> Result<bool, str>` - Send text message
- `send_binary(data: str) -> Result<bool, str>` - Send binary data
- `receive() -> Result<Message, str>` - Receive message (blocking)
- `close(code?, reason?)` - Close connection
- `is_open() -> bool` - Check if connected

**Close codes:** `CloseCode::Normal`, `GoingAway`, `ProtocolError`, `InternalError`

### UDP Socket

User Datagram Protocol for low-latency communication.

```mana
let socket = net::UdpSocket::new()

// Bind to local address
socket.bind("127.0.0.1", 8080)

// Send to remote address
socket.send_to("Hello", "192.168.1.1", 9000)

// Receive data
let (data, addr, port) = socket.recv_from()

socket.close()
```

---

## Concurrency

### Mutex<T>

Mutual exclusion lock for thread-safe access.

```mana
let counter = sync::Mutex::new(0)

// Lock and access
{
    let mut guard = counter.lock()
    *guard += 1
}  // Lock released automatically

// Try to lock without blocking
if let Some(guard) = counter.try_lock() {
    // Got the lock
}
```

### RwLock<T>

Read-write lock allowing multiple readers or single writer.

```mana
let data = sync::RwLock::new(vec![1, 2, 3])

// Multiple readers allowed
{
    let read_guard = data.read()
    println(f"Data: {read_guard}")
}

// Exclusive writer
{
    let mut write_guard = data.write()
    write_guard.push(4)
}
```

### Arc<T>

Atomic reference counting for shared ownership across threads.

```mana
let shared = sync::Arc::new(42)
let clone = shared.clone()

thread::spawn(|| {
    println(f"Value: {clone.get()}")
})
```

### Channel<T>

Message passing between threads.

```mana
let (tx, rx) = sync::Channel::new()

thread::spawn(|| {
    tx.send("Hello from thread")
})

let message = rx.recv()
println(f"Received: {message}")
```

### ThreadPool

Pool of worker threads for parallel execution.

```mana
let pool = thread::ThreadPool::new(4)  // 4 workers

for i in 0..10 {
    pool.execute(|| {
        println(f"Task {i} running on thread")
    })
}

pool.join()  // Wait for all tasks to complete
```

---

## Encoding

### Base64

```mana
let encoded = encoding::base64_encode("Hello, World!")
// "SGVsbG8sIFdvcmxkIQ=="

let decoded = encoding::base64_decode(encoded)
if decoded.is_ok() {
    println(decoded.unwrap())  // "Hello, World!"
}
```

### URL Encoding

```mana
let encoded = encoding::url_encode("hello world&foo=bar")
// "hello%20world%26foo%3Dbar"

let decoded = encoding::url_decode(encoded)
```

### Hex Encoding

```mana
let hex = encoding::hex_encode("ABC")
// "414243"

let bytes = encoding::hex_decode(hex)
```

---

## Compression

### Gzip

```mana
// Compress
let compressed = compression::gzip_compress(data)
if compressed.is_ok() {
    let gzipped = compressed.unwrap()
    println(f"Compressed size: {gzipped.len()}")
}

// Decompress
let decompressed = compression::gzip_decompress(gzipped)
```

### Zlib

```mana
// Compress with level
let compressed = compression::zlib_compress(data, compression::Level::Best)

// Decompress
let decompressed = compression::zlib_decompress(compressed.unwrap())
```

### Raw Deflate

```mana
let deflated = compression::deflate(data)
let inflated = compression::inflate(deflated.unwrap())
```

**Compression levels:**
- `Level::None` - Store only (no compression)
- `Level::Fast` - Fastest compression
- `Level::Default` - Balanced (level 6)
- `Level::Best` - Maximum compression (level 9)

---

## Database

### SQLite

Full SQLite database support with prepared statements and transactions.

```mana
// Check availability
if !sqlite::is_available() {
    println(f"SQLite not available: {sqlite::availability_error()}")
    return
}

// Open database (file or :memory:)
let db = sqlite::open("myapp.db")
if db.is_err() {
    println(f"Error: {db.unwrap_err()}")
    return
}
let conn = db.unwrap()

// Create table
conn.execute_update("
    CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY,
        name TEXT NOT NULL,
        email TEXT UNIQUE,
        age INTEGER
    )
")

// Insert with prepared statement
let stmt = conn.prepare("INSERT INTO users (name, email, age) VALUES (?, ?, ?)")
stmt.unwrap()
    .bind(1, "Alice")
    .bind(2, "alice@example.com")
    .bind(3, 30)
    .execute_update()

// Query data
let results = conn.execute("SELECT * FROM users WHERE age > 25")
for row in results.unwrap() {
    let id = row[0].as_integer()
    let name = row["name"].as_text()
    println(f"User {id}: {name}")
}

// Transactions
conn.begin_transaction()
conn.execute_update("UPDATE users SET age = age + 1")
conn.commit()
// Or: conn.rollback()

// Get metadata
let last_id = conn.last_insert_id()
let changes = conn.changes()
```

**Value types:** `sqlite::Type::Null`, `Integer`, `Float`, `Text`, `Blob`

---

## XML

Full XML parsing and generation support.

### Parsing XML

```mana
let xml_str = """
<?xml version="1.0" encoding="UTF-8"?>
<library>
    <book id="1">
        <title>The Rust Programming Language</title>
        <author>Steve Klabnik</author>
    </book>
    <book id="2">
        <title>Programming in Mana</title>
        <author>John Doe</author>
    </book>
</library>
"""

let doc = xml::parse(xml_str)
if doc.is_err() {
    println(f"Parse error: {doc.unwrap_err()}")
    return
}

let root = doc.unwrap().get_root()
println(f"Root tag: {root.tag}")  // "library"

// Access child elements
for book in root.child_elements("book") {
    let id = book.get_attribute("id")
    let title = book.first_child_element("title").text()
    println(f"Book {id}: {title}")
}

// Find elements by path
let authors = root.find("book/author")
for author in authors {
    println(f"Author: {author.text()}")
}
```

### Building XML

```mana
// Using Builder pattern
let xml = xml::Builder("library")
    .attr("version", "1.0")
    .element("book")
        .attr("id", "1")
        .element("title").text("My Book").end()
        .element("author").text("Jane Doe").end()
    .end()
    .to_string()

// Or manually
let doc = xml::create("root")
let root = doc.get_root()
root.set_attribute("version", "1.0")

let child = root.add_element("item")
child.set_text("Hello")

let xml_string = xml::to_string(doc, true)  // pretty print
```

**Element methods:**
- `get_attribute(name, default?) -> str` - Get attribute value
- `set_attribute(name, value)` - Set attribute
- `has_attribute(name) -> bool` - Check attribute exists
- `child_elements(name?) -> Vec<Element>` - Get child elements
- `first_child_element(name?) -> Element?` - Get first child
- `text() -> str` - Get text content
- `set_text(text)` - Set text content
- `add_element(name) -> Element` - Add child element
- `find(path) -> Vec<Element>` - Find by path (e.g., "a/b/c")

---

## JSON

JSON parsing and serialization.

```mana
// Parse JSON
let data = json::parse("""
{
    "name": "Alice",
    "age": 30,
    "skills": ["Rust", "Mana", "C++"]
}
""")

if data.is_ok() {
    let obj = data.unwrap()
    let name = obj["name"].as_string()
    let age = obj["age"].as_int()

    for skill in obj["skills"].as_array() {
        println(f"Skill: {skill.as_string()}")
    }
}

// Create JSON
let mut obj = json::Object::new()
obj.set("name", "Bob")
obj.set("score", 95.5)
obj.set("active", true)

let json_str = json::stringify(obj)
let pretty = json::stringify_pretty(obj, 2)
```

---

## Cryptography

### SHA-256

```mana
let hash = crypto::sha256("Hello, World!")
// "dffd6021bb2bd5b0af676290809ec3a53191dd81c7f70a4b28688a362182986f"
```

### MD5

```mana
let hash = crypto::md5("Hello, World!")
// "65a8e27d8879283831b664bd8b7f0ad4"
```

---

## Regular Expressions

```mana
let re = regex::Regex::new(r"\d+")

// Check if matches
if regex::is_match(r"\d+", "abc123def") {
    println("Contains numbers")
}

// Replace matches
let result = regex::replace(r"\d+", "abc123def456", "X")
// "abcXdefX"

// Find all matches
let matches = re.find_all("a1b2c3")
for m in matches {
    println(f"Found: {m}")
}
```

---

## File System

### Path Utilities

```mana
// Join paths
let full_path = path::join("/home/user", "documents/file.txt")

// Get components
let filename = path::filename("/home/user/file.txt")  // "file.txt"
let dir = path::dirname("/home/user/file.txt")        // "/home/user"
let ext = path::extension("/home/user/file.txt")      // ".txt"
let stem = path::stem("/home/user/file.txt")          // "file"

// Normalize path
let clean = path::normalize("/home/user/../user/./file.txt")
// "/home/user/file.txt"

// Check path properties
let abs = path::is_absolute("/home/user")  // true
let exists = path::exists("/home/user")
let is_file = path::is_file("/home/user/file.txt")
let is_dir = path::is_dir("/home/user")
```

---

## Date and Time

```mana
// Current time
let now = datetime::now_utc()
let local = datetime::now_local()

// Unix timestamp
let ts = datetime::timestamp()

// Access components
println(f"Year: {now.year}")
println(f"Month: {now.month}")
println(f"Day: {now.day}")
println(f"Hour: {now.hour}")
println(f"Minute: {now.minute}")
println(f"Second: {now.second}")

// Format datetime
let formatted = now.format("%Y-%m-%d %H:%M:%S")
// "2024-01-15 14:30:45"
```

**Format specifiers:**
- `%Y` - 4-digit year
- `%m` - Month (01-12)
- `%d` - Day (01-31)
- `%H` - Hour (00-23)
- `%M` - Minute (00-59)
- `%S` - Second (00-59)

---

## UUID

Generate RFC 4122 compliant UUIDs.

```mana
let id = uuid::v4()
// "550e8400-e29b-41d4-a716-446655440000"

// Validate UUID
let valid = uuid::is_valid("550e8400-e29b-41d4-a716-446655440000")  // true
let invalid = uuid::is_valid("not-a-uuid")  // false
```

---

## Environment

Access environment variables and system directories.

```mana
// Get environment variable
let home = env::get("HOME")
if home.is_some() {
    println(f"Home: {home.unwrap()}")
}

// Set environment variable
env::set("MY_VAR", "value")

// Unset environment variable
env::unset("MY_VAR")

// Common directories
let home_dir = env::home_dir()
let temp_dir = env::temp_dir()
let current = env::current_dir()
```

---

## Async/Await

Mana provides an async runtime for writing asynchronous code using futures and promises.

### Basic Usage

```mana
// Create a future that completes immediately with a value
let fut = async::ready(42)

// Create a future that delays execution
let delay = async::sleep_ms(1000)  // Sleep for 1 second

// Block on a future (wait for completion)
let result = async::block_on(async::ready(42))
println(f"Result: {result}")  // Result: 42
```

### Executor

The executor runs async tasks cooperatively.

```mana
// Get the global executor
let executor = async::global_executor()

// Spawn tasks
async::spawn(async::sleep_ms(100))
async::spawn(async::ready("hello"))

// Run all tasks to completion
async::run()

// Run for a limited duration
async::run_for(5000)  // Run for 5 seconds max
```

### Futures

```mana
// Ready future - completes immediately
let ready_fut = async::ready(42)

// Sleep future - completes after delay
let sleep_fut = async::sleep_ms(100)

// Yield - give other tasks a chance to run
let yield_fut = async::yield_now()
```

### Combinators

```mana
// Join - wait for all futures to complete
let results = async::block_on(async::join([
    async::ready(1),
    async::ready(2),
    async::ready(3)
]))
// results = [1, 2, 3]

// Race - return first completed future
let first = async::block_on(async::race([
    async::sleep_ms(100),
    async::sleep_ms(50),  // This wins
    async::sleep_ms(200)
]))

// Timeout - fail if future takes too long
let result = async::block_on(async::timeout(
    async::sleep_ms(1000),
    500  // 500ms timeout
))
match result {
    Ok(value) => println("Completed in time"),
    Err(e) => println("Timed out"),
}
```

### Promises

Promises allow you to manually resolve futures.

```mana
// Create a promise
let promise = async::Promise<i32>::new()

// Get the future associated with the promise
let future = promise.get_future()

// Resolve the promise from another task
promise.resolve(42)

// The future is now ready
let value = async::block_on(future)
```

### Async Channel

Thread-safe channel for async message passing.

```mana
// Create a channel with buffer capacity
let channel = async::AsyncChannel<str>::new(10)

// Send a message (returns a future)
async::spawn(channel.send("hello"))

// Receive a message (returns a future)
let msg = async::block_on(channel.receive())
match msg {
    Some(m) => println(f"Received: {m}"),
    None => println("Channel closed"),
}

// Close the channel
channel.close()

// Check if channel is closed
if channel.is_closed() {
    println("Channel is closed")
}
```

### Async Semaphore

Limit concurrent access to resources.

```mana
// Create semaphore with 3 permits
let sem = async::AsyncSemaphore::new(3)

// Acquire a permit (returns a future)
async::block_on(sem.acquire())

// Do work with the resource...

// Release the permit
sem.release()

// Check available permits
let available = sem.available_permits()
```

### Async Mutex

Async-aware mutual exclusion.

```mana
// Create a mutex with initial value
let mutex = async::AsyncMutex<i32>::new(0)

// Lock the mutex (returns a future)
async::block_on(mutex.lock())

// Access the protected value
let value = mutex.get()
mutex.set(value + 1)

// Unlock the mutex
mutex.unlock()

// Try to lock without blocking
if mutex.try_lock() {
    // Got the lock
    mutex.unlock()
}
```

### Async HTTP

Perform non-blocking HTTP requests.

```mana
// Async GET request
let response = async::block_on(async::http_get("https://api.example.com/data"))
match response {
    Ok(resp) => {
        println(f"Status: {resp.status_code}")
        println(f"Body: {resp.body}")
    },
    Err(e) => println(f"Error: {e}"),
}

// Async POST request
let json = "{\"name\": \"test\"}"
let post_resp = async::block_on(async::http_post(
    "https://api.example.com/users",
    json,
    "application/json"
))

// Async PUT request
let put_resp = async::block_on(async::http_put(
    "https://api.example.com/users/1",
    json
))

// Async DELETE request
let del_resp = async::block_on(async::http_delete("https://api.example.com/users/1"))

// Custom HTTP request
let custom = async::block_on(async::http_request(
    "PATCH",
    "https://api.example.com/resource",
    "{\"field\": \"value\"}",
    "application/json"
))

// Parallel HTTP requests
let futures = [
    async::http_get("https://api1.example.com/data"),
    async::http_get("https://api2.example.com/data"),
    async::http_get("https://api3.example.com/data")
]
let results = async::block_on(async::join(futures))
// All requests completed in parallel
```

**HTTP Response:**

| Field | Type | Description |
|-------|------|-------------|
| `status_code` | `i32` | HTTP status code (200, 404, etc.) |
| `status_text` | `str` | Status message |
| `headers` | `Map<str, str>` | Response headers |
| `body` | `str` | Response body |

**Methods:**
- `is_ok() -> bool` - Status 200-299
- `is_redirect() -> bool` - Status 300-399
- `is_error() -> bool` - Status 400+

### Async File I/O

Perform non-blocking file operations.

```mana
// Async read file
let content = async::block_on(async::read_file("data.txt"))
match content {
    Ok(data) => println(f"Read {data.len()} bytes"),
    Err(e) => println(f"Error: {e}"),
}

// Async write file
let write_result = async::block_on(async::write_file("output.txt", "Hello, World!"))
match write_result {
    Ok(_) => println("File written"),
    Err(e) => println(f"Error: {e}"),
}

// Async append to file
async::block_on(async::append_file("log.txt", "New log entry\n"))

// Async read lines
let lines = async::block_on(async::read_lines("data.csv"))
match lines {
    Ok(l) => {
        for line in l {
            println(line)
        }
    },
    Err(e) => println(f"Error: {e}"),
}

// Async file exists check
let exists = async::block_on(async::file_exists("config.json"))
if exists {
    println("Config file found")
}

// Async delete file
let deleted = async::block_on(async::delete_file("temp.txt"))

// Async copy file
let copied = async::block_on(async::copy_file("source.txt", "backup.txt"))

// Parallel file operations
let read_futures = [
    async::read_file("file1.txt"),
    async::read_file("file2.txt"),
    async::read_file("file3.txt")
]
let contents = async::block_on(async::join(read_futures))
// All files read in parallel
```

**Available Functions:**

| Function | Returns | Description |
|----------|---------|-------------|
| `read_file(path)` | `Future<Result<str, str>>` | Read entire file |
| `write_file(path, content)` | `Future<Result<bool, str>>` | Write to file |
| `append_file(path, content)` | `Future<Result<bool, str>>` | Append to file |
| `read_lines(path)` | `Future<Result<Vec<str>, str>>` | Read file as lines |
| `file_exists(path)` | `Future<bool>` | Check if file exists |
| `delete_file(path)` | `Future<Result<bool, str>>` | Delete file |
| `copy_file(src, dst)` | `Future<Result<bool, str>>` | Copy file |

### Async SQLite

Perform non-blocking database operations.

```mana
// Open database asynchronously
let db_result = async::block_on(async::sqlite_open("app.db"))
match db_result {
    Ok(db) => {
        // Create table
        async::block_on(async::sqlite_execute(db,
            "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY, name TEXT, email TEXT)"
        ))

        // Insert data with parameters
        let insert_result = async::block_on(async::sqlite_execute(
            db,
            "INSERT INTO users (name, email) VALUES (?, ?)",
            ["Alice", "alice@example.com"]
        ))
        match insert_result {
            Ok(affected) => println(f"Inserted {affected} row(s)"),
            Err(e) => println(f"Error: {e}"),
        }

        // Query data
        let query_result = async::block_on(async::sqlite_query(
            db,
            "SELECT id, name, email FROM users WHERE name = ?",
            ["Alice"]
        ))
        match query_result {
            Ok(results) => {
                for row in results {
                    println(f"User: {row.get_text(1)} - {row.get_text(2)}")
                }
            },
            Err(e) => println(f"Error: {e}"),
        }
    },
    Err(e) => println(f"Failed to open database: {e}"),
}
```

**Batch Operations:**

```mana
// Execute multiple statements
let db = async::block_on(async::sqlite_open_memory()).unwrap()

let batch_result = async::block_on(async::sqlite_batch(db, [
    "CREATE TABLE items (id INTEGER PRIMARY KEY, name TEXT)",
    "INSERT INTO items (name) VALUES ('Item 1')",
    "INSERT INTO items (name) VALUES ('Item 2')",
    "INSERT INTO items (name) VALUES ('Item 3')"
]))

match batch_result {
    Ok(affected_counts) => {
        // affected_counts = [0, 1, 1, 1]
        println(f"Batch executed: {affected_counts.len()} statements")
    },
    Err(e) => println(f"Batch failed: {e}"),
}
```

**Transactions:**

```mana
// Execute operations in a transaction
let tx_result = async::block_on(async::sqlite_transaction(db, |db| {
    // All operations here are atomic
    db.prepare("INSERT INTO users (name) VALUES ('Bob')").unwrap().execute_update()?
    db.prepare("INSERT INTO users (name) VALUES ('Carol')").unwrap().execute_update()?

    // Return Ok to commit, Err to rollback
    return Ok(true)
}))

match tx_result {
    Ok(_) => println("Transaction committed"),
    Err(e) => println(f"Transaction rolled back: {e}"),
}
```

**Parallel Queries:**

```mana
// Run multiple queries in parallel
let db = async::block_on(async::sqlite_open("app.db")).unwrap()

let futures = [
    async::sqlite_query(db, "SELECT COUNT(*) FROM users"),
    async::sqlite_query(db, "SELECT COUNT(*) FROM orders"),
    async::sqlite_query(db, "SELECT COUNT(*) FROM products")
]

let results = async::block_on(async::join(futures))
// All queries executed concurrently
```

**Available Functions:**

| Function | Returns | Description |
|----------|---------|-------------|
| `sqlite_open(path)` | `Future<Result<AsyncDatabase, str>>` | Open database file |
| `sqlite_open_memory()` | `Future<Result<AsyncDatabase, str>>` | Open in-memory database |
| `sqlite_query(db, sql, params?)` | `Future<Result<ResultSet, str>>` | Execute SELECT query |
| `sqlite_execute(db, sql, params?)` | `Future<Result<int, str>>` | Execute INSERT/UPDATE/DELETE |
| `sqlite_batch(db, statements)` | `Future<Result<Vec<int>, str>>` | Execute multiple statements |
| `sqlite_transaction(db, func)` | `Future<Result<bool, str>>` | Run in transaction |

### Async WebSocket

Non-blocking WebSocket communication for real-time applications.

```mana
// Connect to a WebSocket server
let ws_result = async::block_on(async::ws_connect("wss://echo.websocket.org"))
match ws_result {
    Ok(ws) => {
        // Send a message
        async::block_on(async::ws_send(ws, "Hello, WebSocket!"))

        // Receive a message
        let msg_result = async::block_on(async::ws_receive(ws))
        match msg_result {
            Ok(msg) => {
                match msg.type {
                    MessageType::Text => println(f"Received: {msg.data}"),
                    MessageType::Binary => println(f"Binary: {msg.data.len()} bytes"),
                    MessageType::Close => println("Connection closed"),
                    _ => {}
                }
            },
            Err(e) => println(f"Receive error: {e}"),
        }

        // Close the connection
        async::ws_close(ws)
    },
    Err(e) => println(f"Connection failed: {e}"),
}
```

**Binary Messages:**

```mana
let ws = async::block_on(async::ws_connect("wss://example.com/binary")).unwrap()

// Send binary data
let binary_data = "\x00\x01\x02\x03"
async::block_on(async::ws_send_binary(ws, binary_data))

// Receive continues to work for both text and binary
let msg = async::block_on(async::ws_receive(ws)).unwrap()
```

**Receive Loop:**

```mana
// Receive multiple messages until close
let ws = async::block_on(async::ws_connect("wss://stream.example.com")).unwrap()

// Receive up to 100 messages
let messages = async::block_on(async::ws_receive_all(ws, 100))
match messages {
    Ok(msgs) => {
        for msg in msgs {
            println(f"Message: {msg.data}")
        }
    },
    Err(e) => println(f"Error: {e}"),
}

// Or receive until connection closes (0 = unlimited)
let all_messages = async::block_on(async::ws_receive_all(ws, 0))
```

**Real-Time Chat Example:**

```mana
fn main() {
    let ws = async::block_on(async::ws_connect("wss://chat.example.com")).unwrap()

    // Spawn a task to receive messages
    async::spawn(async {
        loop {
            let msg = async::block_on(async::ws_receive(ws))
            match msg {
                Ok(m) => println(f"[Chat] {m.data}"),
                Err(_) => break,
            }
        }
    })

    // Send messages
    async::block_on(async::ws_send(ws, "Hello everyone!"))
    async::block_on(async::ws_send(ws, "How is everyone?"))

    // Run the event loop
    async::run_for(30000)  // Run for 30 seconds

    async::ws_close(ws)
}
```

**Available Functions:**

| Function | Returns | Description |
|----------|---------|-------------|
| `ws_connect(url)` | `Future<Result<AsyncWebSocket, str>>` | Connect to WebSocket server |
| `ws_send(ws, message)` | `Future<Result<bool, str>>` | Send text message |
| `ws_send_binary(ws, data)` | `Future<Result<bool, str>>` | Send binary data |
| `ws_receive(ws)` | `Future<Result<Message, str>>` | Receive one message |
| `ws_receive_all(ws, max?)` | `Future<Result<Vec<Message>, str>>` | Receive multiple messages |
| `ws_close(ws, code?, reason?)` | `void` | Close connection |

**Message Types:**

| Type | Description |
|------|-------------|
| `Text` | UTF-8 text message |
| `Binary` | Binary data |
| `Ping` | Ping frame (handled automatically) |
| `Pong` | Pong frame (handled automatically) |
| `Close` | Connection close |

### Async TCP Sockets

Non-blocking TCP client and server operations.

**TCP Client:**

```mana
// Connect to a TCP server
let socket_result = async::block_on(async::tcp_connect("example.com", 80))
match socket_result {
    Ok(socket) => {
        // Send HTTP request
        let request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"
        async::block_on(async::tcp_send(socket, request))

        // Receive response
        let response = async::block_on(async::tcp_recv(socket, 4096))
        match response {
            Ok(data) => println(f"Received: {data.len()} bytes"),
            Err(e) => println(f"Receive error: {e}"),
        }

        // Close connection
        async::tcp_close(socket)
    },
    Err(e) => println(f"Connection failed: {e}"),
}
```

**TCP Server:**

```mana
// Create a TCP server
let listener_result = async::block_on(async::tcp_listen(8080, "0.0.0.0", 10))
match listener_result {
    Ok(listener) => {
        println(f"Server listening on port {listener.port()}")

        // Accept connections in a loop
        loop {
            let client_result = async::block_on(async::tcp_accept(listener))
            match client_result {
                Ok(client) => {
                    // Handle client in a spawned task
                    async::spawn(async {
                        let data = async::block_on(async::tcp_recv(client, 1024))
                        match data {
                            Ok(request) => {
                                println(f"Received: {request}")
                                async::block_on(async::tcp_send(client, "HTTP/1.1 200 OK\r\n\r\nHello!"))
                            },
                            Err(e) => println(f"Error: {e}"),
                        }
                        async::tcp_close(client)
                    })
                },
                Err(e) => println(f"Accept failed: {e}"),
            }
        }
    },
    Err(e) => println(f"Listen failed: {e}"),
}
```

**Read Until Delimiter:**

```mana
// Read until a specific delimiter (useful for line-based protocols)
let socket = async::block_on(async::tcp_connect("server.com", 1234)).unwrap()

// Read until newline
let line = async::block_on(async::tcp_read_until(socket, "\n"))
match line {
    Ok(data) => println(f"Line: {data}"),
    Err(e) => println(f"Error: {e}"),
}

// Read HTTP headers (until \r\n\r\n)
let headers = async::block_on(async::tcp_read_until(socket, "\r\n\r\n"))
```

**Read Exact Bytes:**

```mana
// Read exactly N bytes (useful for binary protocols)
let socket = async::block_on(async::tcp_connect("server.com", 1234)).unwrap()

// Read a 4-byte header
let header = async::block_on(async::tcp_read_exact(socket, 4)).unwrap()
let length = parse_header_length(header)

// Read the body
let body = async::block_on(async::tcp_read_exact(socket, length)).unwrap()
```

**Parallel Connections:**

```mana
// Connect to multiple servers in parallel
let futures = [
    async::tcp_connect("server1.com", 80),
    async::tcp_connect("server2.com", 80),
    async::tcp_connect("server3.com", 80)
]

let results = async::block_on(async::join(futures))
// All connections attempted concurrently
```

**Echo Server Example:**

```mana
fn main() {
    let listener = async::block_on(async::tcp_listen(9000)).unwrap()
    println("Echo server running on port 9000")

    loop {
        match async::block_on(async::tcp_accept(listener)) {
            Ok(client) => {
                async::spawn(async {
                    loop {
                        match async::block_on(async::tcp_recv(client, 1024)) {
                            Ok(data) => {
                                if data.is_empty() { break }
                                async::block_on(async::tcp_send(client, data))
                            },
                            Err(_) => break,
                        }
                    }
                    async::tcp_close(client)
                })
            },
            Err(_) => break,
        }
    }
}
```

**Available Functions:**

| Function | Returns | Description |
|----------|---------|-------------|
| `tcp_connect(host, port)` | `Future<Result<AsyncTcpSocket, str>>` | Connect to server |
| `tcp_send(socket, data)` | `Future<Result<size_t, str>>` | Send data |
| `tcp_recv(socket, max_size?)` | `Future<Result<str, str>>` | Receive data |
| `tcp_listen(port, addr?, backlog?)` | `Future<Result<AsyncTcpListener, str>>` | Create server |
| `tcp_accept(listener)` | `Future<Result<AsyncTcpSocket, str>>` | Accept connection |
| `tcp_read_until(socket, delim, max?)` | `Future<Result<str, str>>` | Read until delimiter |
| `tcp_read_exact(socket, bytes)` | `Future<Result<str, str>>` | Read exact bytes |
| `tcp_close(socket)` | `void` | Close socket |
| `tcp_listener_close(listener)` | `void` | Close listener |

### Async UDP Sockets

Non-blocking UDP datagram operations.

**UDP Server (Receiver):**

```mana
// Create a bound UDP socket to receive datagrams
let socket_future = async::udp_bind(8080)
match async::block_on(socket_future) {
    Ok(socket) => {
        println("UDP server listening on port 8080")

        // Receive a single datagram
        let recv_future = async::udp_recv_from(socket)
        match async::block_on(recv_future) {
            Ok(datagram) => {
                println(f"Received: {datagram.data}")
                println(f"From: {datagram.sender_addr}:{datagram.sender_port}")

                // Echo back
                let send_future = async::udp_send_to(
                    socket,
                    f"Echo: {datagram.data}",
                    datagram.sender_addr,
                    datagram.sender_port
                )
                async::block_on(send_future)
            },
            Err(e) => println(f"Receive error: {e}"),
        }

        async::udp_close(socket)
    },
    Err(e) => println(f"Bind failed: {e}"),
}
```

**UDP Client (Sender):**

```mana
// Create an unbound socket for sending
let socket_future = async::udp_create()
match async::block_on(socket_future) {
    Ok(socket) => {
        // Send a message
        let send_future = async::udp_send_to(socket, "Hello UDP!", "127.0.0.1", 8080)
        match async::block_on(send_future) {
            Ok(bytes) => println(f"Sent {bytes} bytes"),
            Err(e) => println(f"Send error: {e}"),
        }

        async::udp_close(socket)
    },
    Err(e) => println(f"Create failed: {e}"),
}
```

**Receiving Multiple Datagrams:**

```mana
let socket_future = async::udp_bind(8080)
match async::block_on(socket_future) {
    Ok(socket) => {
        // Receive up to 10 datagrams
        let recv_future = async::udp_recv_loop(socket, 10)
        match async::block_on(recv_future) {
            Ok(datagrams) => {
                for datagram in datagrams {
                    println(f"From {datagram.sender_addr}:{datagram.sender_port}: {datagram.data}")
                }
            },
            Err(e) => println(f"Error: {e}"),
        }

        async::udp_close(socket)
    },
    Err(e) => println(f"Bind failed: {e}"),
}
```

**UDP Echo Server:**

```mana
fn main() {
    let socket_result = async::block_on(async::udp_bind(8080))
    match socket_result {
        Ok(socket) => {
            println("UDP Echo Server running on port 8080")

            // Continuous echo loop
            while true {
                let recv = async::block_on(async::udp_recv_from(socket))
                match recv {
                    Ok(datagram) => {
                        println(f"[{datagram.sender_addr}:{datagram.sender_port}] {datagram.data}")

                        // Echo back the message
                        async::block_on(async::udp_send_to(
                            socket,
                            datagram.data,
                            datagram.sender_addr,
                            datagram.sender_port
                        ))
                    },
                    Err(e) => {
                        println(f"Error: {e}")
                        break
                    },
                }
            }

            async::udp_close(socket)
        },
        Err(e) => println(f"Failed to bind: {e}"),
    }
}
```

**UDP Functions:**

| Function | Return Type | Description |
|----------|-------------|-------------|
| `udp_bind(port, addr?)` | `Future<Result<AsyncUdpSocket, str>>` | Bind socket to port |
| `udp_create()` | `Future<Result<AsyncUdpSocket, str>>` | Create unbound socket |
| `udp_send_to(socket, data, host, port)` | `Future<Result<i64, str>>` | Send datagram |
| `udp_recv_from(socket, max_size?)` | `Future<Result<UdpDatagram, str>>` | Receive datagram |
| `udp_recv_loop(socket, count, max_size?)` | `Future<Result<Vec<UdpDatagram>, str>>` | Receive multiple |
| `udp_close(socket)` | `void` | Close socket |

**UdpDatagram Structure:**

| Field | Type | Description |
|-------|------|-------------|
| `data` | `str` | The datagram payload |
| `sender_addr` | `str` | Sender's IP address |
| `sender_port` | `i32` | Sender's port number |

### Async Timers and Sleep

Non-blocking sleep and timer operations for async programming.

**Basic Sleep:**

```mana
// Sleep for a duration (non-blocking in async context)
async::block_on(async::sleep_ms(1000))    // Sleep 1 second
async::block_on(async::sleep_secs(2))     // Sleep 2 seconds
async::block_on(async::sleep_micros(500)) // Sleep 500 microseconds
async::block_on(async::sleep_nanos(1000)) // Sleep 1000 nanoseconds

// Yield control to other tasks without sleeping
async::block_on(async::yield_now())
```

**Interval Timer (Repeating):**

```mana
// Create an interval timer that ticks every 100ms
let timer = async::interval(100)

// Process 5 ticks
for i in 0..5 {
    let tick = async::block_on(timer)
    println(f"Tick #{tick.tick_count} at {tick.elapsed_ms}ms")
}

// Stop the timer
timer.stop()

// Or create with a limit
let limited_timer = async::interval_with_limit(100, 10)  // 10 ticks max
while !limited_timer.is_stopped() {
    let tick = async::block_on(limited_timer)
    println(f"Tick #{tick.tick_count}")
}
```

**Deadline Timer:**

```mana
// Fire after 500ms
let deadline = async::deadline(500)
let elapsed = async::block_on(deadline)
println(f"Deadline reached after {elapsed}ms")

// Fire at a specific Unix timestamp (milliseconds)
let target_time = datetime::timestamp_ms() + 1000
let deadline_at = async::deadline_at(target_time)
async::block_on(deadline_at)
println("Target time reached!")

// Cancel a deadline
let deadline = async::deadline(5000)
deadline.cancel()  // Returns -1 when polled
```

**Debounce:**

```mana
// Only fires if no reset within 200ms
let debounce = async::debounce(200)

// Simulate rapid input
debounce.reset()  // Reset timer
debounce.reset()  // Reset again
// ... after 200ms of no resets, it fires
async::block_on(debounce)
println("Debounced action executed")
```

**Throttle:**

```mana
// Limit actions to once per 100ms
let throttle = async::throttle(100)

for i in 0..10 {
    if throttle.try_acquire() {
        println(f"Action {i} executed")
    } else {
        println(f"Action {i} throttled, wait {throttle.remaining_ms()}ms")
    }
    async::block_on(async::sleep_ms(30))
}
```

**Stopwatch:**

```mana
// Measure elapsed time
let sw = async::stopwatch_started()  // Start immediately

// Do some work
async::block_on(async::sleep_ms(100))

println(f"Elapsed: {sw.elapsed_ms()}ms")
println(f"Elapsed: {sw.elapsed_micros()}us")
println(f"Elapsed: {sw.elapsed_secs()}s")

// Pause and resume
sw.stop()
async::block_on(async::sleep_ms(50))  // Not counted
sw.start()
async::block_on(async::sleep_ms(50))  // Counted

println(f"Total: {sw.elapsed_ms()}ms")  // ~150ms, not 200ms

// Reset
sw.reset()      // Reset to 0, keeps running state
sw.restart()    // Reset to 0 and start running
```

**Timer with Callbacks:**

```mana
let timer = async::timer()

// One-shot timeout
timer.set_timeout(1000, || {
    println("Timeout fired after 1 second!")
})

// Repeating interval
timer.set_interval(500, || {
    println("Interval tick!")
}, 5)  // Max 5 times (0 = unlimited)

// Cancel timer
timer.cancel()
```

**Delayed Value:**

```mana
// Return a value after a delay
let future = async::delay(100, "Hello after 100ms")
let message = async::block_on(future)
println(message)  // "Hello after 100ms"
```

**Practical Examples:**

```mana
// Rate-limited API calls
fn rate_limited_fetch(urls: Vec<str>) {
    let throttle = async::throttle(100)  // Max 10 requests/second

    for url in urls {
        // Wait for throttle
        while !throttle.try_acquire() {
            async::block_on(async::sleep_ms(10))
        }

        // Make request
        let response = async::block_on(async::http_get(url))
        match response {
            Ok(r) => println(f"Got {url}: {r.status_code}"),
            Err(e) => println(f"Error: {e}"),
        }
    }
}

// Timeout wrapper
fn fetch_with_timeout(url: str, timeout_ms: i64) -> Result<str, str> {
    let deadline = async::deadline(timeout_ms)
    let request = async::http_get(url)

    // Race between request and deadline
    // (simplified - use async::timeout in practice)
    while !request.is_ready() && deadline.remaining_ms() > 0 {
        async::block_on(async::sleep_ms(10))
    }

    if request.is_ready() {
        match async::block_on(request) {
            Ok(r) => Ok(r.body),
            Err(e) => Err(e),
        }
    } else {
        Err("Request timed out")
    }
}

// Periodic task with stopwatch
fn periodic_task() {
    let sw = async::stopwatch()
    let interval = async::interval(1000)

    while true {
        sw.restart()

        // Do work
        process_data()

        let tick = async::block_on(interval)
        println(f"Iteration {tick.tick_count}, work took {sw.elapsed_ms()}ms")
    }
}
```

**Timer Functions:**

| Function | Return Type | Description |
|----------|-------------|-------------|
| `sleep_ms(ms)` | `Future<void>` | Sleep for milliseconds |
| `sleep_secs(secs)` | `Future<void>` | Sleep for seconds |
| `sleep_micros(us)` | `Future<void>` | Sleep for microseconds |
| `sleep_nanos(ns)` | `Future<void>` | Sleep for nanoseconds |
| `yield_now()` | `Future<void>` | Yield to other tasks |
| `interval(ms)` | `IntervalFuture` | Repeating timer |
| `interval_with_limit(ms, max)` | `IntervalFuture` | Limited repeating timer |
| `deadline(ms)` | `DeadlineFuture` | Fire after delay |
| `deadline_at(unix_ms)` | `DeadlineFuture` | Fire at specific time |
| `debounce(ms)` | `DebounceFuture` | Debounced timer |
| `throttle(ms)` | `Throttle` | Rate limiter |
| `stopwatch()` | `Stopwatch` | Create stopped stopwatch |
| `stopwatch_started()` | `Stopwatch` | Create running stopwatch |
| `timer()` | `Timer` | General-purpose timer |
| `delay(ms, value)` | `Future<T>` | Return value after delay |

**IntervalTick Structure:**

| Field | Type | Description |
|-------|------|-------------|
| `tick_count` | `i64` | Number of ticks since start |
| `elapsed_ms` | `i64` | Total elapsed time in ms |

**Stopwatch Methods:**

| Method | Return Type | Description |
|--------|-------------|-------------|
| `start()` | `void` | Start/resume timing |
| `stop()` | `void` | Pause timing |
| `reset()` | `void` | Reset to 0 |
| `restart()` | `void` | Reset and start |
| `elapsed_nanos()` | `i64` | Get elapsed nanoseconds |
| `elapsed_micros()` | `i64` | Get elapsed microseconds |
| `elapsed_ms()` | `i64` | Get elapsed milliseconds |
| `elapsed_secs()` | `f64` | Get elapsed seconds |
| `is_running()` | `bool` | Check if running |

### Async Process Execution

Non-blocking process spawning, execution, and management.

**Execute and Wait:**

```mana
// Simple command execution
let result = async::block_on(async::exec("ls", ["-la"]))
match result {
    Ok(output) => {
        println(f"Exit code: {output.exit_code}")
        println(f"Stdout: {output.stdout_data}")
        println(f"Stderr: {output.stderr_data}")
    },
    Err(e) => println(f"Failed to execute: {e}"),
}

// Execute shell command (uses cmd.exe on Windows, /bin/sh on Unix)
let result = async::block_on(async::shell("echo Hello World"))
match result {
    Ok(output) => println(output.stdout_data),
    Err(e) => println(f"Error: {e}"),
}
```

**Command Builder (Fluent API):**

```mana
// Build command with options
let cmd = async::command("python")
    .arg("-c")
    .arg("print('Hello from Python')")
    .working_dir("/home/user")
    .env("MY_VAR", "value")
    .capture_stdout(true)
    .capture_stderr(true)

let result = async::block_on(cmd.output())
match result {
    Ok(output) => {
        if output.success() {
            println(output.stdout_data)
        } else {
            println(f"Failed with code {output.exit_code}")
        }
    },
    Err(e) => println(f"Error: {e}"),
}
```

**Spawn Without Waiting:**

```mana
// Spawn process and get handle
let spawn_result = async::block_on(async::spawn_process("long_running_task", []))
match spawn_result {
    Ok(process) => {
        println("Process started")

        // Do other work while process runs
        async::block_on(async::sleep_ms(1000))

        // Check if still running
        if async::process_running(process) {
            println("Still running...")
        }

        // Wait for completion
        let output = async::block_on(async::wait_process(process))
        match output {
            Ok(result) => println(f"Finished with code {result.exit_code}"),
            Err(e) => println(f"Wait failed: {e}"),
        }
    },
    Err(e) => println(f"Spawn failed: {e}"),
}
```

**Provide Input via Stdin:**

```mana
// Send data to process stdin
let cmd = async::command("python")
    .arg("-c")
    .arg("import sys; print(sys.stdin.read().upper())")
    .stdin_data("hello world")

let result = async::block_on(cmd.output())
match result {
    Ok(output) => println(output.stdout_data),  // "HELLO WORLD"
    Err(e) => println(f"Error: {e}"),
}
```

**Kill a Process:**

```mana
let spawn_result = async::block_on(async::spawn_process("sleep", ["60"]))
match spawn_result {
    Ok(process) => {
        // Kill after 1 second
        async::block_on(async::sleep_ms(1000))
        async::kill_process(process)
        println("Process killed")
    },
    Err(e) => println(f"Error: {e}"),
}
```

**Run Multiple Processes in Parallel:**

```mana
fn main() {
    // Spawn multiple processes
    let p1 = async::exec("echo", ["Process 1"])
    let p2 = async::exec("echo", ["Process 2"])
    let p3 = async::exec("echo", ["Process 3"])

    // Wait for all to complete
    let r1 = async::block_on(p1)
    let r2 = async::block_on(p2)
    let r3 = async::block_on(p3)

    // Print results
    for (i, r) in [(1, r1), (2, r2), (3, r3)] {
        match r {
            Ok(output) => println(f"P{i}: {output.stdout_data}"),
            Err(e) => println(f"P{i} failed: {e}"),
        }
    }
}
```

**Practical Examples:**

```mana
// Git operations
fn git_status() -> Result<str, str> {
    let result = async::block_on(async::exec("git", ["status", "--short"]))
    match result {
        Ok(output) => {
            if output.success() {
                Ok(output.stdout_data)
            } else {
                Err(output.stderr_data)
            }
        },
        Err(e) => Err(e),
    }
}

// Run npm install with timeout
fn npm_install_with_timeout(timeout_ms: i64) -> Result<str, str> {
    let cmd = async::command("npm")
        .arg("install")
        .working_dir("./my-project")

    let process_result = async::block_on(cmd.spawn())
    match process_result {
        Ok(process) => {
            let deadline = async::deadline(timeout_ms)

            while async::process_running(process) && deadline.remaining_ms() > 0 {
                async::block_on(async::sleep_ms(100))
            }

            if async::process_running(process) {
                async::kill_process(process)
                Err("npm install timed out")
            } else {
                let output = async::block_on(async::wait_process(process))
                match output {
                    Ok(o) => {
                        if o.success() { Ok(o.stdout_data) }
                        else { Err(o.stderr_data) }
                    },
                    Err(e) => Err(e),
                }
            }
        },
        Err(e) => Err(e),
    }
}

// Compile and run
fn compile_and_run(source: str) -> Result<str, str> {
    // Compile
    let compile = async::block_on(async::exec("gcc", ["-o", "temp", source]))
    match compile {
        Ok(output) => {
            if !output.success() {
                return Err(f"Compilation failed: {output.stderr_data}")
            }
        },
        Err(e) => return Err(f"Compiler error: {e}"),
    }

    // Run
    let run = async::block_on(async::exec("./temp", []))
    match run {
        Ok(output) => Ok(output.stdout_data),
        Err(e) => Err(f"Runtime error: {e}"),
    }
}
```

**Process Functions:**

| Function | Return Type | Description |
|----------|-------------|-------------|
| `exec(program, args?)` | `Future<Result<ProcessOutput, str>>` | Execute and wait |
| `shell(command)` | `Future<Result<ProcessOutput, str>>` | Execute shell command |
| `command(program)` | `Command` | Create command builder |
| `spawn_process(program, args?)` | `Future<Result<AsyncProcess, str>>` | Spawn without waiting |
| `wait_process(process)` | `Future<Result<ProcessOutput, str>>` | Wait for process |
| `kill_process(process)` | `void` | Kill process |
| `process_running(process)` | `bool` | Check if running |

**Command Builder Methods:**

| Method | Return Type | Description |
|--------|-------------|-------------|
| `arg(value)` | `Command` | Add single argument |
| `args(values)` | `Command` | Add multiple arguments |
| `working_dir(path)` | `Command` | Set working directory |
| `env(key, value)` | `Command` | Set environment variable |
| `stdin_data(data)` | `Command` | Set stdin input |
| `capture_stdout(bool)` | `Command` | Capture stdout (default: true) |
| `capture_stderr(bool)` | `Command` | Capture stderr (default: true) |
| `output()` | `Future<Result<ProcessOutput, str>>` | Execute and wait |
| `spawn()` | `Future<Result<AsyncProcess, str>>` | Spawn without waiting |

**ProcessOutput Structure:**

| Field | Type | Description |
|-------|------|-------------|
| `exit_code` | `i32` | Process exit code |
| `stdout_data` | `str` | Captured stdout |
| `stderr_data` | `str` | Captured stderr |
| `success()` | `bool` | True if exit_code == 0 |

### Async DNS Resolution

Non-blocking DNS hostname resolution and reverse lookup.

**Basic DNS Lookup:**

```mana
// Resolve hostname to IP address (returns first IPv4)
let result = async::block_on(async::dns_lookup_ip("google.com"))
match result {
    Ok(ip) => println(f"IP: {ip}"),
    Err(e) => println(f"DNS error: {e}"),
}

// Prefer IPv6
let result = async::block_on(async::dns_lookup_ipv6("google.com"))
match result {
    Ok(ip) => println(f"IPv6: {ip}"),
    Err(e) => println(f"DNS error: {e}"),
}
```

**Full DNS Lookup (All Records):**

```mana
// Get all IP addresses and records
let result = async::block_on(async::dns_lookup("google.com"))
match result {
    Ok(dns) => {
        println(f"Hostname: {dns.hostname}")

        // IPv4 addresses
        println("IPv4 addresses:")
        for ip in dns.ipv4_addresses {
            println(f"  {ip}")
        }

        // IPv6 addresses
        println("IPv6 addresses:")
        for ip in dns.ipv6_addresses {
            println(f"  {ip}")
        }

        // First IP (convenience)
        println(f"First IP: {dns.first_ip()}")

        // Check availability
        if dns.has_ipv4() {
            println("Has IPv4 addresses")
        }
        if dns.has_ipv6() {
            println("Has IPv6 addresses")
        }
    },
    Err(e) => println(f"DNS error: {e}"),
}
```

**Reverse DNS Lookup:**

```mana
// Get hostname from IP address
let result = async::block_on(async::dns_reverse("8.8.8.8"))
match result {
    Ok(hostname) => println(f"Hostname: {hostname}"),  // dns.google
    Err(e) => println(f"Reverse lookup failed: {e}"),
}

// Works with IPv6 too
let result = async::block_on(async::dns_reverse("2001:4860:4860::8888"))
match result {
    Ok(hostname) => println(f"Hostname: {hostname}"),
    Err(e) => println(f"Error: {e}"),
}
```

**Batch DNS Lookup:**

```mana
// Resolve multiple hostnames at once
let hostnames = ["google.com", "github.com", "cloudflare.com"]
let result = async::block_on(async::dns_lookup_batch(hostnames))
match result {
    Ok(results) => {
        for dns in results {
            println(f"{dns.hostname}: {dns.first_ip()}")
        }
    },
    Err(e) => println(f"Batch lookup failed: {e}"),
}
```

**IP Address Validation:**

```mana
// Check if string is a valid IP address
if async::is_ip_address("192.168.1.1") {
    println("Valid IP address")
}

// Check specific IP version
if async::is_ipv4_address("192.168.1.1") {
    println("Valid IPv4")
}

if async::is_ipv6_address("::1") {
    println("Valid IPv6")
}

// Useful for conditional resolution
fn resolve_or_use(host: str) -> Result<str, str> {
    if async::is_ip_address(host) {
        // Already an IP, return as-is
        Ok(host)
    } else {
        // Need to resolve hostname
        async::block_on(async::dns_lookup_ip(host))
    }
}
```

**Practical Examples:**

```mana
// DNS-based load balancing (round-robin)
fn get_server_ip(service: str) -> Result<str, str> {
    let result = async::block_on(async::dns_lookup(service))
    match result {
        Ok(dns) => {
            if dns.ipv4_addresses.len() > 0 {
                // Random selection from available IPs
                let idx = random::int(0, dns.ipv4_addresses.len())
                Ok(dns.ipv4_addresses[idx])
            } else {
                Err("No addresses found")
            }
        },
        Err(e) => Err(e),
    }
}

// Health check with DNS
fn check_dns_health(hostname: str) -> bool {
    let result = async::block_on(async::dns_lookup_ip(hostname))
    result.is_ok()
}

// Connect with DNS resolution
fn connect_to_service(hostname: str, port: i32) -> Result<TcpSocket, str> {
    // Resolve hostname first
    let ip_result = async::block_on(async::dns_lookup_ip(hostname))
    match ip_result {
        Ok(ip) => {
            // Connect using resolved IP
            let conn = async::block_on(async::tcp_connect(ip, port))
            match conn {
                Ok(socket) => Ok(socket),
                Err(e) => Err(f"Connection failed: {e}"),
            }
        },
        Err(e) => Err(f"DNS resolution failed: {e}"),
    }
}

// Parallel DNS resolution
fn resolve_all_parallel(hostnames: Vec<str>) -> Vec<(str, str)> {
    let mut results = []

    // Start all lookups
    let futures = []
    for host in hostnames {
        futures.push((host, async::dns_lookup_ip(host)))
    }

    // Collect results
    for (host, future) in futures {
        match async::block_on(future) {
            Ok(ip) => results.push((host, ip)),
            Err(_) => results.push((host, "failed")),
        }
    }

    results
}
```

**DNS Functions:**

| Function | Return Type | Description |
|----------|-------------|-------------|
| `dns_lookup(hostname)` | `Future<Result<DnsResult, str>>` | Full DNS lookup |
| `dns_lookup_ip(hostname)` | `Future<Result<str, str>>` | Get first IPv4 address |
| `dns_lookup_ipv6(hostname)` | `Future<Result<str, str>>` | Get first IPv6 address |
| `dns_reverse(ip)` | `Future<Result<str, str>>` | Reverse lookup |
| `dns_lookup_batch(hostnames)` | `Future<Result<Vec<DnsResult>, str>>` | Batch lookup |
| `is_ip_address(str)` | `bool` | Check if valid IP |
| `is_ipv4_address(str)` | `bool` | Check if valid IPv4 |
| `is_ipv6_address(str)` | `bool` | Check if valid IPv6 |

**DnsResult Structure:**

| Field/Method | Type | Description |
|--------------|------|-------------|
| `hostname` | `str` | Original hostname |
| `ipv4_addresses` | `Vec<str>` | All IPv4 addresses |
| `ipv6_addresses` | `Vec<str>` | All IPv6 addresses |
| `records` | `Vec<DnsRecord>` | All DNS records |
| `has_ipv4()` | `bool` | Has IPv4 addresses |
| `has_ipv6()` | `bool` | Has IPv6 addresses |
| `first_ip()` | `str` | First IP (IPv4 preferred) |

**DnsRecord Structure:**

| Field | Type | Description |
|-------|------|-------------|
| `type` | `DnsRecordType` | Record type (A, AAAA, etc.) |
| `name` | `str` | Record name |
| `value` | `str` | Record value |
| `ttl` | `i32` | Time to live |

**DnsRecordType Enum:**

| Variant | Description |
|---------|-------------|
| `A` | IPv4 address |
| `AAAA` | IPv6 address |
| `CNAME` | Canonical name |
| `MX` | Mail exchange |
| `TXT` | Text record |
| `PTR` | Pointer (reverse) |

### Task States

Tasks can be in one of these states:

| State | Description |
|-------|-------------|
| `Pending` | Task has not started |
| `Running` | Task is currently executing |
| `Completed` | Task finished successfully |
| `Failed` | Task encountered an error |
| `Cancelled` | Task was cancelled |

### Full Example

```mana
fn main() {
    // Simulate fetching data from multiple sources
    let fetch1 = async::sleep_ms(100)  // Simulates API call
    let fetch2 = async::sleep_ms(150)  // Simulates DB query
    let fetch3 = async::sleep_ms(50)   // Simulates cache lookup

    // Wait for all to complete
    async::block_on(async::join([fetch1, fetch2, fetch3]))
    println("All data fetched!")

    // Or race to get the fastest result
    let winner = async::block_on(async::race([
        async::sleep_ms(100),
        async::sleep_ms(50),   // Fastest
        async::sleep_ms(200)
    ]))
    println("Got first result!")

    // Timeout example
    let result = async::block_on(async::timeout(
        async::sleep_ms(500),
        100  // 100ms timeout - will fail
    ))
    match result {
        Ok(_) => println("Completed"),
        Err(e) => println(f"Error: {e}"),  // "Operation timed out"
    }
}
```

### Async File Watcher

Watch files and directories for changes with platform-native implementations.

**Basic Usage:**

```mana
use async

fn main() {
    // Watch a directory for any changes
    let watcher = async::block_on(async::watch_directory("./src"))
    match watcher {
        Ok(w) => {
            println("Watching ./src for changes...")

            // Wait for events with 10 second timeout
            let events = async::block_on(async::wait_events(w, 10000))
            for event in events {
                println(f"Event: {event.type_string()} - {event.path}")
            }

            // Stop watching when done
            async::stop_watching(w)
        }
        Err(e) => println(f"Failed to start watcher: {e}")
    }
}
```

**Watching a Single File:**

```mana
use async

fn main() {
    // Watch a specific file
    let watcher = async::block_on(async::watch_file("config.json"))
    match watcher {
        Ok(w) => {
            // Wait for file to be modified
            let event = async::block_on(async::wait_for_modify(w))
            println(f"Config modified at {event.timestamp}")

            async::stop_watching(w)
        }
        Err(e) => println(f"Error: {e}")
    }
}
```

**Recursive Directory Watching:**

```mana
use async

fn main() {
    // Watch directory and all subdirectories
    let watcher = async::block_on(async::watch_directory("./project", true))  // true = recursive
    match watcher {
        Ok(w) => {
            // Continuously poll for events
            while async::is_watching(w) {
                let events = async::block_on(async::poll_events(w))
                for event in events {
                    match event.type {
                        FileEventType::Created => println(f"Created: {event.path}"),
                        FileEventType::Modified => println(f"Modified: {event.path}"),
                        FileEventType::Deleted => println(f"Deleted: {event.path}"),
                        FileEventType::Renamed => println(f"Renamed: {event.old_path} -> {event.path}"),
                    }
                }
                async::block_on(async::sleep_ms(100))
            }
        }
        Err(e) => println(f"Error: {e}")
    }
}
```

**Custom Configuration:**

```mana
use async

fn main() {
    // Create custom watcher config
    let config = FileWatcherConfig {
        recursive: true,
        watch_creates: true,
        watch_modifies: true,
        watch_deletes: false,  // Ignore deletions
        watch_renames: true,
        poll_interval_ms: 50,  // Faster polling
    }

    let watcher = async::block_on(async::watch_with_config("./assets", config))
    // ... use watcher
}
```

**Waiting for Specific Events:**

```mana
use async

fn main() {
    let watcher = async::block_on(async::watch_directory("./data"))
    match watcher {
        Ok(w) => {
            // Wait specifically for file creation
            println("Waiting for new file...")
            let event = async::block_on(async::wait_for_create(w, 30000))  // 30s timeout
            if event.path != "" {
                println(f"New file created: {event.path}")
            } else {
                println("Timeout - no file created")
            }

            async::stop_watching(w)
        }
        Err(e) => println(f"Error: {e}")
    }
}
```

**File Watcher Functions:**

| Function | Description |
|----------|-------------|
| `watch_path(path, recursive?)` | Watch file or directory |
| `watch_file(path)` | Watch a single file |
| `watch_directory(path, recursive?)` | Watch a directory |
| `watch_with_config(path, config)` | Watch with custom configuration |
| `poll_events(watcher)` | Get pending events (non-blocking) |
| `wait_events(watcher, timeout_ms?)` | Wait for events with optional timeout |
| `wait_for_event(watcher, type, timeout_ms?)` | Wait for specific event type |
| `wait_for_create(watcher, timeout_ms?)` | Wait for file creation |
| `wait_for_modify(watcher, timeout_ms?)` | Wait for file modification |
| `wait_for_delete(watcher, timeout_ms?)` | Wait for file deletion |
| `watch_until(watcher, condition, timeout_ms?)` | Watch until condition is met |
| `stop_watching(watcher)` | Stop the file watcher |
| `is_watching(watcher)` | Check if watcher is active |
| `has_pending_events(watcher)` | Check if events are queued |

**FileEvent Struct:**

| Field | Type | Description |
|-------|------|-------------|
| `type` | `FileEventType` | Type of event |
| `path` | `str` | Path of affected file |
| `old_path` | `str` | Previous path (for renames) |
| `timestamp` | `i64` | Event timestamp (ms since epoch) |

**FileEvent Methods:**

| Method | Description |
|--------|-------------|
| `type_string()` | Get event type as string |
| `is_create()` | Check if creation event |
| `is_modify()` | Check if modification event |
| `is_delete()` | Check if deletion event |
| `is_rename()` | Check if rename event |

**FileEventType Enum:**

| Variant | Description |
|---------|-------------|
| `Created` | File or directory was created |
| `Modified` | File contents changed |
| `Deleted` | File or directory was deleted |
| `Renamed` | File or directory was renamed |
| `Unknown` | Unknown event type |

**FileWatcherConfig Struct:**

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `recursive` | `bool` | `false` | Watch subdirectories |
| `watch_creates` | `bool` | `true` | Watch for creations |
| `watch_modifies` | `bool` | `true` | Watch for modifications |
| `watch_deletes` | `bool` | `true` | Watch for deletions |
| `watch_renames` | `bool` | `true` | Watch for renames |
| `poll_interval_ms` | `i32` | `100` | Polling interval |

**Platform Notes:**

- **Windows**: Uses ReadDirectoryChangesW for efficient native file watching
- **Linux**: Uses inotify for kernel-level file watching
- **macOS/other**: Uses polling fallback with configurable interval

### Async Signal Handling

Handle OS signals (Ctrl+C, SIGTERM, etc.) asynchronously with cross-platform support.

**Basic Usage - Graceful Shutdown:**

```mana
use async

fn main() {
    println("Press Ctrl+C to exit...")

    // Wait for Ctrl+C
    let signal = async::block_on(async::wait_ctrl_c())
    println(f"Received {signal.type_string()} signal")
    println("Cleaning up...")

    // Perform cleanup
    cleanup()
    println("Goodbye!")
}
```

**Handling Multiple Signal Types:**

```mana
use async

fn main() {
    // Register handlers for multiple signals
    async::on_signal(SignalType::Interrupt)   // Ctrl+C
    async::on_signal(SignalType::Terminate)   // kill command
    async::on_signal(SignalType::Hangup)      // terminal closed

    println("Server running. Press Ctrl+C to stop...")

    // Wait for any registered signal
    let signal = async::block_on(async::wait_signal())

    match signal.type {
        SignalType::Interrupt => println("Interrupted by user"),
        SignalType::Terminate => println("Received termination request"),
        SignalType::Hangup => println("Terminal disconnected"),
        _ => println(f"Received signal: {signal.type_string()}")
    }
}
```

**Using String Names for Signals:**

```mana
use async

fn main() {
    // Register by string name
    async::on_signal("sigint")
    async::on_signal("sigterm")

    // Wait by string name
    let signal = async::block_on(async::wait_signal("interrupt", 10000))  // 10s timeout

    if signal.type != SignalType::Unknown {
        println(f"Received: {signal.type_string()}")
    } else {
        println("No signal received (timeout)")
    }
}
```

**Non-blocking Signal Checking:**

```mana
use async

fn main() {
    async::on_signal(SignalType::Interrupt)

    let mut running = true
    while running {
        // Check for signals without blocking
        if async::has_pending_signal() {
            let signal = async::get_signal()
            println(f"Got signal: {signal.type_string()}")
            running = false
        }

        // Do work...
        do_work()
        async::block_on(async::sleep_ms(100))
    }
}
```

**Server with Graceful Shutdown:**

```mana
use async

fn main() {
    // Set up signal handlers
    async::on_signal(SignalType::Interrupt)
    async::on_signal(SignalType::Terminate)

    let server = start_server()
    println("Server started on port 8080")

    // Run until shutdown signal
    while !async::has_pending_signal() {
        handle_requests(server)
    }

    // Graceful shutdown
    let signal = async::get_signal()
    println(f"Shutting down due to {signal.type_string()}...")
    server.shutdown()
    println("Server stopped")
}
```

**Ignoring Signals:**

```mana
use async

fn main() {
    // Ignore SIGPIPE (common for network programs)
    async::ignore_signal(SignalType::Pipe)

    // Your network code here...
    // Broken pipe won't crash the program
}
```

**Testing with Raised Signals:**

```mana
use async

fn test_signal_handler() {
    async::on_signal(SignalType::User1)

    // Simulate receiving a signal
    async::raise_signal(SignalType::User1)

    // Check it was received
    assert(async::has_pending_signal())

    let signal = async::get_signal()
    assert(signal.type == SignalType::User1)
}
```

**Signal Handling Functions:**

| Function | Description |
|----------|-------------|
| `on_signal(type)` | Register handler for signal type |
| `on_signals(types)` | Register handlers for multiple signals |
| `wait_signal(timeout_ms?)` | Wait for any registered signal |
| `wait_signal(type, timeout_ms?)` | Wait for specific signal type |
| `wait_ctrl_c()` | Wait for Ctrl+C (interrupt) |
| `wait_interrupt(timeout_ms?)` | Wait for interrupt signal |
| `wait_terminate(timeout_ms?)` | Wait for terminate signal |
| `wait_hangup(timeout_ms?)` | Wait for hangup signal |
| `collect_signals(condition, timeout_ms?)` | Collect signals until condition |
| `has_pending_signal()` | Check if any signal is pending |
| `has_pending_signal(type)` | Check if specific signal is pending |
| `get_signal()` | Get next pending signal (non-blocking) |
| `get_all_signals()` | Get all pending signals |
| `clear_signals()` | Clear all pending signals |
| `is_signal_registered(type)` | Check if handler is registered |
| `ignore_signal(type)` | Ignore a signal |
| `reset_signal(type)` | Reset signal to default behavior |
| `raise_signal(type)` | Raise a signal (for testing) |
| `get_signal_number(type)` | Get platform signal number |
| `signal_type_to_string(type)` | Convert type to string |
| `string_to_signal_type(str)` | Parse signal type from string |

**SignalEvent Struct:**

| Field | Type | Description |
|-------|------|-------------|
| `type` | `SignalType` | Type of signal received |
| `timestamp` | `i64` | When signal was received (ms since epoch) |
| `raw_signal` | `i32` | Platform-specific signal number |

**SignalEvent Methods:**

| Method | Description |
|--------|-------------|
| `type_string()` | Get signal type as string |
| `is_interrupt()` | Check if interrupt signal |
| `is_terminate()` | Check if terminate signal |
| `is_hangup()` | Check if hangup signal |

**SignalType Enum:**

| Variant | Description | POSIX | Windows |
|---------|-------------|-------|---------|
| `Interrupt` | User interrupt | SIGINT | CTRL_C_EVENT |
| `Terminate` | Termination request | SIGTERM | - |
| `Hangup` | Terminal hangup | SIGHUP | CTRL_CLOSE_EVENT |
| `Quit` | Quit with core dump | SIGQUIT | - |
| `Alarm` | Timer alarm | SIGALRM | - |
| `User1` | User-defined 1 | SIGUSR1 | - |
| `User2` | User-defined 2 | SIGUSR2 | - |
| `Child` | Child process status | SIGCHLD | - |
| `Pipe` | Broken pipe | SIGPIPE | - |
| `Break` | Break | - | CTRL_BREAK_EVENT |
| `Shutdown` | System shutdown | - | CTRL_SHUTDOWN_EVENT |
| `Logoff` | User logoff | - | CTRL_LOGOFF_EVENT |

**Signal String Names:**

Signals can be referenced by string name (case-insensitive):
- `"interrupt"`, `"int"`, `"sigint"` → `SignalType::Interrupt`
- `"terminate"`, `"term"`, `"sigterm"` → `SignalType::Terminate`
- `"hangup"`, `"hup"`, `"sighup"` → `SignalType::Hangup`
- `"quit"`, `"sigquit"` → `SignalType::Quit`
- `"user1"`, `"usr1"`, `"sigusr1"` → `SignalType::User1`
- `"user2"`, `"usr2"`, `"sigusr2"` → `SignalType::User2`

**Platform Notes:**

- **Windows**: Uses `SetConsoleCtrlHandler` for console control events
- **POSIX**: Uses `sigaction` for reliable signal handling
- Some signals (SIGUSR1, SIGQUIT, etc.) are only available on POSIX systems
- Windows-specific signals (Break, Shutdown, Logoff) only work on Windows

### Async Channels

Channels provide a safe way to communicate between concurrent tasks through message passing.

**Basic Channel (MPMC - Multiple Producer, Multiple Consumer):**

```mana
use async

fn main() {
    // Create an unbounded channel
    let ch = async::channel<i32>()

    // Send values
    ch.sender.send(42)
    ch.sender.send(100)

    // Receive values
    let result = ch.receiver.recv()
    if result.is_ok() {
        println(f"Received: {result.value}")
    }
}
```

**Bounded Channel with Backpressure:**

```mana
use async

fn main() {
    // Create a bounded channel with capacity 5
    let ch = async::bounded_channel<str>(5)

    // Sending to a full channel will block until space is available
    for i in 0..10 {
        let result = ch.sender.send(f"message {i}")
        if result.is_ok() {
            println(f"Sent message {i}")
        }
    }
}
```

**Producer-Consumer Pattern:**

```mana
use async

fn producer(tx: Sender<i32>) {
    for i in 0..100 {
        tx.send(i)
    }
    // Sender is dropped when function exits
}

fn consumer(rx: Receiver<i32>) {
    // Iterate over received values until channel closes
    for value in rx {
        println(f"Processing: {value}")
    }
}

fn main() {
    let ch = async::bounded_channel<i32>(10)

    // Clone sender for multiple producers
    let tx1 = ch.sender.clone()
    let tx2 = ch.sender.clone()

    // Start producers and consumer in threads
    spawn(|| producer(tx1))
    spawn(|| producer(tx2))
    spawn(|| consumer(ch.receiver))
}
```

**Non-blocking Operations:**

```mana
use async

fn main() {
    let ch = async::bounded_channel<i32>(1)

    // Try send (non-blocking)
    let result = ch.sender.try_send(42)
    match result {
        Ok(_) => println("Sent successfully"),
        Err(ChannelError::Full) => println("Channel is full"),
        Err(e) => println(f"Error: {e.error_string()}"),
    }

    // Try receive (non-blocking)
    let recv_result = ch.receiver.try_recv()
    match recv_result {
        Ok(value) => println(f"Got: {value}"),
        Err(ChannelError::Empty) => println("Channel is empty"),
        Err(e) => println(f"Error: {e.error_string()}"),
    }
}
```

**Timeout Operations:**

```mana
use async

fn main() {
    let ch = async::bounded_channel<str>(1)

    // Send with 1 second timeout
    let send_result = ch.sender.send_timeout("hello", 1000)
    if send_result.is_ok() {
        println("Sent!")
    } else if send_result.error == ChannelError::Timeout {
        println("Send timed out")
    }

    // Receive with 500ms timeout
    let recv_result = ch.receiver.recv_timeout(500)
    if recv_result.is_ok() {
        println(f"Got: {recv_result.value}")
    } else if recv_result.is_timeout() {
        println("Receive timed out")
    }
}
```

**Oneshot Channel (Single Value):**

```mana
use async

fn main() {
    // Oneshot for single-use request/response
    let ch = async::oneshot_channel<str>()

    // Send exactly one value
    ch.sender.send("result data")

    // Receive the value (blocks until sent)
    let result = ch.receiver.recv()
    if result.is_ok() {
        println(f"Got response: {result.value}")
    }
}
```

**Watch Channel (Broadcast Latest Value):**

```mana
use async

fn main() {
    // Watch channel always holds the latest value
    let ch = async::watch_channel<i32>(0)  // Initial value

    // Update the value
    ch.sender.send(42)
    ch.sender.send(100)

    // Get current value (always available)
    let current = ch.receiver.get()
    println(f"Current value: {current}")  // 100

    // Wait for changes
    let new_value = ch.receiver.wait_for_change()
    println(f"Value changed to: {new_value}")
}
```

**Broadcast Channel (Multiple Receivers):**

```mana
use async

fn main() {
    // Broadcast to multiple receivers
    let ch = async::broadcast_channel<str>(16)  // Buffer size 16

    // Create multiple receivers
    let rx1 = ch.receiver.clone()
    let rx2 = ch.receiver.clone()

    // Send to all receivers
    ch.sender.send("hello everyone")

    // Each receiver gets the message
    println(rx1.recv().value)  // "hello everyone"
    println(rx2.recv().value)  // "hello everyone"
}
```

**Async Channel Operations:**

```mana
use async

fn main() {
    let ch = async::bounded_channel<i32>(5)

    // Async send
    let send_future = async::channel_send(ch.sender, 42)
    let result = async::block_on(send_future)

    // Async receive
    let recv_future = async::channel_recv(ch.receiver)
    let value = async::block_on(recv_future)

    // Async receive with timeout
    let timeout_future = async::channel_recv_timeout(ch.receiver, 1000)
    let timeout_result = async::block_on(timeout_future)
}
```

**Channel Types:**

| Type | Description |
|------|-------------|
| `Channel<T>` | Basic MPMC channel (unbounded or bounded) |
| `Sender<T>` | Send end of a channel |
| `Receiver<T>` | Receive end of a channel |
| `OneshotChannel<T>` | Single-value channel |
| `WatchChannel<T>` | Broadcast latest value |
| `BroadcastChannel<T>` | One-to-many broadcast |

**Channel Creation Functions:**

| Function | Description |
|----------|-------------|
| `channel<T>()` | Create unbounded MPMC channel |
| `bounded_channel<T>(capacity)` | Create bounded channel |
| `rendezvous_channel<T>()` | Create channel with capacity 1 |
| `oneshot_channel<T>()` | Create single-use channel |
| `watch_channel<T>(initial)` | Create watch channel |
| `broadcast_channel<T>(capacity?)` | Create broadcast channel |

**Sender Methods:**

| Method | Description |
|--------|-------------|
| `send(value)` | Send value (blocking) |
| `try_send(value)` | Try to send (non-blocking) |
| `send_timeout(value, ms)` | Send with timeout |
| `is_closed()` | Check if channel is closed |
| `len()` | Get queue length |
| `capacity()` | Get channel capacity |
| `clone()` | Clone the sender |

**Receiver Methods:**

| Method | Description |
|--------|-------------|
| `recv()` | Receive value (blocking) |
| `try_recv()` | Try to receive (non-blocking) |
| `recv_timeout(ms)` | Receive with timeout |
| `is_closed()` | Check if channel is closed |
| `is_empty()` | Check if queue is empty |
| `len()` | Get queue length |
| `clone()` | Clone the receiver |

**SendResult<T> Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `success` | `bool` | Whether send succeeded |
| `error` | `ChannelError` | Error type if failed |
| `value` | `T` | Returned value if failed |

**RecvResult<T> Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `success` | `bool` | Whether receive succeeded |
| `error` | `ChannelError` | Error type if failed |
| `value` | `T` | Received value |

**RecvResult<T> Methods:**

| Method | Description |
|--------|-------------|
| `is_ok()` | Check if successful |
| `is_err()` | Check if error |
| `is_closed()` | Check if channel closed |
| `is_empty()` | Check if channel was empty |
| `is_timeout()` | Check if operation timed out |
| `unwrap()` | Get value (panics if error) |
| `unwrap_or(default)` | Get value or default |

**ChannelError Enum:**

| Variant | Description |
|---------|-------------|
| `None` | No error |
| `Closed` | Channel has been closed |
| `Full` | Channel is full (bounded) |
| `Empty` | Channel is empty |
| `Timeout` | Operation timed out |
| `Disconnected` | All senders/receivers gone |

---

### Async Synchronization Primitives

Mana provides async-friendly synchronization primitives for coordinating concurrent access to shared resources.

**Mutex (Mutual Exclusion):**

```mana
use async

fn main() {
    // Create an async mutex
    let mtx = async::mutex()

    // Lock the mutex (blocking)
    mtx.lock()
    // ... critical section ...
    mtx.unlock()

    // Try to lock (non-blocking)
    if mtx.try_lock() {
        // Got the lock
        mtx.unlock()
    }

    // Lock with timeout
    let result = mtx.lock_timeout(1000)  // 1 second
    if result == LockResult::Acquired {
        mtx.unlock()
    } else if result == LockResult::Timeout {
        println("Lock timed out")
    }

    // Check if locked
    if mtx.is_locked() {
        println("Mutex is currently locked")
    }
}
```

**RAII Mutex Guard:**

```mana
use async

fn main() {
    let mtx = async::mutex()

    {
        // Guard automatically unlocks when it goes out of scope
        let guard = async::mutex_guard(mtx)
        // ... critical section ...
    } // guard unlocked here

    // Async lock operation
    let lock_future = async::mutex_lock(mtx)
    let result = async::block_on(lock_future)
    if result == LockResult::Acquired {
        mtx.unlock()
    }
}
```

**Reader-Writer Lock:**

```mana
use async

fn main() {
    let rwlock = async::rwlock()

    // Multiple readers can hold the lock simultaneously
    rwlock.read_lock()
    // ... read data ...
    rwlock.read_unlock()

    // Only one writer can hold the lock
    rwlock.write_lock()
    // ... write data ...
    rwlock.write_unlock()

    // Try variants (non-blocking)
    if rwlock.try_read_lock() {
        rwlock.read_unlock()
    }

    if rwlock.try_write_lock() {
        rwlock.write_unlock()
    }

    // With timeouts
    let read_result = rwlock.read_lock_timeout(1000)
    let write_result = rwlock.write_lock_timeout(1000)
}
```

**RAII RwLock Guards:**

```mana
use async

fn main() {
    let rwlock = async::rwlock()

    {
        // Read guard
        let read_guard = async::rwlock_read_guard(rwlock)
        // ... read access ...
    } // automatically releases read lock

    {
        // Write guard
        let write_guard = async::rwlock_write_guard(rwlock)
        // ... write access ...
    } // automatically releases write lock
}
```

**Semaphore:**

```mana
use async

fn main() {
    // Counting semaphore with 5 permits
    let sem = async::semaphore(5)

    // Acquire a permit (blocking)
    sem.acquire()
    // ... use resource ...
    sem.release()

    // Acquire multiple permits
    sem.acquire(3)
    sem.release(3)

    // Try to acquire (non-blocking)
    if sem.try_acquire() {
        sem.release()
    }

    // Acquire with timeout
    let result = sem.acquire_timeout(1, 1000)  // 1 permit, 1 second
    if result == LockResult::Acquired {
        sem.release()
    }

    // Check available permits
    let available = sem.available_permits()
    println(f"Available: {available}")

    // Binary semaphore (like a mutex but can be released by any thread)
    let binary = async::binary_semaphore(true)  // initially available
    binary.acquire()
    binary.release()
}
```

**RAII Semaphore Permit:**

```mana
use async

fn main() {
    let sem = async::semaphore(3)

    {
        // Permit automatically releases when it goes out of scope
        let permit = async::semaphore_permit(sem)
        // ... use resource ...
    } // permit released here

    // Async acquire
    let acquire_future = async::semaphore_acquire(sem, 1)
    let result = async::block_on(acquire_future)
}
```

**Barrier:**

```mana
use async

fn main() {
    // Barrier for 4 threads
    let barrier = async::barrier(4)

    // Each thread waits at the barrier
    // Returns when all threads have reached the barrier
    let arrival_index = barrier.wait()
    println(f"Thread arrived at position {arrival_index}")

    // Wait with timeout
    let result = barrier.wait_timeout(5000)  // 5 seconds
    if result >= 0 {
        println(f"Arrived at position {result}")
    } else {
        println("Barrier wait timed out")
    }
}
```

**Condition Variable:**

```mana
use async

fn main() {
    let cv = async::condvar()
    let mtx = async::mutex()

    // Wait for a condition
    mtx.lock()
    cv.wait(mtx)  // Atomically releases mutex and waits
    // ... condition is now true ...
    mtx.unlock()

    // Wait with timeout
    mtx.lock()
    let signaled = cv.wait_timeout(mtx, 1000)
    if signaled {
        // Condition was signaled
    } else {
        // Timed out
    }
    mtx.unlock()

    // Signal one waiter
    cv.notify_one()

    // Signal all waiters
    cv.notify_all()
}
```

**Once (One-Time Initialization):**

```mana
use async

let init_once = async::once()
let mut shared_resource = 0

fn initialize() {
    shared_resource = expensive_computation()
}

fn get_resource() -> i32 {
    // Only the first call executes the function
    init_once.call_once(initialize)
    return shared_resource
}

fn main() {
    // Multiple calls, but initialize() runs only once
    let a = get_resource()
    let b = get_resource()
    let c = get_resource()

    // Check if initialization completed
    if init_once.is_completed() {
        println("Resource is initialized")
    }
}
```

**LockResult Enum:**

| Variant | Description |
|---------|-------------|
| `Acquired` | Lock successfully acquired |
| `WouldBlock` | Lock not available (try_lock failed) |
| `Timeout` | Lock operation timed out |
| `Error` | An error occurred |

**Mutex Functions:**

| Function | Description |
|----------|-------------|
| `mutex()` | Create a new async mutex |
| `mutex_lock(mtx)` | Create async lock future |
| `mutex_guard(mtx)` | Create RAII guard future |
| `mutex_try_lock(mtx)` | Try to acquire lock (non-blocking) |
| `mutex_unlock(mtx)` | Release the lock |
| `mutex_is_locked(mtx)` | Check if mutex is locked |

**RwLock Functions:**

| Function | Description |
|----------|-------------|
| `rwlock()` | Create a new reader-writer lock |
| `rwlock_read(rwl)` | Create async read lock future |
| `rwlock_write(rwl)` | Create async write lock future |
| `rwlock_read_guard(rwl)` | Create RAII read guard future |
| `rwlock_write_guard(rwl)` | Create RAII write guard future |
| `rwlock_try_read(rwl)` | Try to acquire read lock |
| `rwlock_try_write(rwl)` | Try to acquire write lock |

**Semaphore Functions:**

| Function | Description |
|----------|-------------|
| `semaphore(permits)` | Create counting semaphore |
| `binary_semaphore(initial?)` | Create binary semaphore |
| `semaphore_acquire(sem, n?)` | Create async acquire future |
| `semaphore_permit(sem)` | Create RAII permit future |
| `semaphore_try_acquire(sem, n?)` | Try to acquire permits |
| `semaphore_release(sem, n?)` | Release permits |
| `semaphore_available(sem)` | Get available permits |
| `semaphore_drain(sem)` | Drain all permits |
| `semaphore_reset(sem, permits)` | Reset to specified permits |

**Barrier Functions:**

| Function | Description |
|----------|-------------|
| `barrier(count)` | Create barrier for count threads |
| `barrier_wait(bar)` | Create async wait future |
| `barrier_wait_timeout(bar, ms)` | Wait with timeout |

**CondVar Functions:**

| Function | Description |
|----------|-------------|
| `condvar()` | Create condition variable |
| `condvar_wait(cv, mtx)` | Create async wait future |
| `condvar_wait_timeout(cv, mtx, ms)` | Wait with timeout |
| `condvar_notify_one(cv)` | Wake one waiter |
| `condvar_notify_all(cv)` | Wake all waiters |

**Once Functions:**

| Function | Description |
|----------|-------------|
| `once()` | Create once instance |
| `once_call(o, fn)` | Call function once |
| `once_is_completed(o)` | Check if completed |

---

### Async File I/O

Mana provides comprehensive async file I/O operations for non-blocking file system access.

**Reading Files:**

```mana
use async

fn main() {
    // Read entire file as string
    let content_future = async::read_file("config.txt")
    let result = async::block_on(content_future)
    if result.is_ok() {
        println(f"Content: {result.value}")
    }

    // Read file as bytes (binary)
    let bytes_future = async::read_bytes("image.png")
    let bytes_result = async::block_on(bytes_future)
    if bytes_result.is_ok() {
        println(f"Read {bytes_result.value.len()} bytes")
    }

    // Read specific range of bytes
    let range_future = async::read_range("large_file.bin", 1000, 500)  // offset, length
    let range_result = async::block_on(range_future)

    // Read file lines
    let lines_future = async::read_lines("data.csv")
    let lines_result = async::block_on(lines_future)
    if lines_result.is_ok() {
        for line in lines_result.value {
            println(line)
        }
    }
}
```

**Writing Files:**

```mana
use async

fn main() {
    // Write string to file
    let write_future = async::write_file("output.txt", "Hello, World!")
    let result = async::block_on(write_future)

    // Append to file
    let append_future = async::append_file("log.txt", "New log entry\n")
    async::block_on(append_future)

    // Write bytes (binary)
    let data: Vec<u8> = [0x89, 0x50, 0x4E, 0x47]
    let bytes_future = async::write_bytes("output.bin", data)
    async::block_on(bytes_future)

    // Append bytes
    let append_bytes_future = async::append_bytes("output.bin", more_data)
    async::block_on(append_bytes_future)
}
```

**File Operations:**

```mana
use async

fn main() {
    // Check if file exists
    let exists_future = async::file_exists("config.json")
    let exists = async::block_on(exists_future)

    // Get file size
    let size_future = async::file_size("large_file.bin")
    let size_result = async::block_on(size_future)
    if size_result.is_ok() {
        println(f"File size: {size_result.value} bytes")
    }

    // Get file metadata
    let meta_future = async::file_metadata("document.txt")
    let meta_result = async::block_on(meta_future)
    if meta_result.is_ok() {
        let meta = meta_result.value
        println(f"Path: {meta.path}")
        println(f"Size: {meta.size}")
        println(f"Is file: {meta.is_file}")
        println(f"Is directory: {meta.is_directory}")
        println(f"Modified: {meta.modified_time}")
    }

    // Copy file
    let copy_future = async::copy_file("source.txt", "dest.txt")
    async::block_on(copy_future)

    // Move/rename file
    let move_future = async::move_file("old_name.txt", "new_name.txt")
    async::block_on(move_future)

    // Delete file
    let delete_future = async::delete_file("temp.txt")
    async::block_on(delete_future)

    // Truncate file
    let trunc_future = async::truncate_file("file.txt", 1024)  // Resize to 1024 bytes
    async::block_on(trunc_future)

    // Set permissions (Unix-style octal)
    let perm_future = async::set_permissions("script.sh", 0o755)
    async::block_on(perm_future)
}
```

**Directory Operations:**

```mana
use async

fn main() {
    // Create directory
    let mkdir_future = async::create_dir("new_folder")
    async::block_on(mkdir_future)

    // Create directory recursively (like mkdir -p)
    let mkdir_all_future = async::create_dir_all("path/to/nested/folder")
    async::block_on(mkdir_all_future)

    // Remove empty directory
    let rmdir_future = async::remove_dir("empty_folder")
    async::block_on(rmdir_future)

    // Remove directory recursively (like rm -rf)
    let rmdir_all_future = async::remove_dir_all("folder_with_contents")
    async::block_on(rmdir_all_future)

    // List directory contents
    let list_future = async::list_dir("./src")
    let list_result = async::block_on(list_future)
    if list_result.is_ok() {
        for entry in list_result.value {
            println(f"{entry.name} - {'dir' if entry.is_directory else 'file'}")
        }
    }

    // List directory recursively
    let list_all_future = async::list_dir_recursive("./project")
    let entries = async::block_on(list_all_future)
}
```

**Glob Pattern Matching:**

```mana
use async

fn main() {
    // Find files matching pattern
    let glob_future = async::glob("./src", "**/*.mana")
    let matches = async::block_on(glob_future)
    if matches.is_ok() {
        for path in matches.value {
            println(f"Found: {path}")
        }
    }

    // Patterns support:
    // * - matches any characters except /
    // ** - matches any characters including /
    // ? - matches single character
    async::glob(".", "*.txt")           // All .txt in current dir
    async::glob(".", "**/*.rs")         // All .rs files recursively
    async::glob(".", "src/**/test_*")   // All test_ files under src
}
```

**Symlinks:**

```mana
use async

fn main() {
    // Create symlink to file
    let link_future = async::create_symlink("target.txt", "link.txt")
    async::block_on(link_future)

    // Create symlink to directory
    let dir_link_future = async::create_dir_symlink("target_dir", "link_dir")
    async::block_on(dir_link_future)

    // Read symlink target
    let read_link_future = async::read_symlink("link.txt")
    let target = async::block_on(read_link_future)
    if target.is_ok() {
        println(f"Link points to: {target.value}")
    }
}
```

**Path Operations:**

```mana
use async

fn main() {
    // Get canonical (absolute) path
    let canon_future = async::canonical_path("./relative/path/../file.txt")
    let canon_result = async::block_on(canon_future)
    if canon_result.is_ok() {
        println(f"Canonical: {canon_result.value}")
    }
}
```

**Temporary Files and Directories:**

```mana
use async

fn main() {
    // Create temporary file
    let temp_future = async::temp_file("myapp_", ".tmp")
    let temp_result = async::block_on(temp_future)
    if temp_result.is_ok() {
        let temp_path = temp_result.value
        println(f"Created temp file: {temp_path}")
        // Use the file...
        async::delete_file(temp_path)  // Clean up
    }

    // Create temporary directory
    let temp_dir_future = async::temp_dir("myapp_")
    let temp_dir_result = async::block_on(temp_dir_future)
    if temp_dir_result.is_ok() {
        let temp_dir_path = temp_dir_result.value
        println(f"Created temp dir: {temp_dir_path}")
        // Use the directory...
        async::remove_dir_all(temp_dir_path)  // Clean up
    }
}
```

**Streaming File I/O (AsyncFile):**

```mana
use async

fn main() {
    // Open file for reading
    let file = async::open_file_read("large_file.bin")

    // Read in chunks
    while true {
        let chunk_future = async::async_file_read(file, 4096)
        let chunk = async::block_on(chunk_future)
        if chunk.is_err() || chunk.value.is_empty() {
            break
        }
        process_chunk(chunk.value)
    }

    file.close()

    // Open file for writing
    let out_file = async::open_file_write("output.bin")

    // Write data
    let write_future = async::async_file_write(out_file, data)
    async::block_on(write_future)

    // Seek to position
    out_file.seek(0, 0)  // Seek to beginning (whence: 0=start, 1=current, 2=end)

    // Flush buffers
    out_file.flush()

    out_file.close()

    // Open for appending
    let append_file = async::open_file_append("log.txt")
}
```

**FileMetadata Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `path` | `str` | File path |
| `size` | `u64` | File size in bytes |
| `created_time` | `i64` | Creation time (Unix timestamp) |
| `modified_time` | `i64` | Last modified time |
| `accessed_time` | `i64` | Last accessed time |
| `is_file` | `bool` | Is regular file |
| `is_directory` | `bool` | Is directory |
| `is_symlink` | `bool` | Is symbolic link |
| `is_readonly` | `bool` | Is read-only |
| `file_type` | `str` | Type: "file", "directory", "symlink", "other" |

**DirEntry Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `name` | `str` | Entry name (filename only) |
| `path` | `str` | Full path |
| `is_file` | `bool` | Is regular file |
| `is_directory` | `bool` | Is directory |
| `is_symlink` | `bool` | Is symbolic link |
| `size` | `u64` | File size (if file) |

**File Reading Functions:**

| Function | Description |
|----------|-------------|
| `read_file(path)` | Read file as string |
| `read_bytes(path)` | Read file as byte array |
| `read_lines(path)` | Read file as lines |
| `read_range(path, offset, len)` | Read byte range |

**File Writing Functions:**

| Function | Description |
|----------|-------------|
| `write_file(path, content)` | Write string to file |
| `write_bytes(path, data)` | Write bytes to file |
| `append_file(path, content)` | Append string to file |
| `append_bytes(path, data)` | Append bytes to file |

**File Operations:**

| Function | Description |
|----------|-------------|
| `file_exists(path)` | Check if file exists |
| `file_size(path)` | Get file size |
| `file_metadata(path)` | Get file metadata |
| `copy_file(src, dst)` | Copy file |
| `move_file(src, dst)` | Move/rename file |
| `rename_file(src, dst)` | Alias for move_file |
| `delete_file(path)` | Delete file |
| `truncate_file(path, size)` | Resize file |
| `set_permissions(path, mode)` | Set file permissions |

**Directory Operations:**

| Function | Description |
|----------|-------------|
| `create_dir(path)` | Create directory |
| `create_dir_all(path)` | Create directory recursively |
| `remove_dir(path)` | Remove empty directory |
| `remove_dir_all(path)` | Remove directory recursively |
| `list_dir(path)` | List directory contents |
| `list_dir_recursive(path)` | List recursively |
| `glob(base, pattern)` | Find files by pattern |

**Symlink Operations:**

| Function | Description |
|----------|-------------|
| `create_symlink(target, link)` | Create file symlink |
| `create_dir_symlink(target, link)` | Create directory symlink |
| `read_symlink(path)` | Read symlink target |

**Path Operations:**

| Function | Description |
|----------|-------------|
| `canonical_path(path)` | Get canonical path |

**Temp File Operations:**

| Function | Description |
|----------|-------------|
| `temp_file(prefix?, suffix?)` | Create temp file |
| `temp_dir(prefix?)` | Create temp directory |

**AsyncFile Handle Functions:**

| Function | Description |
|----------|-------------|
| `open_file(path, mode)` | Open with mode |
| `open_file_read(path)` | Open for reading |
| `open_file_write(path)` | Open for writing |
| `open_file_append(path)` | Open for appending |
| `async_file_read(file, count)` | Read bytes from handle |
| `async_file_write(file, data)` | Write bytes to handle |

**AsyncFile Methods:**

| Method | Description |
|--------|-------------|
| `close()` | Close the file |
| `is_open()` | Check if open |
| `position()` | Get current position |
| `path()` | Get file path |
| `read(count)` | Read bytes (sync) |
| `write(data)` | Write bytes (sync) |
| `seek(offset, whence)` | Seek position |
| `flush()` | Flush buffers |

---

## Basic I/O

### print / println

Output to standard output.

```mana
print("Hello")          // No newline
println("Hello")        // With newline
println(42)             // Works with numbers
println(3.14)           // Works with floats
println(true)           // Works with bools
```

### read_line

Read a line from standard input.

```mana
print("Enter your name: ")
let name = read_line()
println(f"Hello, {name}!")
```

### parse_int / parse_float

Parse strings to numbers.

```mana
let num = parse_int("42")     // Option<i32>
if num.is_some() {
    println(f"Got: {num.unwrap()}")
}

let pi = parse_float("3.14")  // Option<f32>
```

---

## String Functions

### Basic Operations

```mana
let s = "Hello, World!"

len(s)                    // 13 - string length
is_empty(s)               // false
to_string(42)             // "42" - convert to string
```

### Searching

```mana
contains(s, "World")      // true
starts_with(s, "Hello")   // true
ends_with(s, "!")         // true
find(s, "World")          // Some(7) - returns Option<i32>
```

### Transformation

```mana
to_uppercase(s)           // "HELLO, WORLD!"
to_lowercase(s)           // "hello, world!"
trim("  hello  ")         // "hello"
replace(s, "World", "Mana") // "Hello, Mana!"
reverse(s)                // "!dlroW ,olleH"
```

### Slicing and Splitting

```mana
substr(s, 0, 5)           // "Hello"
split(s, ", ")            // Vec["Hello", "World!"]
join(vec, ", ")           // Join with separator
repeat("ab", 3)           // "ababab"
```

---

## Math Functions

### Basic Math

```mana
abs(-5)                   // 5
min(3, 7)                 // 3
max(3, 7)                 // 7
clamp(x, 0, 100)          // Clamp x to range [0, 100]
sign(-5)                  // -1 (returns -1, 0, or 1)
```

### Trigonometry

```mana
sin(x)                    // Sine
cos(x)                    // Cosine
tan(x)                    // Tangent
asin(x)                   // Arc sine
acos(x)                   // Arc cosine
atan(x)                   // Arc tangent
atan2(y, x)               // Two-argument arc tangent
```

### Exponential and Logarithmic

```mana
exp(x)                    // e^x
exp2(x)                   // 2^x
log(x)                    // Natural logarithm
log2(x)                   // Base-2 logarithm
log10(x)                  // Base-10 logarithm
pow(base, exp)            // Power function
sqrt(x)                   // Square root
cbrt(x)                   // Cube root
```

### Rounding

```mana
floor(3.7)                // 3.0
ceil(3.2)                 // 4.0
round(3.5)                // 4.0
trunc(3.7)                // 3.0
fract(3.7)                // 0.7 (fractional part)
```

### Constants

```mana
PI()                      // 3.14159265358979
TAU()                     // 6.28318530717958 (2π)
E()                       // 2.71828182845904
```

### Angle Conversion

```mana
radians(180.0)            // 3.14159... (degrees to radians)
degrees(PI())             // 180.0 (radians to degrees)
```

### Interpolation

```mana
lerp(0.0, 100.0, 0.5)     // 50.0 (linear interpolation)
smoothstep(0.0, 1.0, x)   // Smooth interpolation
```

---

## Math Types

### Vec2

2D vector for positions, directions, and UVs.

```mana
let v = Vec2(3.0, 4.0)
let u = Vec2(1.0, 0.0)

// Operations
v + u                     // Vector addition
v - u                     // Vector subtraction
v * 2.0                   // Scalar multiplication
v / 2.0                   // Scalar division
-v                        // Negation

// Methods
v.length()                // 5.0
v.length_squared()        // 25.0
v.normalized()            // Unit vector

// Functions
dot(v, u)                 // Dot product
cross(v, u)               // 2D cross product (scalar)
distance(v, u)            // Distance between points
lerp(v, u, 0.5)           // Linear interpolation
reflect(v, normal)        // Reflection

// Constants
Vec2::zero()              // (0, 0)
Vec2::one()               // (1, 1)
Vec2::up()                // (0, 1)
Vec2::right()             // (1, 0)
```

### Vec3

3D vector for positions, directions, colors, and normals.

```mana
let v = Vec3(1.0, 2.0, 3.0)
let u = Vec3(0.0, 1.0, 0.0)

// All Vec2 operations plus:
cross(v, u)               // 3D cross product (Vec3)
refract(v, normal, eta)   // Refraction

// Swizzling
v.xy()                    // Vec2(1.0, 2.0)
v.xz()                    // Vec2(1.0, 3.0)

// Constants
Vec3::zero()              // (0, 0, 0)
Vec3::one()               // (1, 1, 1)
Vec3::up()                // (0, 1, 0)
Vec3::forward()           // (0, 0, -1) OpenGL convention
Vec3::right()             // (1, 0, 0)
```

### Vec4

4D vector for homogeneous coordinates and colors with alpha.

```mana
let v = Vec4(1.0, 2.0, 3.0, 1.0)
let color = Vec4(1.0, 0.0, 0.0, 1.0)  // Red with full alpha

// Swizzling
v.xyz()                   // Vec3
v.xy()                    // Vec2
v.rgb()                   // Vec3 (same as xyz)

// Constants
Vec4::zero()              // (0, 0, 0, 0)
Vec4::one()               // (1, 1, 1, 1)
```

### Mat4

4x4 matrix for transformations (column-major for OpenGL).

```mana
let m = Mat4::identity()

// Access
m(row, col)               // Element access
m.column(i)               // Get column as Vec4
m.row(i)                  // Get row as Vec4
m.data()                  // Raw pointer for OpenGL

// Operations
m1 * m2                   // Matrix multiplication
m * vec4                  // Transform Vec4
m.transform_point(vec3)   // Transform point (w=1)
m.transform_direction(v)  // Transform direction (w=0)

// Properties
m.determinant()           // Determinant
m.transposed()            // Transpose
m.inverse()               // Inverse

// Transform constructors
translate(Vec3(x, y, z))  // Translation matrix
scale(Vec3(x, y, z))      // Scale matrix
scale(uniform)            // Uniform scale
rotate_x(angle)           // Rotation around X axis
rotate_y(angle)           // Rotation around Y axis
rotate_z(angle)           // Rotation around Z axis
rotate(axis, angle)       // Rotation around arbitrary axis

// Projection matrices
perspective(fov_y, aspect, near, far)
ortho(left, right, bottom, top, near, far)

// View matrix
look_at(eye, target, up)
```

### Quat

Quaternion for smooth rotations without gimbal lock.

```mana
let q = Quat::identity()
let rotation = Quat::from_axis_angle(Vec3::up(), radians(90.0))

// Operations
q1 * q2                   // Quaternion multiplication
q.conjugate()             // Conjugate
q.inverse()               // Inverse
q.normalized()            // Normalize

// Rotation
q * vec3                  // Rotate vector
q.to_mat4()               // Convert to Mat4
slerp(q1, q2, t)          // Spherical interpolation

// Constructors
Quat::from_axis_angle(axis, angle)
Quat::from_euler(pitch, yaw, roll)
Quat::look_rotation(forward, up)
```

---

## Random

Generate random numbers.

```mana
random()                  // Random f32 in [0, 1)
random_int(1, 100)        // Random i32 in [1, 100]
random_float(0.0, 1.0)    // Random f32 in range
random_double(0.0, 1.0)   // Random f64 in range
```

---

## Time

Time and timing functions.

```mana
time_now_ms()             // Current time in milliseconds (i64)
time_now_secs()           // Current time in seconds (f64)
sleep_ms(100)             // Sleep for 100 milliseconds
```

---

## Graphics (OpenGL)

The `gl` namespace provides OpenGL 3.3 core profile bindings for rendering.

### Window Management

```mana
// Create window
let window = gl::create_window(800, 600, "My Game")
if !window.valid {
    println("Failed to create window")
    return
}

// Main loop
while !gl::should_close(window) {
    gl::poll_events()

    // Clear screen
    gl::clear_color(0.2, 0.3, 0.3, 1.0)
    gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    // Render here...

    gl::swap_buffers(window)
}

gl::destroy_window(window)
```

### Shaders

```mana
// Create shader program
let shader = gl::create_shader_program(vertex_src, fragment_src)
if !shader.valid {
    println(f"Shader error: {shader.error}")
}

gl::use_program(shader)

// Set uniforms
gl::set_uniform_mat4(shader, "model", model_matrix)
gl::set_uniform_vec3(shader, "color", Vec3(1.0, 0.0, 0.0))
gl::set_uniform_float(shader, "time", time)
gl::set_uniform_int(shader, "texture0", 0)
```

### Buffers and VAOs

```mana
// Create VAO
let vao = gl::create_vao()
gl::bind_vao(vao)

// Create VBO with vertex data
let vertices: Vec<f32> = [
    // positions      // colors
    0.0, 0.5, 0.0,   1.0, 0.0, 0.0,
   -0.5,-0.5, 0.0,   0.0, 1.0, 0.0,
    0.5,-0.5, 0.0,   0.0, 0.0, 1.0
]
let vbo = gl::create_vbo(vertices)

// Define vertex attributes
gl::vertex_attrib(0, 3, 6, 0)  // position: 3 floats, stride 6, offset 0
gl::vertex_attrib(1, 3, 6, 3)  // color: 3 floats, stride 6, offset 3
```

### Textures

```mana
let tex = gl::load_texture("texture.bmp")
if tex.valid {
    gl::bind_texture(tex, 0)  // Bind to texture unit 0
}

// Texture parameters
gl::set_texture_filter(tex, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR)
gl::set_texture_wrap(tex, GL_REPEAT, GL_REPEAT)
gl::generate_mipmaps(tex)
```

### Drawing

```mana
gl::draw_arrays(GL_TRIANGLES, 0, 3)
gl::draw_elements(GL_TRIANGLES, index_count, indices)
```

### Input

```mana
// Keyboard
if gl::key_pressed(window, KEY_ESCAPE) {
    gl::set_should_close(window, true)
}
if gl::key_down(window, KEY_W) {
    // Move forward
}

// Mouse
let mouse_pos = gl::get_mouse_pos(window)   // Vec2
let mouse_delta = gl::get_mouse_delta(window)
if gl::mouse_button_down(window, MOUSE_LEFT) {
    // Left click held
}

// Cursor modes
gl::set_cursor_mode(window, CURSOR_DISABLED)  // FPS camera
gl::set_cursor_mode(window, CURSOR_NORMAL)    // Normal cursor
gl::enable_raw_mouse_motion(window, true)
```

### Gamepad

```mana
if gl::gamepad_present(0) {
    let state = gl::get_gamepad_state(0)

    // Buttons
    if state.button_a { /* A pressed */ }
    if state.button_b { /* B pressed */ }

    // Axes (with deadzone)
    let left_stick = Vec2(state.left_x, state.left_y)
    let right_stick = Vec2(state.right_x, state.right_y)
    let triggers = Vec2(state.left_trigger, state.right_trigger)
}
```

### OpenGL State

```mana
gl::enable(GL_DEPTH_TEST)
gl::enable(GL_BLEND)
gl::blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
gl::cull_face(GL_BACK)
gl::viewport(0, 0, width, height)
```

---

## Audio

The `audio` namespace provides sound and music playback.

### Initialization

```mana
audio::init()      // Initialize audio system
defer audio::shutdown()
```

### Sound Effects

Short sounds loaded entirely into memory.

```mana
let sfx = audio::load_sound("explosion.wav")
if !sfx.valid {
    println("Failed to load sound")
}

// Playback
audio::play_sound(sfx)
audio::play_sound_from_start(sfx)  // Restart if already playing
audio::stop_sound(sfx)

// Properties
sfx.volume = 0.8          // 0.0 to 1.0
sfx.pitch = 1.2           // Playback speed
sfx.pan = -0.5            // -1.0 (left) to 1.0 (right)
sfx.looping = true        // Loop the sound
```

### Music

For longer audio tracks.

```mana
let music = audio::load_music("background.wav")

audio::play_music(music)
audio::pause_music(music)
audio::resume_music(music)
audio::stop_music(music)
audio::seek_music(music, 30.0)  // Seek to 30 seconds

music.volume = 0.5
music.looping = true
```

### Volume Control

```mana
audio::set_master_volume(0.8)   // Global volume
audio::set_sound_volume(1.0)    // All sound effects
audio::set_music_volume(0.5)    // Music volume
```

### 3D Positional Audio

```mana
audio::set_listener_position(camera_pos)
audio::set_listener_orientation(forward, up)
audio::play_sound_3d(sfx, position, max_distance)
```

### Procedural Audio

```mana
// Generate tones
audio::play_tone(frequency, duration_ms, volume)
audio::play_noise(duration_ms, volume)  // White noise
```

---

## Platform Support

| Module | Windows | macOS | Linux |
|--------|---------|-------|-------|
| HTTP/HTTPS | WinHTTP (native TLS) | libcurl | libcurl |
| WebSocket | WinHTTP | Raw sockets | Raw sockets |
| SQLite | winsqlite3.dll / sqlite3.dll | libsqlite3 | libsqlite3 |
| Threading | Win32 threads | pthreads | pthreads |
| Async | Built-in | Built-in | Built-in |
| Channels | Built-in | Built-in | Built-in |
| File Watcher | ReadDirectoryChangesW | Polling | inotify |
| Signal Handling | SetConsoleCtrlHandler | sigaction | sigaction |
| Compression | Built-in | Built-in | Built-in |
| XML | Built-in | Built-in | Built-in |
| JSON | Built-in | Built-in | Built-in |
| Crypto | Built-in | Built-in | Built-in |

---

## Error Handling Best Practices

```mana
// Use Result for fallible operations
fn load_data(path: str) -> Result<Data, str> {
    let content = read_file(path)?
    let parsed = parse(content)?
    return Ok(parsed)
}

// Pattern matching for comprehensive handling
match load_data("config.json") {
    Ok(data) => process(data),
    Err(e) => {
        log_error(e)
        use_defaults()
    }
}

// Provide context when propagating errors
fn init() -> Result<(), str> {
    load_config().map_err(|e| f"Config error: {e}")?
    connect_db().map_err(|e| f"Database error: {e}")?
    return Ok(())
}
```
