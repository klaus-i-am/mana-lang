# Getting Started with Mana

Create your first Mana program from scratch.

## Prerequisites

1. **Download Mana** from [releases](https://github.com/klaus-i-am/mana-lang/releases) or build from source
2. **Install a C++ compiler**:
   - Windows: [Visual Studio 2022](https://visualstudio.microsoft.com/) (Community, free)
   - Mac: `xcode-select --install`
   - Linux: `sudo apt install g++`

## Your First Program

### Step 1: Create a Project Folder

```
my-project/
├── hello.mana
└── mana_runtime.h    (copy from mana-lang/examples/)
```

### Step 2: Write Your Code

Create `hello.mana`:

```mana
module hello

fn main() -> i32 {
    println("Hello, Mana!")

    let name = "World"
    println("Hello, ", name, "!")

    for i in 1..=5 {
        println("Count: ", i)
    }

    return 0
}
```

### Step 3: Copy the Runtime Header

Copy `mana_runtime.h` from the Mana repository's `examples/` folder into your project folder.

### Step 4: Compile Mana → C++

```bash
mana_lang hello.mana -c
```

This creates `hello.cpp`.

### Step 5: Compile C++ → Executable

**Windows (Visual Studio):**
```powershell
cl /EHsc /std:c++17 hello.cpp /Fe:hello.exe
```

**Windows (MinGW):**
```bash
g++ -std=c++17 hello.cpp -o hello.exe
```

**Mac/Linux:**
```bash
g++ -std=c++17 hello.cpp -o hello
```

### Step 6: Run

```bash
./hello
```

Output:
```
Hello, Mana!
Hello, World!
Count: 1
Count: 2
Count: 3
Count: 4
Count: 5
```

---

## VS Code Setup (Recommended)

For a better experience, set up VS Code with automatic compile & run.

### Project Structure

```
my-project/
├── .vscode/
│   └── tasks.json
├── src/
│   └── main.mana
├── mana_runtime.h
└── build/           (created automatically)
```

### tasks.json

Create `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Compile Mana",
            "type": "shell",
            "command": "mana_lang",
            "args": ["${file}", "-c"],
            "problemMatcher": []
        },
        {
            "label": "Build C++",
            "type": "shell",
            "command": "g++",
            "args": [
                "-std=c++17",
                "${fileDirname}/${fileBasenameNoExtension}.cpp",
                "-o",
                "${workspaceFolder}/build/${fileBasenameNoExtension}"
            ],
            "problemMatcher": "$gcc"
        },
        {
            "label": "Compile and Build",
            "dependsOn": ["Compile Mana", "Build C++"],
            "dependsOrder": "sequence",
            "group": {
                "kind": "build",
                "isDefault": true
            }
        },
        {
            "label": "Run",
            "type": "shell",
            "command": "${workspaceFolder}/build/${fileBasenameNoExtension}",
            "problemMatcher": [],
            "dependsOn": "Compile and Build"
        }
    ]
}
```

**For Windows with MSVC**, replace the "Build C++" task:

```json
{
    "label": "Build C++",
    "type": "shell",
    "command": "cl",
    "args": [
        "/EHsc",
        "/std:c++17",
        "${fileDirname}/${fileBasenameNoExtension}.cpp",
        "/Fe:${workspaceFolder}/build/${fileBasenameNoExtension}.exe"
    ],
    "problemMatcher": "$msCompile"
}
```

### Usage

1. Open your project folder in VS Code
2. Create `build/` folder: `mkdir build`
3. Open a `.mana` file
4. Press `Ctrl+Shift+B` to compile and build
5. Run with `Ctrl+Shift+P` → "Run Task" → "Run"

---

## Adding Graphics

To create graphics programs, you need additional setup.

### Install Graphics Libraries

**Windows (vcpkg):**
```powershell
vcpkg install glfw3:x64-windows glad:x64-windows
```

**Mac:**
```bash
brew install glfw
```

**Linux:**
```bash
sudo apt install libglfw3-dev libgl1-mesa-dev
```

### Project Structure for Graphics

```
my-graphics-project/
├── .vscode/
│   └── tasks.json
├── main.mana
├── graphics.mana       (copy from mana-lang/examples/graphics/)
├── mana_runtime.h      (copy from mana-lang/examples/)
├── mana_graphics.h     (copy from mana-lang/examples/graphics/)
└── CMakeLists.txt
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_graphics_project)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

file(GLOB SOURCES "*.cpp")

foreach(SOURCE ${SOURCES})
    get_filename_component(NAME ${SOURCE} NAME_WE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE glfw glad::glad)
    if(WIN32)
        target_link_libraries(${NAME} PRIVATE opengl32)
    endif()
endforeach()
```

### Example Graphics Program

Create `window.mana`:

```mana
module window

import "graphics"

fn main() -> i32 {
    let window = create_window("My Window", 800, 600)
    if window == 0 { return 1 }

    while window_open(window) {
        clear(0.2, 0.3, 0.4)
        present(window)
    }

    close_window(window)
    return 0
}
```

### Build and Run

```bash
# Compile Mana files
mana_lang graphics.mana -c
mana_lang window.mana -c

# Configure and build with CMake
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release

# Run
./build/Release/window.exe
```

---

## Language Quick Reference

### Variables

```mana
let x = 42              // Immutable, inferred type
let mut count = 0       // Mutable
let pi: f64 = 3.14159   // Explicit type
```

### Functions

```mana
fn add(a: i32, b: i32) -> i32 {
    return a + b
}

fn greet(name: string) {
    println("Hello, ", name)
}
```

### Control Flow

```mana
// If/else
if x > 0 {
    println("positive")
} else {
    println("not positive")
}

// For loops
for i in 0..10 { }         // 0 to 9
for i in 0..=10 { }        // 0 to 10

// While loops
while condition {
    // ...
}
```

### Structs

```mana
struct Point {
    x: f32,
    y: f32
}

let p = Point { x: 1.0, y: 2.0 }
println(p.x)
```

### Enums

```mana
enum Color {
    Red,
    Green,
    Blue
}

let c = Color::Red
```

---

## Common Issues

### "mana_lang: command not found"

Add Mana to your PATH:
```bash
# Windows PowerShell
$env:PATH += ";C:\path\to\mana-lang\build\Release"

# Or add permanently via System Properties → Environment Variables
```

### "mana_runtime.h: No such file"

Copy `mana_runtime.h` to your project folder:
```bash
cp /path/to/mana-lang/examples/mana_runtime.h ./
```

### "undefined reference to mana::println"

The runtime is header-only. Make sure `mana_runtime.h` is in the same folder as your `.cpp` file, or use `-I` to specify the include path:
```bash
g++ -std=c++17 -I/path/to/mana-lang/examples hello.cpp -o hello
```

---

## Next Steps

- Read the [Language Reference](LANGUAGE_REFERENCE.md) for full syntax
- Check out [examples/](../examples/) for more code samples
- Try the [CLI App Tutorial](TUTORIAL_CLI_APP.md)
- Build a [3D Graphics Demo](../examples/graphics/)
