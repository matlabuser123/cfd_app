# CFDApp

CFDApp is a C++20 finite-volume CFD reference application. It provides a deterministic CPU SIMPLE solver, validation and benchmark gates, optional OpenMP acceleration, and an optional Qt interface when Qt 6 is available.

## Requirements

- CMake 3.25 or later
- Ninja
- A C++20 compiler
- Windows PowerShell on Windows

Optional capabilities:

- OpenMP, detected automatically by CMake
- Qt 6 Widgets, required only when `CFDAPP_BUILD_GUI=ON`
- CUDA Toolkit and a supported device, required only when `CFDAPP_BUILD_CUDA=ON`

## Build and Run

Build the debug configuration from the project root:

```powershell
cmake --preset windows-debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure
```

Run the command-line application:

```powershell
.\build\debug\cfdapp.exe
.\build\debug\cfdapp.exe --version
```

The current version command prints `CFDApp 0.1.0`.

## Validation Evidence

Run the built-in 20x20 cavity validation:

```powershell
.\build\debug\cfdapp.exe --validate-20
```

The command writes validation evidence under `results/validation/lid_driven_cavity/20x20`:

- `validation_report.txt` contains human-readable solver and residual output.
- `validation.json` contains machine-readable metadata.
- `residuals.csv` contains the recorded final residual data.

The command uses a strict residual check for its exit code and separately reports whether the broader smoke gate passed. Review both fields in the output before treating a run as numerical evidence.

## Example Case

The included lid-driven cavity case is in `cases/cavity_20x20`. A case directory contains:

- `case.json` for the case manifest
- `mesh.json` for mesh dimensions and geometry
- `physics.json` for fluid properties
- `boundary_conditions.json` for boundary specifications
- `initial_conditions.json` for initial fields
- `numerics.json` for solver settings
- `output.json` for output preferences

The Qt case editor can load this directory when the optional GUI is built. The CLI `--case` command is reserved for future direct case execution and currently returns an explanatory error.

## Benchmarks

The 80x80 benchmark verifies solver convergence and finite results:

```powershell
.\build\debug\benchmarks\benchmark_80x80.exe
```

Run its thread scaling report:

```powershell
.\build\debug\benchmarks\benchmark_80x80.exe --thread-scaling
```

CTest labels provide focused checks:

```powershell
ctest --test-dir build/debug -L performance --output-on-failure
ctest --test-dir build/debug -L parallel --output-on-failure
ctest --test-dir build/debug -L gpu --output-on-failure
```

GPU benchmarks skip cleanly when the CUDA toolkit or a compatible device is unavailable.

## Release Package

Build the optimized release configuration and run its complete regression suite:

```powershell
cmake --preset windows-release
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
```

Create the portable ZIP:

```powershell
.\scripts\package-release.ps1
```

The artifact is `build/release/CFDApp-0.1.0-Windows-x64.zip`. It contains `bin/cfdapp.exe`, the example case, and this documentation. Validate an extracted package or a fresh CMake installation with:

```powershell
.\scripts\clean-machine-validate.ps1
.\scripts\installation-test.ps1
```

If local PowerShell policy blocks scripts, run the command in a process configured with `Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass`.

## Optional Qt GUI

Configure the GUI only on a machine with Qt 6 Widgets installed and discoverable by CMake:

```powershell
cmake -S . -B build/gui -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCFDAPP_BUILD_GUI=ON
cmake --build build/gui --target cfd_gui
.\build\gui\cfd_gui.exe
```

The GUI provides case editing/loading, CPU/OpenMP/CUDA backend selection, cancellable validation, residual monitoring, VTK heatmaps, and selectable CSV/JSON/VTK exports. CUDA selection falls back to the CPU solver until CUDA SIMPLE coupling is available.# CFDApp

