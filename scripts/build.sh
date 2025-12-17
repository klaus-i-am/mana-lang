#!/bin/bash
# Mana Language Build Script for Unix/Linux/macOS
# Usage: ./build.sh [debug|release|clean|test|examples]

set -e

BUILD_TYPE="Release"
BUILD_DIR="build"
ACTION="build"

# Parse arguments
case "$1" in
    debug)
        BUILD_TYPE="Debug"
        ACTION="build"
        ;;
    release)
        BUILD_TYPE="Release"
        ACTION="build"
        ;;
    clean)
        ACTION="clean"
        ;;
    test)
        ACTION="test"
        ;;
    examples)
        ACTION="examples"
        ;;
    all)
        ACTION="all"
        ;;
    help|--help|-h)
        echo ""
        echo "Mana Language Build Script"
        echo ""
        echo "Usage: ./build.sh [command]"
        echo ""
        echo "Commands:"
        echo "  debug     Build compiler in debug mode"
        echo "  release   Build compiler in release mode (default)"
        echo "  clean     Remove all build artifacts"
        echo "  test      Build and run tests"
        echo "  examples  Build all example programs"
        echo "  all       Build compiler, examples, and run tests"
        echo "  help      Show this help message"
        echo ""
        exit 0
        ;;
    "")
        # Default to build
        ;;
    *)
        echo "Unknown option: $1"
        echo "Use './build.sh help' for usage information"
        exit 1
        ;;
esac

build_compiler() {
    echo ""
    echo "=== Building Mana Compiler ($BUILD_TYPE) ==="
    echo ""
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    cmake --build . --config "$BUILD_TYPE" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    cd ..
    echo ""
    echo "Compiler built: $BUILD_DIR/mana_lang"
}

build_examples() {
    echo ""
    echo "=== Building Examples ==="
    echo ""
    mkdir -p "examples/build"
    cd "examples/build"
    cmake ..
    cmake --build . --config "$BUILD_TYPE" -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    cd ../..
    echo ""
    echo "Examples built in: examples/build"
}

run_tests() {
    echo ""
    echo "=== Running Tests ==="
    echo ""
    COMPILER="$BUILD_DIR/mana_lang"
    if [ ! -f "$COMPILER" ]; then
        echo "Error: Compiler not found. Build first."
        exit 1
    fi

    # Run built-in test runner
    "$COMPILER" examples/testing_example.mana --test -c
    if [ -f "examples/build/testing_example" ]; then
        ./examples/build/testing_example
    fi
}

clean_build() {
    echo "Cleaning build directories..."
    rm -rf "$BUILD_DIR"
    rm -rf "examples/build"
    echo "Done."
}

# Execute action
case "$ACTION" in
    clean)
        clean_build
        ;;
    build)
        build_compiler
        ;;
    test)
        build_compiler
        build_examples
        run_tests
        ;;
    examples)
        build_compiler
        build_examples
        ;;
    all)
        build_compiler
        build_examples
        run_tests
        ;;
esac
