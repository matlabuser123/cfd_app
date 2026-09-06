param(
    [string]$BuildDirectory = (Join-Path (Split-Path -Parent $PSScriptRoot) 'build/release')
)

$ErrorActionPreference = 'Stop'

$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($null -eq $cmake) {
    $cmakePath = 'C:\Program Files\CMake\bin\cmake.exe'
    if (!(Test-Path $cmakePath -PathType Leaf)) { throw 'CMake executable not found' }
}
else {
    $cmakePath = $cmake.Source
}

if (!(Test-Path (Join-Path $BuildDirectory 'cmake_install.cmake') -PathType Leaf)) {
    throw "Release build installation script not found: $BuildDirectory"
}

$installRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("cfdapp-install-test-" + [System.Guid]::NewGuid().ToString())
& $cmakePath --install $BuildDirectory --prefix $installRoot
if ($LASTEXITCODE -ne 0) { throw 'CMake installation failed' }

$exe = Join-Path $installRoot 'bin/cfdapp.exe'
if (!(Test-Path $exe -PathType Leaf)) { throw "Installed executable not found: $exe" }

Push-Location $installRoot
try {
    $version = & $exe --version
    if ($LASTEXITCODE -ne 0) { throw 'Installed application failed --version' }
}
finally {
    Pop-Location
}
if ($version -notmatch '^CFDApp [0-9]+\.[0-9]+\.[0-9]+$') { throw "Unexpected version output: $version" }

foreach ($relativePath in @(
    'cases/cavity_20x20/case.json',
    'cases/cavity_20x20/mesh.json',
    'cases/cavity_20x20/physics.json',
    'cases/cavity_20x20/boundary_conditions.json',
    'cases/cavity_20x20/initial_conditions.json',
    'cases/cavity_20x20/numerics.json',
    'cases/cavity_20x20/output.json',
    'docs/README.md',
    'docs/developer-guide.md',
    'docs/roadmap.md'
)) {
    if (!(Test-Path (Join-Path $installRoot $relativePath) -PathType Leaf)) { throw "Installed file missing: $relativePath" }
}

Write-Output "Installation test passed: $version"
Write-Output "Installation root: $installRoot"