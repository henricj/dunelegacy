#!/bin/sh
#
# Stress test for determinism: loop until failure, generating a new random seed each iteration.
#
# This script repeatedly runs two instances of headless_hash_runner with identical
# random seeds and verifies they produce identical SHA-384 hashes. Useful for detecting
# sporadic non-determinism bugs that might only appear with specific game states.
#
# Usage: ./stress_determinism.sh [map_file] [ticks] [runner_executable]
#
# Defaults:
#   map_file:          SCENF001.INI
#   ticks:             500
#   runner_executable: ./headless_hash_runner (or ./headless_hash_runner.exe on Windows)

set -e

MAP_FILE="${1:-SCENF001.INI}"
TICKS="${2:-500}"
RUNNER="${3:-./headless_hash_runner}"

# Try .exe variant on Windows if not found
if [ ! -x "$RUNNER" ] && [ ! -f "$RUNNER" ]; then
    if [ -x "$RUNNER.exe" ] || [ -f "$RUNNER.exe" ]; then
        RUNNER="$RUNNER.exe"
    fi
fi

if [ ! -x "$RUNNER" ] && [ ! -f "$RUNNER" ]; then
    echo "Error: Runner not found: $RUNNER" >&2
    exit 1
fi

echo "Stress testing determinism"
echo "  Runner: $RUNNER"
echo "  Map: $MAP_FILE"
echo "  Ticks per run: $TICKS"
echo ""

ITERATION=0
PASSED=0

while true; do
    ITERATION=$((ITERATION + 1))

    # Generate a random 64-byte seed via openssl and convert to hex
    SEED_HEX=$(openssl rand -hex 64)
    if [ $? -ne 0 ]; then
        echo "Error: openssl rand failed" >&2
        exit 1
    fi

    # Verify seed is 128 hex characters (64 bytes)
    SEED_LEN=${#SEED_HEX}
    if [ "$SEED_LEN" -ne 128 ]; then
        echo "Error: Invalid seed length: $SEED_LEN (expected 128)" >&2
        exit 1
    fi

    # Show iteration header
    SEED_PREFIX=$(printf "%.16s" "$SEED_HEX")
    printf "[$ITERATION] seed=%s... " "$SEED_PREFIX"

    # Run both instances in parallel using temp files for output
    TMPDIR="${TMPDIR:-/tmp}"
    HASH1_FILE="$TMPDIR/hash1_$$.txt"
    HASH2_FILE="$TMPDIR/hash2_$$.txt"

    "$RUNNER" "$MAP_FILE" "$TICKS" "$SEED_HEX" > "$HASH1_FILE" 2>/dev/null &
    PID1=$!

    "$RUNNER" "$MAP_FILE" "$TICKS" "$SEED_HEX" > "$HASH2_FILE" 2>/dev/null
    EXIT2=$?

    wait $PID1
    EXIT1=$?

    # Clean up temp files and check exit codes
    HASH1=$(cat "$HASH1_FILE" 2>/dev/null || true)
    HASH2=$(cat "$HASH2_FILE" 2>/dev/null || true)
    rm -f "$HASH1_FILE" "$HASH2_FILE"

    if [ $EXIT1 -ne 0 ]; then
        echo ""
        echo "Error: Run 1 failed (exit code $EXIT1)" >&2
        exit 1
    fi

    if [ $EXIT2 -ne 0 ]; then
        echo ""
        echo "Error: Run 2 failed (exit code $EXIT2)" >&2
        exit 1
    fi

    if [ -z "$HASH1" ] || [ -z "$HASH2" ]; then
        echo ""
        echo "Error: No output from one or both runs" >&2
        exit 1
    fi

    # Compare hashes
    if [ "$HASH1" = "$HASH2" ]; then
        HASH_PREFIX=$(printf "%.16s" "$HASH1")
        echo "✓ PASS (hash=$HASH_PREFIX...)"
        PASSED=$((PASSED + 1))
    else
        echo "✗ FAIL"
        echo "  Run 1: $HASH1"
        echo "  Run 2: $HASH2"
        echo ""
        echo "Stopped after $ITERATION iterations ($PASSED passed, 1 failed)"
        exit 1
    fi
done
