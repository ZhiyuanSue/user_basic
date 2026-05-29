#!/bin/bash
# Simple script to run all user payload tests on qemu-user

ARCH=$1

if [ -z "$ARCH" ]; then
    echo "Usage: $0 <arch>"
    echo "Example: $0 aarch64"
    echo "Supported: aarch64, x86_64"
    exit 1
fi

QEMU="qemu-${ARCH}-static"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TEST_DIR="$SCRIPT_DIR/../user/build/${ARCH}"

# Check qemu-user exists
if ! command -v $QEMU &>/dev/null; then
    echo "ERROR: $QEMU not found"
    exit 1
fi

# Check test directory exists, if not build it
if [ ! -d "$TEST_DIR" ]; then
    echo "Test directory not found, building $ARCH..."
    cd "$SCRIPT_DIR/../.."
    make user ARCH=$ARCH || {
        echo "ERROR: Build failed for $ARCH"
        exit 1
    }
    echo "Build complete for $ARCH"
fi

# Save original directory and setup cleanup
ORIGINAL_DIR="$(pwd)"

cleanup_temp_files() {
    cd "$SCRIPT_DIR/../.."
    rm -f test_*.txt test_*.bin
    rm -rf test_dir
    cd "$ORIGINAL_DIR"
}

# Clean up before starting
cleanup_temp_files

# Clean up on exit
trap cleanup_temp_files EXIT

passed=0
failed=0

echo "Running tests for $ARCH..."
echo "================================"

for test in ${TEST_DIR}/*; do
    if [ ! -f "$test" ]; then
        continue
    fi

    test_name=$(basename "$test")
    printf "%-30s" "$test_name"

    if timeout 5s ${QEMU} "$test" >/dev/null 2>&1; then
        echo "✓ PASS"
        ((passed++))
    else
        echo "✗ FAIL"
        ((failed++))
    fi
done

echo "================================"
echo "Total: $((passed + failed))"
echo "Passed: $passed"
echo "Failed: $failed"
