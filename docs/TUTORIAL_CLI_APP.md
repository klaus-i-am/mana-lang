# Building a CLI Application in Mana

This hands-on tutorial walks you through building a complete command-line application: a task manager that stores todos in a file.

## Prerequisites

- Mana installed and working (`mana --version`)
- Completed the basic [Tutorial](TUTORIAL.md) chapters 1-3

## What We're Building

A `todo` command-line app with these features:

```bash
todo add "Buy groceries"       # Add a task
todo list                      # Show all tasks
todo done 1                    # Mark task #1 complete
todo remove 1                  # Remove task #1
todo clear                     # Clear completed tasks
```

## Project Setup

Create a new project:

```bash
mana new todo-app
cd todo-app
```

This creates:
```
todo-app/
  package.toml
  src/
    main.mana
```

## Step 1: Parsing Arguments

First, let's handle command-line arguments.

**src/main.mana:**
```mana
module main

fn main() {
    let args = std::args()

    if args.len() < 2 {
        print_usage()
        return
    }

    let command = args[1]

    when command.as_str() {
        "add" -> {
            if args.len() < 3 {
                println("Usage: todo add <task>")
                return
            }
            add_task(args[2])
        }
        "list" -> list_tasks()
        "done" -> {
            if args.len() < 3 {
                println("Usage: todo done <number>")
                return
            }
            mark_done(parse_int(args[2]))
        }
        "remove" -> {
            if args.len() < 3 {
                println("Usage: todo remove <number>")
                return
            }
            remove_task(parse_int(args[2]))
        }
        "clear" -> clear_completed()
        _ -> {
            println("Unknown command: ", command)
            print_usage()
        }
    }
}

fn print_usage() {
    println("Todo - A simple task manager")
    println("")
    println("Usage:")
    println("  todo add <task>    Add a new task")
    println("  todo list          List all tasks")
    println("  todo done <n>      Mark task n as done")
    println("  todo remove <n>    Remove task n")
    println("  todo clear         Remove completed tasks")
}

// Placeholder functions - we'll implement these next
fn add_task(text: string) {
    println("Adding: ", text)
}

fn list_tasks() {
    println("Listing tasks...")
}

fn mark_done(n: int) {
    println("Marking ", n, " as done")
}

fn remove_task(n: int) {
    println("Removing ", n)
}

fn clear_completed() {
    println("Clearing completed")
}

fn parse_int(s: string) -> int {
    return s.parse_i32().unwrap_or(0)
}
```

Build and test:
```bash
mana run -- list
mana run -- add "Test task"
```

## Step 2: Task Data Structure

Create a module for our task model.

**src/task.mana:**
```mana
module task

/// A single todo task
pub struct Task {
    pub id: int,
    pub text: string,
    pub done: bool,
}

impl Task {
    pub fn new(id: int, text: string) -> Task {
        return Task {
            id: id,
            text: text,
            done: false,
        }
    }

    /// Format: "id|done|text"
    pub fn to_line(self) -> string {
        let done_str = if self.done { "1" } else { "0" }
        return self.id.to_string() + "|" + done_str + "|" + self.text
    }

    /// Parse from line format
    pub fn from_line(line: string) -> Option<Task> {
        let parts = line.split("|")
        if parts.len() < 3 {
            return none
        }

        let id = parts[0].parse_i32().unwrap_or(-1)
        if id < 0 {
            return none
        }

        let done = parts[1] == "1"
        let text = parts[2]

        return some(Task {
            id: id,
            text: text,
            done: done,
        })
    }
}
```

## Step 3: File Storage

Create a storage module to persist tasks.

**src/storage.mana:**
```mana
module storage

import task
import std::fs

const TODO_FILE: string = "todos.txt"

/// Load all tasks from file
pub fn load_tasks() -> Vec<task::Task> {
    let mut tasks = vec![]

    if !fs::exists(TODO_FILE) {
        return tasks
    }

    let content = fs::read_string(TODO_FILE).unwrap_or("")

    for line in content.lines() {
        if line.trim().is_empty() {
            continue
        }

        if let some(task) = task::Task::from_line(line) {
            tasks.push(task)
        }
    }

    return tasks
}

/// Save all tasks to file
pub fn save_tasks(tasks: Vec<task::Task>) {
    let mut lines = vec![]

    for t in tasks {
        lines.push(t.to_line())
    }

    let content = lines.join("\n")
    fs::write_string(TODO_FILE, content).expect("Failed to save tasks")
}

/// Get next available task ID
pub fn next_id(tasks: Vec<task::Task>) -> int {
    let mut max_id = 0
    for t in tasks {
        if t.id > max_id {
            max_id = t.id
        }
    }
    return max_id + 1
}
```

## Step 4: Implement Commands

Update main.mana with full implementations:

**src/main.mana:**
```mana
module main

import task
import storage

fn main() {
    let args = std::args()

    if args.len() < 2 {
        print_usage()
        return
    }

    let command = args[1]

    when command.as_str() {
        "add" -> {
            if args.len() < 3 {
                println("Usage: todo add <task>")
                return
            }
            // Join remaining args as task text
            let text = args[2..].join(" ")
            add_task(text)
        }
        "list" -> list_tasks()
        "done" -> {
            if args.len() < 3 {
                println("Usage: todo done <number>")
                return
            }
            mark_done(parse_int(args[2]))
        }
        "remove" -> {
            if args.len() < 3 {
                println("Usage: todo remove <number>")
                return
            }
            remove_task(parse_int(args[2]))
        }
        "clear" -> clear_completed()
        "help" | "-h" | "--help" -> print_usage()
        _ -> {
            println("Unknown command: ", command)
            print_usage()
        }
    }
}

fn print_usage() {
    println("Todo - A simple task manager")
    println("")
    println("Usage:")
    println("  todo add <task>    Add a new task")
    println("  todo list          List all tasks")
    println("  todo done <n>      Mark task n as done")
    println("  todo remove <n>    Remove task n")
    println("  todo clear         Remove completed tasks")
    println("  todo help          Show this message")
}

fn add_task(text: string) {
    let mut tasks = storage::load_tasks()
    let id = storage::next_id(tasks)

    let new_task = task::Task::new(id, text)
    tasks.push(new_task)

    storage::save_tasks(tasks)
    println("Added task #", id, ": ", text)
}

fn list_tasks() {
    let tasks = storage::load_tasks()

    if tasks.is_empty() {
        println("No tasks. Add one with: todo add <task>")
        return
    }

    println("Tasks:")
    println("------")

    for t in tasks {
        let status = if t.done { "[x]" } else { "[ ]" }
        println("  ", t.id, ". ", status, " ", t.text)
    }

    // Summary
    let total = tasks.len()
    let done = tasks.filter(|t| t.done).len()
    println("------")
    println(done, "/", total, " completed")
}

fn mark_done(n: int) {
    let mut tasks = storage::load_tasks()
    let mut found = false

    for mut t in tasks {
        if t.id == n {
            t.done = true
            found = true
            println("Completed: ", t.text)
            break
        }
    }

    if !found {
        println("Task #", n, " not found")
        return
    }

    storage::save_tasks(tasks)
}

fn remove_task(n: int) {
    let tasks = storage::load_tasks()
    let initial_len = tasks.len()

    let filtered = tasks.filter(|t| t.id != n)

    if filtered.len() == initial_len {
        println("Task #", n, " not found")
        return
    }

    storage::save_tasks(filtered)
    println("Removed task #", n)
}

fn clear_completed() {
    let tasks = storage::load_tasks()
    let remaining = tasks.filter(|t| !t.done)

    let removed = tasks.len() - remaining.len()

    if removed == 0 {
        println("No completed tasks to clear")
        return
    }

    storage::save_tasks(remaining)
    println("Cleared ", removed, " completed task(s)")
}

fn parse_int(s: string) -> int {
    return s.parse_i32().unwrap_or(0)
}
```

## Step 5: Test the App

```bash
# Build
mana build

# Test commands
./target/release/todo-app add "Learn Mana"
./target/release/todo-app add "Build a CLI app"
./target/release/todo-app add "Share with friends"
./target/release/todo-app list

# Output:
# Tasks:
# ------
#   1. [ ] Learn Mana
#   2. [ ] Build a CLI app
#   3. [ ] Share with friends
# ------
# 0/3 completed

./target/release/todo-app done 1
./target/release/todo-app list

# Output:
# Tasks:
# ------
#   1. [x] Learn Mana
#   2. [ ] Build a CLI app
#   3. [ ] Share with friends
# ------
# 1/3 completed

./target/release/todo-app clear
./target/release/todo-app list

# Output:
# Tasks:
# ------
#   2. [ ] Build a CLI app
#   3. [ ] Share with friends
# ------
# 0/2 completed
```

## Step 6: Add Colors (Optional Enhancement)

Let's add color output for a better experience.

**src/colors.mana:**
```mana
module colors

// ANSI color codes
pub const RESET: string = "\x1b[0m"
pub const RED: string = "\x1b[31m"
pub const GREEN: string = "\x1b[32m"
pub const YELLOW: string = "\x1b[33m"
pub const BLUE: string = "\x1b[34m"
pub const GRAY: string = "\x1b[90m"

pub fn green(s: string) -> string {
    return GREEN + s + RESET
}

pub fn red(s: string) -> string {
    return RED + s + RESET
}

pub fn yellow(s: string) -> string {
    return YELLOW + s + RESET
}

pub fn gray(s: string) -> string {
    return GRAY + s + RESET
}
```

Update `list_tasks()` in main.mana:

```mana
import colors

fn list_tasks() {
    let tasks = storage::load_tasks()

    if tasks.is_empty() {
        println(colors::yellow("No tasks. Add one with: todo add <task>"))
        return
    }

    println(colors::blue("Tasks:"))
    println("------")

    for t in tasks {
        if t.done {
            println(colors::gray("  " + t.id.to_string() + ". [x] " + t.text))
        } else {
            println("  ", t.id, ". [ ] ", t.text)
        }
    }

    let total = tasks.len()
    let done = tasks.filter(|t| t.done).len()
    println("------")

    if done == total {
        println(colors::green("All " + total.to_string() + " tasks completed!"))
    } else {
        println(done, "/", total, " completed")
    }
}
```

## Step 7: Add Due Dates (Exercise)

As an exercise, extend the app to support due dates:

1. Add a `due: Option<Date>` field to `Task`
2. Update `add` command: `todo add "Task" --due 2024-12-25`
3. Show overdue tasks in red
4. Add `todo due` to list tasks by due date

**Hints:**
```mana
// In task.mana
pub struct Task {
    pub id: int,
    pub text: string,
    pub done: bool,
    pub due: Option<Date>,
}

// Parsing dates
let date = Date::parse("2024-12-25")

// Comparing dates
if let some(d) = task.due {
    if d < Date::today() {
        // Overdue!
    }
}
```

## Summary

You've built a complete CLI application with:
- Command-line argument parsing
- File-based persistence
- Multiple modules
- Pattern matching for command dispatch
- Optional enhancements with colors

### Key Patterns Used

1. **`when` for command dispatch** - Clean handling of subcommands with arrow syntax
2. **Option type** - Safe handling of missing data
3. **Modules** - Organized code into task, storage, colors
4. **File I/O** - Simple text-based storage format

### Next Steps

- Add `todo edit <n> <new text>` command
- Add priority levels (high/medium/low)
- Add categories/tags
- Export to JSON or Markdown
- Add search: `todo search "keyword"`

## Complete Project Files

The complete project is available in `examples/todo-app/` in the Mana repository.

---

*Next tutorial: [Building a Game with OpenGL](opengl-quickstart.md)*
