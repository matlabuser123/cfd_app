$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
$build = Join-Path $root 'build/release'
$staging = Join-Path $root 'package/staging'

cmake --preset windows-release
cmake --build --preset release

if (Test-Path $staging) { Remove-Item -Recurse -Force $staging }
cmake --install $build --prefix $staging

$exe = Join-Path $staging 'bin/cfdapp.exe'
if (!(Test-Path $exe)) { throw "Release executable not found: $exe" }

$version = & $exe --version
if ($LASTEXITCODE -ne 0) { throw "Release version smoke test failed" }
Write-Output $version

Push-Location $build
try {
    cpack --config CPackConfig.cmake -G ZIP
}
finally {
    Pop-Location
}

Write-Output "Release package created under $build"
