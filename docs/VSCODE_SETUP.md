# VS Code Development Setup

This guide walks you through setting up VS Code for Mana development, from installation to running your first graphics demo.

## Prerequisites

### 1. Install Visual Studio 2022 Build Tools

Mana compiles to C++17. You need the MSVC compiler:

1. Download [Visual Studio 2022](https://visualstudio.microsoft.com/downloads/) (Community is free)
2. In the installer, select **"Desktop development with C++"**
3. Make sure these are checked:
   - MSVC v143 build tools
   - Windows 11 SDK
   - C++ CMake tools

### 2. Install vcpkg (Package Manager)

vcpkg manages C++ libraries like GLFW and GLAD:

```powershell
# Clone vcpkg to C:\vcpkg (or your preferred location)
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg

# Bootstrap vcpkg
.\bootstrap-vcpkg.bat

# Add to PATH (run as admin, or add manually via System Properties)
setx /M PATH "%PATH%;C:\vcpkg"
```

### 3. Install Graphics Libraries

```powershell
# Install GLFW and GLAD for graphics demos
vcpkg install glfw3:x64-windows glad:x64-windows

# Integrate with Visual Studio/CMake
vcpkg integrate install
```

### 4. Install VS Code Extensions

Open VS Code and install these extensions:
- **C/C++** (Microsoft) - IntelliSense, debugging
- **CMake Tools** (Microsoft) - CMake integration
- **Mana** (if available) - Syntax highlighting

## Project Structure

```
mana-lang/
├── build/Release/
│   └── mana_lang.exe      # The Mana compiler
├── examples/graphics/
│   ├── .vscode/           # VS Code config (we'll create this)
│   ├── build/Release/     # Built executables
│   ├── mana_graphics.h    # Graphics FFI bridge
│   ├── mana_runtime.h     # Mana runtime
│   ├── graphics.mana      # Graphics library
│   ├── cube.mana          # Rotating cube demo
│   ├── voxel.mana         # Voxel Minecraft demo
│   └── CMakeLists.txt     # Build configuration
└── docs/
```

## Step-by-Step: Running the Cube Demo

### Step 1: Build the Mana Compiler

First, build the compiler itself:

```powershell
cd C:\dev\mana-lang
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

This creates `build/Release/mana_lang.exe`.

### Step 2: Configure the Graphics Project

```powershell
cd C:\dev\mana-lang\examples\graphics

# Configure CMake with vcpkg
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### Step 3: Compile Mana to C++

```powershell
# From the mana-lang root directory
cd C:\dev\mana-lang

# Compile cube.mana to cube.cpp
.\build\Release\mana_lang.exe examples/graphics/cube.mana -c
```

This generates `examples/graphics/cube.cpp`.

### Step 4: Build the C++ Executable

```powershell
cd examples/graphics
cmake --build build --config Release
```

This creates `build/Release/cube.exe`.

### Step 5: Run!

```powershell
.\build\Release\cube.exe
```

## One-Click Setup with VS Code Tasks

Instead of running commands manually, use VS Code tasks. Create these files in `examples/graphics/.vscode/`:

### tasks.json

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Mana: Compile Current File",
            "type": "shell",
            "command": "${workspaceFolder}/../../build/Release/mana_lang.exe",
            "args": ["${file}", "-c"],
            "group": "build",
            "problemMatcher": [],
            "presentation": {
                "echo": true,
                "reveal": "always",
                "panel": "shared"
            }
        },
        {
            "label": "CMake: Build All",
            "type": "shell",
            "command": "cmake",
            "args": ["--build", "build", "--config", "Release"],
            "group": "build",
            "problemMatcher": "$msCompile"
        },
        {
            "label": "Mana: Compile and Build",
            "dependsOn": ["Mana: Compile Current File", "CMake: Build All"],
            "dependsOrder": "sequence",
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        },
        {
            "label": "Run: cube",
            "type": "shell",
            "command": "${workspaceFolder}/build/Release/cube.exe",
            "group": "test",
            "problemMatcher": []
        },
        {
            "label": "Run: voxel",
            "type": "shell",
            "command": "${workspaceFolder}/build/Release/voxel.exe",
            "group": "test",
            "problemMatcher": []
        },
        {
            "label": "CMake: Configure",
            "type": "shell",
            "command": "cmake",
            "args": ["-B", "build", "-S", ".", "-DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake"],
            "problemMatcher": []
        }
    ]
}
```

### launch.json

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Run cube.exe",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/Release/cube.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal",
            "preLaunchTask": "Mana: Compile and Build"
        },
        {
            "name": "Run voxel.exe",
            "type": "cppvsdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/Release/voxel.exe",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "console": "integratedTerminal",
            "preLaunchTask": "Mana: Compile and Build"
        }
    ]
}
```

## Keyboard Shortcuts

After setting up tasks:

| Action | Shortcut |
|--------|----------|
| Compile + Build | `Ctrl+Shift+B` |
| Run (with debugger) | `F5` |
| Run Task | `Ctrl+Shift+P` → "Run Task" |

## Workflow

1. **Open the graphics folder** in VS Code:
   ```
   File → Open Folder → C:\dev\mana-lang\examples\graphics
   ```

2. **Edit your .mana file** (e.g., `cube.mana`)

3. **Press `Ctrl+Shift+B`** to compile and build

4. **Press `F5`** to run with debugging (or use Run Task for no debugger)

## Troubleshooting

### "mana_lang.exe not found"

Build the compiler first:
```powershell
cd C:\dev\mana-lang
cmake -B build -S .
cmake --build build --config Release
```

### "Cannot find glfw3.h" or "Cannot find glad/glad.h"

Install the libraries with vcpkg:
```powershell
vcpkg install glfw3:x64-windows glad:x64-windows
```

Then reconfigure CMake:
```powershell
cd examples/graphics
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### "LNK2019: unresolved external symbol"

Make sure vcpkg is integrated:
```powershell
vcpkg integrate install
```

### Black window or crash on startup

- Check that your graphics drivers are up to date
- Ensure you're running the Release build, not Debug
- Try running from command line to see error messages

### "graphics.mana not found" during compilation

The graphics demos import the graphics library. Make sure you're compiling from the correct directory:
```powershell
cd C:\dev\mana-lang
.\build\Release\mana_lang.exe examples/graphics/cube.mana -c
```

## Creating Your Own Demo

1. Create a new file, e.g., `examples/graphics/myproject.mana`:

```mana
module myproject

import "graphics"

fn main() -> i32 {
    let window = create_window("My Project", 800, 600)
    if window == 0 { return 1 }

    while window_open(window) {
        clear(0.2, 0.3, 0.4)  // Dark blue background
        present(window)
    }

    close_window(window)
    return 0
}
```

2. Compile it:
```powershell
.\build\Release\mana_lang.exe examples/graphics/myproject.mana -c
```

3. Rebuild (CMake auto-detects new .cpp files):
```powershell
cd examples/graphics
cmake -B build -S .
cmake --build build --config Release
```

4. Run:
```powershell
.\build\Release\myproject.exe
```

## Quick Reference

| Task | Command |
|------|---------|
| Compile .mana → .cpp | `mana_lang.exe file.mana -c` |
| Configure CMake | `cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake` |
| Build C++ | `cmake --build build --config Release` |
| Run | `./build/Release/name.exe` |
