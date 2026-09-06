param(
    [string]$PackagePath
)

$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'build/release'

if ([string]::IsNullOrWhiteSpace($PackagePath)) {
    Push-Location $build
    try {
        cpack --config CPackConfig.cmake -G ZIP
    }
    finally {
        Pop-Location
    }
    $PackagePath = Join-Path $build 'CFDApp-0.1.0-Windows-x64.zip'
}

if (!(Test-Path $PackagePath -PathType Leaf)) { throw "Release package not found: $PackagePath" }

$validationRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("cfdapp-clean-machine-" + [System.Guid]::NewGuid().ToString())
Expand-Archive -Path $PackagePath -DestinationPath $validationRoot -Force

$executables = @(Get-ChildItem -Path $validationRoot -Recurse -Filter 'cfdapp.exe' -File)
if ($executables.Count -ne 1) { throw "Expected one packaged executable, found $($executables.Count)" }
$packageRoot = Split-Path -Parent (Split-Path -Parent $executables[0].FullName)

$version = & $executables[0].FullName --version
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
    if (!(Test-Path (Join-Path $packageRoot $relativePath) -PathType Leaf)) { throw "Packaged case file missing: $relativePath" }
}

foreach ($relativePath in @('docs/README.md', 'docs/developer-guide.md', 'docs/roadmap.md')) {
    if (!(Test-Path (Join-Path $packageRoot $relativePath) -PathType Leaf)) { throw "Packaged documentation file missing: $relativePath" }
}

Write-Output "Clean-machine validation passed: $version"
Write-Output "Extracted package root: $packageRoot"