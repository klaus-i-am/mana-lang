#!/bin/sh
# Mana Programming Language Installer for Unix-like systems
# Usage: curl --proto '=https' --tlsv1.2 -sSf https://mana-lang.dev/install.sh | sh
#
# This script installs the Mana compiler toolchain including:
# - mana (compiler)
# - mana-lsp (language server)
# - mana-debug (debug adapter)
# - mana_runtime.h (C++ runtime header)

set -u

# ============================================================================
# Configuration
# ============================================================================

MANA_VERSION="1.0.0"
MANA_REPO="https://github.com/mana-lang/mana"
MANA_RELEASE_URL="https://github.com/mana-lang/mana/releases/download/v${MANA_VERSION}"

# Default installation directory
MANA_HOME="${MANA_HOME:-$HOME/.mana}"

# ============================================================================
# Terminal Colors
# ============================================================================

if [ -t 1 ]; then
    tty_escape() { printf "\033[%sm" "$1"; }
else
    tty_escape() { :; }
fi

tty_mkbold() { tty_escape "1;$1"; }
tty_cyan="$(tty_mkbold 36)"
tty_yellow="$(tty_mkbold 33)"
tty_green="$(tty_mkbold 32)"
tty_red="$(tty_mkbold 31)"
tty_white="$(tty_mkbold 37)"
tty_reset="$(tty_escape 0)"

# ============================================================================
# Output Functions
# ============================================================================

print_banner() {
    printf "\n"
    printf "%s  __  __                   %s\n" "$tty_cyan" "$tty_reset"
    printf "%s |  \\/  | __ _ _ __   __ _ %s\n" "$tty_cyan" "$tty_reset"
    printf "%s | |\\/| |/ _\` | '_ \\ / _\` |%s\n" "$tty_cyan" "$tty_reset"
    printf "%s | |  | | (_| | | | | (_| |%s\n" "$tty_cyan" "$tty_reset"
    printf "%s |_|  |_|\\__,_|_| |_|\\__,_|%s\n" "$tty_cyan" "$tty_reset"
    printf "\n"
    printf " The Mana Programming Language Installer\n"
    printf " Version %s\n" "$MANA_VERSION"
    printf "\n"
}

section() {
    printf "\n%s==> %s%s\n" "$tty_yellow" "$1" "$tty_reset"
}

info() {
    printf "    %s\n" "$1"
}

success() {
    printf "    %s[OK]%s %s\n" "$tty_green" "$tty_reset" "$1"
}

warn() {
    printf "    %s[!]%s %s\n" "$tty_yellow" "$tty_reset" "$1"
}

error() {
    printf "    %s[X]%s %s\n" "$tty_red" "$tty_reset" "$1"
}

abort() {
    printf "%sError: %s%s\n" "$tty_red" "$1" "$tty_reset" >&2
    exit 1
}

# ============================================================================
# Utility Functions
# ============================================================================

command_exists() {
    command -v "$1" >/dev/null 2>&1
}

get_os() {
    case "$(uname -s)" in
        Linux*)  echo "linux" ;;
        Darwin*) echo "macos" ;;
        MINGW*|MSYS*|CYGWIN*) echo "windows" ;;
        *)       echo "unknown" ;;
    esac
}

get_arch() {
    case "$(uname -m)" in
        x86_64|amd64)  echo "x64" ;;
        aarch64|arm64) echo "arm64" ;;
        armv7l)        echo "armv7" ;;
        i686|i386)     echo "x86" ;;
        *)             echo "unknown" ;;
    esac
}

get_shell_profile() {
    case "$SHELL" in
        */zsh)  echo "$HOME/.zshrc" ;;
        */bash)
            if [ -f "$HOME/.bash_profile" ]; then
                echo "$HOME/.bash_profile"
            else
                echo "$HOME/.bashrc"
            fi
            ;;
        */fish) echo "$HOME/.config/fish/config.fish" ;;
        *)      echo "$HOME/.profile" ;;
    esac
}

download() {
    url="$1"
    output="$2"

    if command_exists curl; then
        curl --proto '=https' --tlsv1.2 -sSfL "$url" -o "$output"
    elif command_exists wget; then
        wget -q "$url" -O "$output"
    else
        abort "Neither curl nor wget found. Please install one."
    fi
}

# ============================================================================
# Installation Functions
# ============================================================================

install_mana() {
    bin_dir="$MANA_HOME/bin"
    include_dir="$MANA_HOME/include"

    os=$(get_os)
    arch=$(get_arch)

    section "Installation Options"
    info "Installation directory: $MANA_HOME"
    info "Binary directory: $bin_dir"
    info "Include directory: $include_dir"
    info "Operating system: $os"
    info "Architecture: $arch"

    if [ "$os" = "unknown" ]; then
        abort "Unsupported operating system: $(uname -s)"
    fi

    if [ "$arch" = "unknown" ]; then
        abort "Unsupported architecture: $(uname -m)"
    fi

    printf "\n    Proceed with installation? [Y/n] "
    read -r response
    case "$response" in
        [nN]*)
            warn "Installation cancelled."
            exit 0
            ;;
    esac

    # Create directories
    section "Creating directories"
    for dir in "$MANA_HOME" "$bin_dir" "$include_dir"; do
        if [ ! -d "$dir" ]; then
            mkdir -p "$dir"
            success "Created $dir"
        else
            info "$dir already exists"
        fi
    done

    # Check if we're doing a local install (from source) or download
    script_dir="$(cd "$(dirname "$0")" && pwd)"
    local_build="$script_dir/../build"

    if [ -f "$local_build/mana" ] || [ -f "$local_build/Release/mana" ]; then
        section "Installing from local build"
        copy_local_build "$bin_dir" "$include_dir" "$script_dir"
    else
        section "Downloading Mana $MANA_VERSION"
        download_mana "$bin_dir" "$include_dir" "$os" "$arch"
    fi

    # Configure shell
    section "Configuring environment"
    configure_shell "$bin_dir"

    # Create uninstaller
    section "Creating uninstaller"
    create_uninstaller
    success "Created uninstall.sh"

    # Check prerequisites
    section "Checking prerequisites"
    check_prerequisites

    # Installation complete
    section "Installation complete!"
    printf "\n"
    printf "    %sMana has been installed to: %s%s\n" "$tty_green" "$MANA_HOME" "$tty_reset"
    printf "\n"
    info "To get started, restart your terminal or run:"
    printf "\n"
    printf "        %ssource %s%s\n" "$tty_cyan" "$(get_shell_profile)" "$tty_reset"
    printf "\n"
    info "Then try:"
    printf "\n"
    printf "        %smana --version%s\n" "$tty_cyan" "$tty_reset"
    printf "\n"
    info "To compile a Mana program:"
    printf "\n"
    printf "        %smana hello.mana -o hello.cpp%s\n" "$tty_cyan" "$tty_reset"
    printf "\n"
    info "Documentation: https://mana-lang.dev/docs"
    info "Report issues: $MANA_REPO/issues"
    printf "\n"
}

copy_local_build() {
    bin_dir="$1"
    include_dir="$2"
    script_dir="$3"
    source_dir="$script_dir/.."

    # Find the build directory
    if [ -d "$source_dir/build/Release" ]; then
        build_dir="$source_dir/build/Release"
    elif [ -d "$source_dir/build/Debug" ]; then
        build_dir="$source_dir/build/Debug"
    else
        build_dir="$source_dir/build"
    fi

    # Copy executables
    for bin in mana mana-lsp mana-debug; do
        if [ -f "$build_dir/$bin" ]; then
            cp "$build_dir/$bin" "$bin_dir/"
            chmod +x "$bin_dir/$bin"
            success "Installed $bin"
        else
            warn "$bin not found, skipping"
        fi
    done

    # Copy runtime header
    runtime_src="$source_dir/backend-cpp/mana_runtime.h"
    if [ -f "$runtime_src" ]; then
        cp "$runtime_src" "$include_dir/"
        success "Installed mana_runtime.h"
    fi

    # Copy examples
    examples_src="$source_dir/examples"
    examples_dst="$MANA_HOME/examples"
    if [ -d "$examples_src" ]; then
        mkdir -p "$examples_dst"
        cp "$examples_src"/*.mana "$examples_dst/" 2>/dev/null || true
        success "Installed examples"
    fi
}

download_mana() {
    bin_dir="$1"
    include_dir="$2"
    os="$3"
    arch="$4"

    archive_name="mana-${MANA_VERSION}-${os}-${arch}.tar.gz"
    download_url="${MANA_RELEASE_URL}/${archive_name}"
    temp_dir=$(mktemp -d)
    temp_archive="$temp_dir/$archive_name"

    info "Downloading from $download_url"

    if ! download "$download_url" "$temp_archive"; then
        rm -rf "$temp_dir"
        error "Failed to download Mana"
        printf "\n"
        info "You can manually download from: $MANA_REPO/releases"
        info "Or build from source with: ./scripts/build.sh release"
        exit 1
    fi

    success "Downloaded $archive_name"

    # Extract
    tar -xzf "$temp_archive" -C "$temp_dir"
    success "Extracted archive"

    # Copy files
    if [ -d "$temp_dir/bin" ]; then
        cp "$temp_dir"/bin/* "$bin_dir/"
    else
        cp "$temp_dir"/mana* "$bin_dir/" 2>/dev/null || true
    fi

    chmod +x "$bin_dir"/*

    if [ -d "$temp_dir/include" ]; then
        cp "$temp_dir"/include/* "$include_dir/"
    fi

    success "Installed Mana binaries"

    # Cleanup
    rm -rf "$temp_dir"
}

configure_shell() {
    bin_dir="$1"
    profile=$(get_shell_profile)

    # Check if already configured
    if grep -q "MANA_HOME" "$profile" 2>/dev/null; then
        info "Shell already configured"
        return
    fi

    # Determine the config block based on shell
    case "$SHELL" in
        */fish)
            config_block="
# Mana Programming Language
set -gx MANA_HOME \"$MANA_HOME\"
set -gx PATH \"\$MANA_HOME/bin\" \$PATH
"
            ;;
        *)
            config_block="
# Mana Programming Language
export MANA_HOME=\"$MANA_HOME\"
export PATH=\"\$MANA_HOME/bin:\$PATH\"
"
            ;;
    esac

    # Append to profile
    printf "%s" "$config_block" >> "$profile"
    success "Added Mana to $profile"
}

create_uninstaller() {
    cat > "$MANA_HOME/uninstall.sh" << 'UNINSTALL_EOF'
#!/bin/sh
# Mana Uninstaller

MANA_HOME="${MANA_HOME:-$HOME/.mana}"

printf "Uninstalling Mana from %s...\n" "$MANA_HOME"

# Remove installation directory
if [ -d "$MANA_HOME" ]; then
    rm -rf "$MANA_HOME"
    printf "Removed %s\n" "$MANA_HOME"
fi

# Note about shell configuration
printf "\n"
printf "NOTE: You may want to remove the Mana configuration from your shell profile.\n"
printf "Look for lines containing 'MANA_HOME' in:\n"
printf "  - ~/.bashrc or ~/.bash_profile\n"
printf "  - ~/.zshrc\n"
printf "  - ~/.config/fish/config.fish\n"
printf "\n"
printf "Mana has been uninstalled.\n"
printf "Thank you for using Mana!\n"
UNINSTALL_EOF

    chmod +x "$MANA_HOME/uninstall.sh"
}

check_prerequisites() {
    # Check for C++ compiler
    has_compiler=0

    if command_exists clang++; then
        success "clang++ found"
        has_compiler=1
    fi

    if command_exists g++; then
        success "g++ found"
        has_compiler=1
    fi

    if [ "$has_compiler" -eq 0 ]; then
        warn "No C++ compiler detected"
        info "  Mana compiles to C++, so you'll need a C++ compiler."
        os=$(get_os)
        case "$os" in
            macos)
                info "  Install with: xcode-select --install"
                ;;
            linux)
                info "  Install with: sudo apt install g++ (Debian/Ubuntu)"
                info "            or: sudo dnf install gcc-c++ (Fedora)"
                ;;
        esac
    fi

    # Check for CMake
    if command_exists cmake; then
        success "cmake found"
    else
        info "cmake not found (optional, for building projects)"
    fi
}

# ============================================================================
# Uninstall Function
# ============================================================================

uninstall_mana() {
    section "Uninstalling Mana"

    if [ ! -d "$MANA_HOME" ]; then
        warn "Mana is not installed at $MANA_HOME"
        exit 0
    fi

    info "This will remove Mana from: $MANA_HOME"
    printf "    Are you sure? [y/N] "
    read -r response
    case "$response" in
        [yY]*)
            ;;
        *)
            warn "Uninstall cancelled."
            exit 0
            ;;
    esac

    rm -rf "$MANA_HOME"
    success "Removed $MANA_HOME"

    printf "\n"
    printf "%sMana has been uninstalled.%s\n" "$tty_green" "$tty_reset"
    printf "\n"
    info "NOTE: You may want to remove the Mana configuration from your shell profile."
    info "Look for lines containing 'MANA_HOME' in your shell config file."
    printf "\n"
    printf "Thank you for using Mana!\n"
}

# ============================================================================
# Parse Arguments
# ============================================================================

UNINSTALL=0

while [ $# -gt 0 ]; do
    case "$1" in
        --uninstall)
            UNINSTALL=1
            ;;
        --prefix)
            shift
            MANA_HOME="$1"
            ;;
        --help|-h)
            printf "Mana Installer\n\n"
            printf "Usage: %s [OPTIONS]\n\n" "$0"
            printf "Options:\n"
            printf "  --prefix <DIR>   Install to custom directory (default: ~/.mana)\n"
            printf "  --uninstall      Uninstall Mana\n"
            printf "  -h, --help       Show this help message\n"
            exit 0
            ;;
        *)
            abort "Unknown option: $1"
            ;;
    esac
    shift
done

# ============================================================================
# Main
# ============================================================================

print_banner

if [ "$UNINSTALL" -eq 1 ]; then
    uninstall_mana
else
    install_mana
fi
