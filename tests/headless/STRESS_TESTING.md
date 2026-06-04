# Determinism Stress Testing

## Quick Start

### Run a single determinism check
```bash
cd path/to/runner/directory
/path/to/stress_determinism.sh [map_file] [ticks] [runner_executable] [--max-units=N] [--credits=N]
```

Defaults:
- `map_file`: `SCENF001.INI`
- `ticks`: `500`
- `runner_executable`: `./headless_hash_runner` (or `./headless_hash_runner.exe` on Windows)

Optional stress-test parameters:
- `--max-units=N`: Override MaxUnits in all factions (e.g., 200 for chaos testing)
- `--credits=N`: Override Credits in all factions (e.g., 50000 for unit spam)

Example:
```bash
cd out/build/windows-x64-release/tests/headless
../../../tests/headless/stress_determinism.sh SCENF001.INI 500
```

Or with explicit runner:
```bash
/path/to/stress_determinism.sh SCENF001.INI 500 /usr/local/bin/headless_hash_runner
```

With stress parameters for chaos testing:
```bash
cd out/build/windows-x64-release/tests/headless
../../../tests/headless/stress_determinism.sh SCENF001.INI 500 "" --max-units=200 --credits=50000
```

### Run continuous stress test (loops until failure)
The script loops indefinitely, generating a new random seed each iteration via `openssl rand -hex 64`. Each iteration:
1. Generates a random 64-byte seed
2. Runs `headless_hash_runner <map> <ticks> <seed>` twice with that seed
3. Compares SHA-384 hashes
4. Exits on first mismatch or crash

Example output (passing):
```
Stress testing determinism
  Runner: ./headless_hash_runner
  Map: SCENF001.INI
  Ticks per run: 500

[1] seed=b84b356c6709390f... ✓ PASS (hash=a4f9d0d18f006fb7...)
[2] seed=30d6a62410caa00b... ✓ PASS (hash=a0c399bde7302902...)
...continues indefinitely until a failure is found...
```

## Dependencies

- `openssl` (for `rand -hex`)
- `bash` or any POSIX shell
- The compiled `headless_hash_runner` executable

## Platform Support

- Linux ✓
- macOS ✓
- Windows (WSL, Git Bash, MSYS2) ✓

The script is pure POSIX shell with no Windows-specific code.

## Usage Patterns

### Find fast desync
Run with a low tick count to catch issues quickly:
```bash
./stress_determinism.sh SCENF001.INI 10 ./headless_hash_runner
```

### Deep stress test with high unit counts
For maximum chaos, combine higher ticks with unit/credit overrides on large maps:
```bash
./stress_determinism.sh SCENF001.INI 5000 ./headless_hash_runner --max-units=150 --credits=100000
```

This creates scenarios where each faction can have 150+ concurrent units and structures, stressing unit-building interactions and AI pathfinding.

### Different maps
The game may behave differently depending on map layout:
```bash
./stress_determinism.sh SCENF002.INI 500 ./headless_hash_runner
./stress_determinism.sh SCENF003.INI 500 ./headless_hash_runner
```
