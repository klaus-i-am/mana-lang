#!/bin/sh
# Package Mana releases for distribution
# Run this after building the compiler in Release mode.
#
# Usage: ./package-release.sh <version>
# Example: ./package-release.sh 1.0.0

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 1.0.0"
    exit 1
fi

VERSION="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"
OUTPUT_DIR="$SCRIPT_DIR/dist"

# Colors
CYAN='\033[0;36m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

step() {
    printf "\n${CYAN}==> %s${NC}\n" "$1"
}

echo ""
printf "${CYAN}  Mana Release Packager${NC}\n"
printf "  Version: %s\n" "$VERSION"
echo ""

# Detect OS and architecture
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
case "$OS" in
    darwin) OS="macos" ;;
    linux) OS="linux" ;;
    mingw*|msys*|cygwin*) OS="windows" ;;
esac

ARCH=$(uname -m)
case "$ARCH" in
    x86_64|amd64) ARCH="x64" ;;
    aarch64|arm64) ARCH="arm64" ;;
    armv7l) ARCH="armv7" ;;
esac

# Find build directory
if [ -d "$BUILD_DIR/Release" ]; then
    BUILD_DIR="$BUILD_DIR/Release"
fi

# Check build exists
if [ ! -f "$BUILD_DIR/mana" ]; then
    printf "${RED}Error: mana not found in %s${NC}\n" "$BUILD_DIR"
    printf "${YELLOW}Run './scripts/build.sh release' first.${NC}\n"
    exit 1
fi

# Create output directory
mkdir -p "$OUTPUT_DIR"

PACKAGE_NAME="mana-${VERSION}-${OS}-${ARCH}"
TEMP_DIR=$(mktemp -d)
PACKAGE_DIR="$TEMP_DIR/$PACKAGE_NAME"
TARBALL="$OUTPUT_DIR/$PACKAGE_NAME.tar.gz"

step "Creating $OS $ARCH package"

# Create package structure
mkdir -p "$PACKAGE_DIR/bin"
mkdir -p "$PACKAGE_DIR/include"
mkdir -p "$PACKAGE_DIR/examples"

# Copy binaries
echo "  Copying binaries..."
cp "$BUILD_DIR/mana" "$PACKAGE_DIR/bin/"
chmod +x "$PACKAGE_DIR/bin/mana"

if [ -f "$BUILD_DIR/mana-lsp" ]; then
    cp "$BUILD_DIR/mana-lsp" "$PACKAGE_DIR/bin/"
    chmod +x "$PACKAGE_DIR/bin/mana-lsp"
fi

if [ -f "$BUILD_DIR/mana-debug" ]; then
    cp "$BUILD_DIR/mana-debug" "$PACKAGE_DIR/bin/"
    chmod +x "$PACKAGE_DIR/bin/mana-debug"
fi

# Copy runtime header
echo "  Copying runtime header..."
cp "$REPO_ROOT/backend-cpp/mana_runtime.h" "$PACKAGE_DIR/include/"

# Copy examples
echo "  Copying examples..."
cp "$REPO_ROOT"/examples/*.mana "$PACKAGE_DIR/examples/" 2>/dev/null || true

# Copy README
echo "  Copying documentation..."
cp "$SCRIPT_DIR/README.md" "$PACKAGE_DIR/"

# Create tarball
echo "  Creating archive..."
tar -czf "$TARBALL" -C "$TEMP_DIR" "$PACKAGE_NAME"

# Cleanup
rm -rf "$TEMP_DIR"

# Calculate checksum
if command -v sha256sum >/dev/null 2>&1; then
    HASH=$(sha256sum "$TARBALL" | cut -d' ' -f1)
elif command -v shasum >/dev/null 2>&1; then
    HASH=$(shasum -a 256 "$TARBALL" | cut -d' ' -f1)
else
    HASH="(checksum tool not found)"
fi

CHECKSUM_FILE="$OUTPUT_DIR/$PACKAGE_NAME.sha256"
echo "$HASH  $PACKAGE_NAME.tar.gz" > "$CHECKSUM_FILE"

echo ""
printf "${GREEN}  Package created:${NC}\n"
echo "    $TARBALL"
echo "    SHA256: $HASH"

step "Release packaging complete!"

echo ""
echo "Files created in $OUTPUT_DIR:"
ls -1 "$OUTPUT_DIR" | while read f; do
    echo "  - $f"
done
echo ""
printf "${YELLOW}Upload these files to GitHub Releases.${NC}\n"
