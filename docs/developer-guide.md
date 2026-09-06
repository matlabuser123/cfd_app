# CFDApp Developer Guide

## Architecture

`cfd_core` is the production library. The command-line application, benchmarks, tests, and optional Qt GUI link to it rather than duplicating CFD logic.

| Area | Responsibility |
| --- | --- |
| `core` | Mesh, fields, boundaries, and JSON case loading |
| `physics` | Momentum, pressure, and mass-flux discretization |
| `numerics` | Sparse matrices and linear solvers |
| `solver/simple` | Iterative SIMPLE coupling, cancellation, and iteration callbacks |
| `validation` | Numerical gates, determinism checks, refinement, and profile comparison |
| `parallel` | Runtime-controlled OpenMP loops and reductions |
| `gpu` | CUDA capability detection, field operations, CSR conversion, and SpMV boundaries |
| `io` | Deterministic CSV, JSON, and legacy ASCII VTK export/import |
| `gui` | Qt controller/worker boundary and presentation only |

The CPU solver remains the numerical reference. GUI code must call `CFDController`; it must not implement CFD mathematics. CUDA selection currently resolves to CPU SIMPLE because the full SIMPLE algorithm is not ported to CUDA.

## Build Configurations

The CMake presets are the supported local entry points:

```powershell
cmake --preset windows-debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure

cmake --preset windows-release
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
```

Both configurations build tests and benchmarks. Tests explicitly undefine `NDEBUG`, so `assert`-based checks execute in release-mode regression runs while production binaries remain optimized.

Optional Qt and CUDA builds require their toolchains to be installed and discoverable by CMake:

```powershell
cmake -S . -B build/gui -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCFDAPP_BUILD_GUI=ON
cmake -S . -B build/cuda -G Ninja -DCMAKE_BUILD_TYPE=Release -DCFDAPP_BUILD_CUDA=ON
```

## Test and Benchmark Gates

Run the complete suite with `ctest --test-dir build/debug --output-on-failure`. Use labels for focused verification:

```powershell
ctest --test-dir build/debug -L correctness --output-on-failure
ctest --test-dir build/debug -L parallel --output-on-failure
ctest --test-dir build/debug -L performance --output-on-failure
ctest --test-dir build/debug -L gpu --output-on-failure
```

The GPU SpMV benchmark returns CTest skip code `77` when CUDA hardware is unavailable. A skip is expected on CPU-only hosts; it is not GPU performance evidence.

The 80x80 benchmark runs five samples and gates the median runtime. Its `--thread-scaling` mode reports 1/2/4/8-thread results. Do not change thresholds or claim performance improvements without re-running the benchmark.

## Determinism and Concurrency

Result writers use the classic locale and binary output to stabilize numeric formatting and newline bytes. Keep output field ordering fixed and extend `test_io.cpp` whenever an output format changes.

OpenMP work must preserve the serial reference result. Parallel matrix assembly owns disjoint sparse-matrix rows; the aggregate nonzero counter is atomic. Continuity residuals evaluate per-cell fluxes in parallel but accumulate in deterministic cell order. Extend serial-versus-parallel equivalence tests for every new parallel loop.

`SIMPLE::solve` accepts an atomic cancellation flag and an iteration callback. The callback is forwarded through validation and the GUI worker. Cancellation must remain cooperative and must not mutate shared solver state from the UI thread.

## CUDA Boundaries

CUDA code is conditionally compiled only when `CFDAPP_BUILD_CUDA=ON`.

- `CUDAKernels` owns raw device kernels.
- `GPUFieldOperations` owns host/device field transfer for supported operations.
- `GPUMatrixAssembly` converts host sparse matrices to deterministic CSR.
- `GPULinearAlgebra` owns CSR SpMV transfer and execution.

Every GPU operation needs a CPU-reference comparison and a fallback test. The current machine may not have `nvcc`; in that case validate the CPU fallback paths and run CUDA builds on a CUDA-capable host before considering device code release-ready.

## Release Workflow

Create and validate a release artifact from the project root:

```powershell
.\scripts\package-release.ps1
.\scripts\clean-machine-validate.ps1
.\scripts\installation-test.ps1
```

`clean-machine-validate.ps1` expands the CPack ZIP into a unique temporary directory and validates the executable, case files, and docs. `installation-test.ps1` performs a separate fresh `cmake --install` into a temporary prefix. When script execution is restricted, use a process-local PowerShell bypass:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
```

Keep generated `build/`, package staging, temporary installations, and benchmark artifacts out of source changes. Validate CMake target source lists after adding `.cpp` files, because IDE automation can accidentally add a new file to `cfdapp` instead of its intended target.

## CI

`.github/workflows/windows-ci.yml` runs on every push and pull request. It configures, builds, and tests both debug and release presets; packages the release ZIP; validates an isolated ZIP extraction; validates a fresh CMake installation; and uploads the release ZIP as a workflow artifact. CUDA-specific benchmarks are expected to skip on runners without CUDA hardware.