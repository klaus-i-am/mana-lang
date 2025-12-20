# Installing Mana

This guide covers all methods for installing the Mana programming language on your system.

## Quick Install

### Windows

Open PowerShell and run:

```powershell
irm https://mana-lang.dev/install.ps1 | iex
```

### macOS / Linux

Open a terminal and run:

```bash
curl --proto '=https' --tlsv1.2 -sSf https://mana-lang.dev/install.sh | sh
```

After installation, **restart your terminal** and verify:

```bash
mana --version
```

---

## What Gets Installed

The installer sets up the complete Mana toolchain:

| Component | Location | Description |
|-----------|----------|-------------|
| `mana` | `~/.mana/bin/mana` | Main compiler |
| `mana-lsp` | `~/.mana/bin/mana-lsp` | Language Server for IDE support |
| `mana-debug` | `~/.mana/bin/mana-debug` | Debug adapter for debugging |
| `mana_runtime.h` | `~/.mana/include/` | C++ runtime header |
| Examples | `~/.mana/examples/` | Sample Mana programs |

The installer also:
- Adds `~/.mana/bin` to your PATH
- Sets the `MANA_HOME` environment variable

---

## Prerequisites

Mana compiles to C++17, so you need a C++ compiler to build the generated code.

### Windows

**Option 1: Visual Studio 2022 (Recommended)**
1. Download [Visual Studio 2022](https://visualstudio.microsoft.com/)
2. Install the "Desktop development with C++" workload
3. The `cl` compiler will be available in the Developer Command Prompt

**Option 2: MinGW-w64**
1. Download [MinGW-w64](https://www.mingw-w64.org/)
2. Add the `bin` directory to your PATH
3. Use `g++` to compile

**Option 3: LLVM/Clang**
1. Download [LLVM](https://releases.llvm.org/)
2. Add to PATH during installation
3. Use `clang++` to compile

### macOS

Install Xcode Command Line Tools:

```bash
xcode-select --install
```

This provides both `clang++` and `g++` (which is aliased to clang on macOS).

### Linux

**Debian/Ubuntu:**
```bash
sudo apt update
sudo apt install g++ build-essential
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S gcc
```

---

## Installation Options

### Windows PowerShell Options

```powershell
# Install to a custom directory
.\mana-init.ps1 -InstallDir "C:\tools\mana"

# Install without modifying PATH (manual setup)
.\mana-init.ps1 -NoModifyPath

# Non-interactive installation (skip prompts)
.\mana-init.ps1 -Yes

# Uninstall Mana
.\mana-init.ps1 -Uninstall
```

### Unix Shell Options

```bash
# Install to a custom directory
./mana-init.sh --prefix /opt/mana

# Uninstall Mana
./mana-init.sh --uninstall

# Show all options
./mana-init.sh --help
```

---

## Manual Installation

If you prefer to install manually or the automated installer doesn't work for your system:

### 1. Download the Release

Download the appropriate archive from the [releases page](https://github.com/mana-lang/mana/releases):

| Platform | Architecture | File |
|----------|-------------|------|
| Windows | x64 | `mana-X.X.X-windows-x64.zip` |
| Windows | ARM64 | `mana-X.X.X-windows-arm64.zip` |
| macOS | x64 (Intel) | `mana-X.X.X-macos-x64.tar.gz` |
| macOS | ARM64 (Apple Silicon) | `mana-X.X.X-macos-arm64.tar.gz` |
| Linux | x64 | `mana-X.X.X-linux-x64.tar.gz` |
| Linux | ARM64 | `mana-X.X.X-linux-arm64.tar.gz` |

### 2. Extract the Archive

**Windows:**
```powershell
Expand-Archive mana-X.X.X-windows-x64.zip -DestinationPath C:\mana
```

**Unix:**
```bash
tar -xzf mana-X.X.X-linux-x64.tar.gz
mv mana-X.X.X ~/.mana
```

### 3. Add to PATH

**Windows (PowerShell):**
```powershell
# Add to user PATH permanently
$path = [Environment]::GetEnvironmentVariable("PATH", "User")
[Environment]::SetEnvironmentVariable("PATH", "$path;C:\mana\bin", "User")

# Set MANA_HOME
[Environment]::SetEnvironmentVariable("MANA_HOME", "C:\mana", "User")
```

**Unix (bash/zsh):**
```bash
# Add to ~/.bashrc or ~/.zshrc
export MANA_HOME="$HOME/.mana"
export PATH="$MANA_HOME/bin:$PATH"
```

**Unix (fish):**
```fish
# Add to ~/.config/fish/config.fish
set -gx MANA_HOME "$HOME/.mana"
set -gx PATH "$MANA_HOME/bin" $PATH
```

### 4. Verify Installation

```bash
mana --version
```

---

## Building from Source

If you want the latest development version or need to build for an unsupported platform:

### Requirements

- CMake 3.14 or later
- C++17 compatible compiler
- Git (to clone the repository)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/mana-lang/mana.git
cd mana

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

# The binaries are now in build/Release/ (Windows) or build/ (Unix)
```

### Install from Local Build

After building, you can use the installer to install your local build:

**Windows:**
```powershell
.\installer\mana-init.ps1
```

**Unix:**
```bash
./installer/mana-init.sh
```

The installer will detect and use the local build automatically.

### Build Scripts

For convenience, build scripts are provided:

**Windows:**
```batch
scripts\build.bat release
scripts\build.bat test
scripts\build.bat clean
```

**Unix:**
```bash
./scripts/build.sh release
./scripts/build.sh test
./scripts/build.sh clean
```

---

## Directory Structure

After installation, your Mana installation looks like this:

```
~/.mana/                    # MANA_HOME
├── bin/
│   ├── mana               # Main compiler executable
│   ├── mana-lsp           # Language Server Protocol server
│   └── mana-debug         # Debug adapter
├── include/
│   └── mana_runtime.h     # C++ runtime header (include when compiling)
├── examples/
│   ├── hello.mana
│   ├── structs.mana
│   └── ...
└── uninstall.sh           # Uninstaller script (or .ps1 on Windows)
```

---

## Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `MANA_HOME` | Root installation directory | `~/.mana` |
| `PATH` | Should include `$MANA_HOME/bin` | Set by installer |

---

## Uninstalling

### Using the Uninstaller

**Windows:**
```powershell
# Option 1: Run the uninstaller in MANA_HOME
~\.mana\uninstall.ps1

# Option 2: Use the installer with -Uninstall
.\mana-init.ps1 -Uninstall
```

**Unix:**
```bash
# Option 1: Run the uninstaller in MANA_HOME
~/.mana/uninstall.sh

# Option 2: Use the installer with --uninstall
./mana-init.sh --uninstall
```

### Manual Uninstallation

1. Remove the installation directory:
   ```bash
   rm -rf ~/.mana
   ```

2. Remove from PATH and MANA_HOME from your shell configuration file

3. On Windows, you may also need to remove environment variables:
   ```powershell
   [Environment]::SetEnvironmentVariable("MANA_HOME", $null, "User")
   ```

---

## Troubleshooting

### "mana: command not found"

1. **Restart your terminal** - PATH changes require a new terminal session
2. Check if Mana is in your PATH:
   ```bash
   echo $PATH | tr ':' '\n' | grep mana
   ```
3. Source your shell config manually:
   ```bash
   source ~/.bashrc  # or ~/.zshrc
   ```

### "Cannot find mana_runtime.h"

When compiling generated C++ code, include the Mana include directory:

```bash
g++ -std=c++17 -I ~/.mana/include your_program.cpp -o your_program
```

Or on Windows:
```batch
cl /std:c++17 /I %USERPROFILE%\.mana\include your_program.cpp
```

### Windows: "Running scripts is disabled"

Enable PowerShell script execution:

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Windows: "curl is not recognized"

Use PowerShell's `Invoke-RestMethod` instead:

```powershell
irm https://mana-lang.dev/install.ps1 | iex
```

### Build Errors: Missing C++ Compiler

Make sure you have a C++ compiler installed (see [Prerequisites](#prerequisites)).

### Permission Denied on Unix

Make the installer executable:

```bash
chmod +x mana-init.sh
./mana-init.sh
```

---

## IDE Integration

### VS Code

Install the Mana extension for syntax highlighting, diagnostics, and IntelliSense:

1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Search for "Mana"
4. Install the extension

Or from the command line:
```bash
code --install-extension mana-lang.mana
```

The extension automatically uses `mana-lsp` for language features.

### Other Editors

Any editor supporting LSP can use `mana-lsp`. Configure your editor to use:
```
mana-lsp
```
as the language server command for `.mana` files.

---

## Updating Mana

To update to a new version, simply run the installer again:

**Windows:**
```powershell
irm https://mana-lang.dev/install.ps1 | iex
```

**Unix:**
```bash
curl --proto '=https' --tlsv1.2 -sSf https://mana-lang.dev/install.sh | sh
```

The installer will overwrite the existing installation with the new version.

---

## Getting Help

- **Documentation:** https://mana-lang.dev/docs
- **Tutorial:** See [TUTORIAL.md](TUTORIAL.md)
- **Report Issues:** https://github.com/mana-lang/mana/issues
- **Discussions:** https://github.com/mana-lang/mana/discussions
