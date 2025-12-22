# Mana Graphics Examples

Interactive 3D demos showcasing Mana's FFI capabilities with OpenGL.

## Quick Start (VS Code)

### First-Time Setup

1. **Install prerequisites** (see [docs/VSCODE_SETUP.md](../../docs/VSCODE_SETUP.md)):
   - Visual Studio 2022 with C++ tools
   - vcpkg with `glfw3` and `glad` installed

2. **Build the Mana compiler** (from repo root):
   ```powershell
   cmake -B build -S .
   cmake --build build --config Release
   ```

3. **Open this folder in VS Code**:
   ```
   File → Open Folder → examples/graphics
   ```

4. **Configure CMake** (run once):
   - Press `Ctrl+Shift+P` → "Run Task" → "CMake: Configure"

### Running Demos

1. Open a `.mana` file (e.g., `cube.mana`)
2. Press `Ctrl+Shift+B` to compile and build
3. Press `F5` to run

Or use tasks: `Ctrl+Shift+P` → "Run Task" → "Run: cube"

## Available Demos

| Demo | Description | Controls |
|------|-------------|----------|
| `triangle.mana` | Basic colored triangle | - |
| `cube.mana` | Rotating 3D cube | Arrows rotate, WASD move, Q/E roll, ESC quit |
| `voxel.mana` | 9x9 Minecraft-style terrain | WASD move, Arrows look, Q/E fly, Space reset |

## Creating Your Own

```mana
module mydemo

import "graphics"

fn main() -> i32 {
    let window = create_window("My Demo", 800, 600)
    if window == 0 { return 1 }

    while window_open(window) {
        clear(0.1, 0.2, 0.3)
        // Your rendering here
        present(window)
    }

    close_window(window)
    return 0
}
```

Then:
1. Save as `mydemo.mana`
2. Press `Ctrl+Shift+B` to compile
3. Press `F5` to run

## Troubleshooting

**"mana_lang.exe not found"**
→ Build the compiler first (see step 2 above)

**"Cannot find glfw3.h"**
→ Run: `vcpkg install glfw3:x64-windows glad:x64-windows`

**Build errors after adding new .mana file**
→ Run "CMake: Configure" task to pick up the new .cpp file

See [docs/VSCODE_SETUP.md](../../docs/VSCODE_SETUP.md) for detailed setup instructions.
