#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Stress test for determinism: loop until failure, generating a new random seed each iteration.

.DESCRIPTION
    This script repeatedly runs two instances of headless_hash_runner with identical
    random seeds and verifies they produce identical SHA-384 hashes. Useful for detecting
    sporadic non-determinism bugs that might only appear with specific game states.

.PARAMETER MapFile
    Map filename (e.g., "SCENF001.INI"). Defaults to "SCENF001.INI".

.PARAMETER Ticks
    Number of simulation ticks per run. Defaults to 500.

.PARAMETER RunnerPath
    Full path to headless_hash_runner executable. If omitted, searches in:
    - Current directory
    - ../../../out/build/windows-x64-release/tests/headless/headless_hash_runner.exe
    - ../../../out/build/linux-release/tests/headless/headless_hash_runner

.EXAMPLE
    .\stress_determinism.ps1
    # Runs with defaults: SCENF001.INI, 500 ticks, searches for runner

.EXAMPLE
    .\stress_determinism.ps1 -MapFile SCENF002.INI -Ticks 1000 -RunnerPath C:\path\to\headless_hash_runner.exe
#>

param(
    [string]$MapFile = "SCENF001.INI",
    [uint32]$Ticks = 500,
    [string]$RunnerPath = ""
)

# Find the runner executable if not specified
if (-not $RunnerPath) {
    $candidates = @(
        "headless_hash_runner.exe",
        "headless_hash_runner",
        "../../out/build/windows-x64-release/tests/headless/headless_hash_runner.exe",
        "../../out/build/linux-release/tests/headless/headless_hash_runner"
    )
    
    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            $RunnerPath = Resolve-Path $candidate
            Write-Host "Found runner: $RunnerPath"
            break
        }
    }
}

if (-not $RunnerPath -or -not (Test-Path $RunnerPath)) {
    Write-Error "Could not find headless_hash_runner. Specify -RunnerPath explicitly."
    exit 1
}

Write-Host "Stress testing determinism"
Write-Host "  Runner: $RunnerPath"
Write-Host "  Map: $MapFile"
Write-Host "  Ticks per run: $Ticks"
Write-Host ""

$iteration = 0
$passed = 0

try {
    while ($true) {
        $iteration++
        
        # Generate a random 64-byte seed via openssl and convert to hex
        $randOutput = & openssl rand -hex 64
        if ($LASTEXITCODE -ne 0) {
            Write-Error "openssl rand failed"
            exit 1
        }
        
        $seedHex = $randOutput.Trim()
        
        # Verify seed is 128 hex characters (64 bytes)
        if ($seedHex.Length -ne 128) {
            Write-Error "Invalid seed length: $($seedHex.Length) (expected 128)"
            exit 1
        }
        
        Write-Host -NoNewline "[$iteration] seed=$($seedHex.Substring(0, 16))... "
        
        # Run both instances
        $hash1 = & $RunnerPath $MapFile $Ticks $seedHex 2>$null
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Run 1 failed (exit code $LASTEXITCODE)"
            exit 1
        }
        
        $hash2 = & $RunnerPath $MapFile $Ticks $seedHex 2>$null
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Run 2 failed (exit code $LASTEXITCODE)"
            exit 1
        }
        
        # Compare hashes
        if ($hash1 -eq $hash2) {
            Write-Host "✓ PASS (hash=$($hash1.Substring(0, 16))...)"
            $passed++
        } else {
            Write-Host "✗ FAIL"
            Write-Host "  Run 1: $hash1"
            Write-Host "  Run 2: $hash2"
            Write-Host ""
            Write-Host "Stopped after $iteration iterations ($passed passed, 1 failed)"
            exit 1
        }
    }
} catch {
    Write-Error $_.Exception.Message
    exit 1
}
