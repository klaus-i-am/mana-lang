#!/bin/bash

# Mana End-to-End Test Runner Script
# Usage: run_e2e_tests.sh [options]
# Options:
#   -v, --verbose    Show detailed output
#   -k, --keep       Keep compiled artifacts

echo ""
echo "=== Mana End-to-End Tests ==="
echo ""

# Parse arguments
VERBOSE=""
KEEP=""
for arg in "$@"; do
    case $arg in
        -v|--verbose)
            VERBOSE="-v"
            ;;
        -k|--keep)
            KEEP="-k"
            ;;
    esac
done

# Check if compiler exists
if [ ! -f "build/mana" ]; then
    echo "Error: Compiler not found at build/mana"
    echo "Please build the compiler first: ./scripts/build.sh release"
    exit 1
fi

# Check if E2E test runner exists
if [ ! -f "build/mana_e2e_tests" ]; then
    echo "E2E test runner not found, building..."
    ./scripts/build.sh release
    if [ $? -ne 0 ]; then
        echo "Failed to build E2E test runner"
        exit 1
    fi
fi

# Create temp directory for compiled files
mkdir -p build/e2e_temp

# Run E2E tests
echo "Running end-to-end tests..."
echo ""

./build/mana_e2e_tests $VERBOSE $KEEP --dir tests/e2e/tests

RESULT=$?

# Cleanup temp directory if not keeping artifacts
if [ "$KEEP" != "-k" ]; then
    rm -rf build/e2e_temp 2>/dev/null
fi

exit $RESULT
