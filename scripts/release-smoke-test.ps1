param(
    [string]$PackageRoot = (Join-Path (Split-Path -Parent $PSScriptRoot) 'package/staging')
)

$ErrorActionPreference = 'Stop'

$exe = Join-Path $PackageRoot 'bin/cfdapp.exe'
if (!(Test-Path $exe)) { throw "Packaged executable not found: $exe" }

$version = & $exe --version
if ($LASTEXITCODE -ne 0) { throw "Packaged application failed --version" }
if ($version -notmatch '^CFDApp [0-9]+\.[0-9]+\.[0-9]+$') { throw "Unexpected version output: $version" }

$caseFiles = @(
    'cases/cavity_20x20/case.json',
    'cases/cavity_20x20/mesh.json',
    'cases/cavity_20x20/physics.json',
    'cases/cavity_20x20/boundary_conditions.json',
    'cases/cavity_20x20/initial_conditions.json',
    'cases/cavity_20x20/numerics.json',
    'cases/cavity_20x20/output.json'
)
foreach ($relativePath in $caseFiles) {
    if (!(Test-Path (Join-Path $PackageRoot $relativePath))) { throw "Packaged case file missing: $relativePath" }
}

$benchmark = Join-Path (Split-Path -Parent $PSScriptRoot) 'build/release/benchmarks/benchmark_80x80.exe'
if (!(Test-Path $benchmark)) { throw "Release benchmark not found: $benchmark" }
# & $benchmark 2>&1 returns one array element per output line. Matching the array directly
# with -notmatch matches element-by-element and returns the non-matching *lines* (e.g. the
# hotspot breakdown), which is a non-empty (truthy) array even when the benchmark line itself
# matched -- so join to a single string before matching.
$benchmarkOutput = & $benchmark 2>&1
if ($LASTEXITCODE -ne 0) { throw "80x80 benchmark failed: $benchmarkOutput" }
$benchmarkOutputText = $benchmarkOutput -join [Environment]::NewLine
if ($benchmarkOutputText -notmatch '80x80 benchmark:') { throw "Unexpected benchmark output: $benchmarkOutputText" }

Write-Output "Release smoke test passed: $version"
Write-Output $benchmarkOutputText
