# Changelog

All notable changes to CFDApp are documented in this file. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [0.1.0] — 2026-09-07

Initial versioned release. First release engineered end-to-end: CPU reference solver through
validation, performance, OpenMP/CUDA acceleration infrastructure, an optional Qt GUI, and a
packaged, installable release artifact.

### Added

- C++20/CMake project infrastructure with Ninja, debug and release presets
- Mesh, field, and boundary-condition core with case (JSON) configuration/loading
- Finite-volume discretization, sparse matrix infrastructure, CG/BiCGSTAB linear solvers
- Momentum and pressure equations with SIMPLE coupling
- Numerical validation: 20×20 stability, 40×40, 80×80 benchmark/release gate, formal
  grid-refinement analysis, analytical/published-case comparison
- Performance profiling, measured bottleneck optimization, row-major assembly optimization,
  80×80 performance regression benchmark, CTest performance gate
- OpenMP-accelerated field/flux operations and matrix assembly with deterministic parallel
  residual accumulation; 1/2/4/8-thread scaling benchmark; parallel correctness verification
- CUDA backend infrastructure: kernels, field operations, matrix assembly, and linear algebra,
  with CPU/GPU numerical equivalence tests and a GPU performance benchmark (hardware-gated —
  skips cleanly without a CUDA device; full GPU-accelerated SIMPLE coupling is not yet ported,
  see Known Limitations)
- Optional Qt 6 GUI: case editor, mesh/case loading, solver controls, residual monitoring, live
  status, CPU/OpenMP/CUDA backend selection, VTK result visualization, CSV/JSON/VTK export
- CSV/JSON/VTK result export and validation
- CPack ZIP packaging, clean-machine extraction validation, and isolated CMake installation test
- Windows CI pipeline (`.github/workflows/windows-ci.yml`) covering debug/release build, test,
  packaging, and installation validation
- User and developer documentation (build, validation, cases, benchmarks, packaging, GUI,
  architecture, testing, concurrency, CUDA, release workflow)

### Fixed

- `scripts/release-smoke-test.ps1` threw `"Unexpected benchmark output"` on every run despite the
  benchmark succeeding, because `& $benchmark 2>&1` returns a PowerShell array (one element per
  output line) and `-notmatch` against an array matches per-element — the multi-line hotspot
  breakdown doesn't contain the literal match text, making the check truthy even on success.
  Fixed by joining the array to a single string before matching.
- 4 of 21 local tests (`CFDCaseTests`, `CFDGUIControllerTests`, `CFDProfilingTests`, `CFDIOTests`)
  could fail to launch (`STATUS_ENTRYPOINT_NOT_FOUND`) when an unrelated MinGW toolchain earlier
  on `PATH` (e.g. Git for Windows' bundled `mingw64\bin`) shadowed the runtime DLLs
  (`libstdc++-6.dll`, `libgcc_s_seh-1.dll`, `libwinpthread-1.dll`) with an ABI-incompatible build.
  Fixed by statically linking the MinGW C/C++ runtime (`-static-libgcc -static-libstdc++ -static`)
  for non-MSVC builds, removing the runtime DLL dependency entirely — this also protects installs
  of the packaged ZIP on machines without this exact MinGW runtime present.

### Known Limitations

- The CLI's `--case` flag is not yet wired up (`"Case execution is not wired into the CLI yet."`);
  the only end-to-end CLI run path today is the built-in `--validate-20` smoke validation.
- CUDA kernel/field-op/matrix-assembly/linear-algebra primitives are implemented and tested for
  CPU/GPU equivalence, but SIMPLE coupling itself has not been ported to GPU — the GUI's CUDA
  backend selection currently falls back to the CPU solver.
- This release was validated on a single development machine; the CI pipeline referenced above
  has not yet executed against a hosted runner (see repository history — this is the initial
  commit bringing the project under version control).

<!-- No remote repository is configured yet, so there is no release URL to link here. Once one
     exists, add: [0.1.0]: <remote-url>/releases/tag/v0.1.0 -->
