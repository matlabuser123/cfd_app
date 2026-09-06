# Changelog

All notable changes to CFDApp are documented in this file. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Added

- `cfdapp --case <path>` now loads and runs any conforming case directory (not just the hardcoded
  `--validate-20`), emitting the same evidence artifacts under `results/case/<case-name>/`; covered
  by `CFDCLICaseTests`. See `TODO.md` item #3.
- `SIMPLESettings`/`ValidationCase` gained additive `innerMomentumTolerance`/
  `innerPressureTolerance` fields for decoupling the inner per-iteration linear-solve gate from the
  outer convergence tolerance (default-unset, zero behavior change for existing callers).

### Known Limitations

- **High severity, needs dedicated investigation:** every existing `SIMPLE`-driving code path
  checked so far (`--validate-20`, `test_validation.cpp`, `GridRefinementAnalyzer.cpp`) uses a
  momentum/pressure tolerance far looser than any case's own declared `numerics.json` — loose
  enough that the inner linear solver performs **zero actual iterations**, leaving the velocity
  field frozen at its initial value for the entire run (confirmed: bit-identical residuals across
  1000 SIMPLE iterations, center-cell velocity exactly `(0, 0)` throughout). Tightening the
  tolerance enough to force real iterative work instead causes the outer SIMPLE iteration to
  visibly diverge. `cfdapp --case` inherits the same loose operational tolerance for now (matching
  `--validate-20`'s precedent) but evaluates and reports true pass/fail against a case's actual
  declared tolerance, so it honestly reports non-convergence rather than masking it. See `TODO.md`
  item #3a for the full record — this calls into question whether the v0.1.0 validation entries
  below that share this pipeline reflect genuine convergence.

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
- This release was validated on a single development machine; regression, validation, and
  benchmark evidence in `results/release/0.1.0/` was captured locally rather than on the hosted
  CI runner, though the CI pipeline itself has run green on GitHub Actions against this commit.

[0.1.0]: https://github.com/matlabuser123/cfd_app/releases/tag/v0.1.0
