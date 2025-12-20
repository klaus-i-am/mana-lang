# Mana Installer

The Mana installer provides a simple way to install the Mana programming language toolchain on Windows, macOS, and Linux.

## Quick Install

### Windows (PowerShell)

```powershell
irm https://mana-lang.dev/install.ps1 | iex
```

Or download and run manually:
```powershell
.\mana-init.ps1
```

### macOS / Linux

```bash
curl --proto '=https' --tlsv1.2 -sSf https://mana-lang.dev/install.sh | sh
```

Or download and run manually:
```bash
chmod +x mana-init.sh
./mana-init.sh
```

## What Gets Installed

The installer will:

1. **Create the Mana home directory** at `~/.mana` (or `%USERPROFILE%\.mana` on Windows)
2. **Install binaries** to `~/.mana/bin`:
   - `mana` - The Mana compiler
   - `mana-lsp` - Language Server Protocol server for IDE support
   - `mana-debug` - Debug adapter for IDE debugging
3. **Install headers** to `~/.mana/include`:
   - `mana_runtime.h` - C++ runtime header for compiled programs
4. **Install examples** to `~/.mana/examples`
5. **Configure your shell** by adding `~/.mana/bin` to your PATH

## Installation Options

### Windows

```powershell
# Install to custom directory
.\mana-init.ps1 -InstallDir "C:\tools\mana"

# Skip PATH modification
.\mana-init.ps1 -NoModifyPath

# Non-interactive install (skip prompts)
.\mana-init.ps1 -Yes

# Uninstall
.\mana-init.ps1 -Uninstall
```

### Unix

```bash
# Install to custom directory
./mana-init.sh --prefix /opt/mana

# Uninstall
./mana-init.sh --uninstall

# Show help
./mana-init.sh --help
```

## Uninstalling

### Windows

```powershell
# Option 1: Use the uninstaller
~\.mana\uninstall.ps1

# Option 2: Use the installer with -Uninstall flag
.\mana-init.ps1 -Uninstall

# Option 3: Use standalone uninstaller
.\mana-uninstall.ps1
```

### Unix

```bash
# Option 1: Use the uninstaller
~/.mana/uninstall.sh

# Option 2: Use the installer with --uninstall flag
./mana-init.sh --uninstall
```

## Prerequisites

Mana compiles to C++, so you'll need a C++ compiler to compile the generated code:

### Windows
- **Visual Studio 2022** with "Desktop development with C++" workload
- Or **MinGW-w64** with g++
- Or **LLVM/Clang**

### macOS
- **Xcode Command Line Tools**: `xcode-select --install`

### Linux
- **g++**: `sudo apt install g++` (Debian/Ubuntu) or `sudo dnf install gcc-c++` (Fedora)
- Or **clang++**: `sudo apt install clang`

## Environment Variables

After installation, the following environment variables are set:

| Variable | Description |
|----------|-------------|
| `MANA_HOME` | Root installation directory (e.g., `~/.mana`) |
| `PATH` | Updated to include `$MANA_HOME/bin` |

## Directory Structure

```
~/.mana/
├── bin/
│   ├── mana          # Main compiler
│   ├── mana-lsp      # Language server
│   └── mana-debug    # Debug adapter
├── include/
│   └── mana_runtime.h  # C++ runtime header
├── examples/
│   ├── hello.mana
│   └── ...
└── uninstall.sh      # Uninstaller script
```

## Installing from Source

If you have the Mana source code and have built it locally, you can run the installer from the `installer/` directory:

```bash
# Build first
./scripts/build.sh release

# Then install (will detect local build)
./installer/mana-init.sh
```

The installer will automatically detect and use the local build instead of downloading from the internet.

## Verifying Installation

After installation, open a new terminal and run:

```bash
mana --version
```

You should see output like:
```
Mana Programming Language v1.0.0
```

## Getting Started

1. Create a file `hello.mana`:
   ```mana
   fn main() {
       println("Hello, World!")
   }
   ```

2. Compile it:
   ```bash
   mana hello.mana -o hello.cpp
   ```

3. Compile the C++ output:
   ```bash
   g++ -std=c++17 -I ~/.mana/include hello.cpp -o hello
   ./hello
   ```

## Troubleshooting

### "mana: command not found"

Your shell may not have picked up the PATH changes. Try:
- Opening a new terminal window
- Running `source ~/.bashrc` (or `~/.zshrc` for Zsh)
- Checking that `~/.mana/bin` is in your PATH: `echo $PATH`

### "Cannot find mana_runtime.h"

When compiling the generated C++, include the Mana include directory:
```bash
g++ -I ~/.mana/include your_program.cpp -o your_program
```

### Windows: "Running scripts is disabled"

Enable PowerShell script execution:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

## Support

- Documentation: https://mana-lang.dev/docs
- Issues: https://github.com/mana-lang/mana/issues
- Discussions: https://github.com/mana-lang/mana/discussions
