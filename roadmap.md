# CFDApp — Roadmap
**C++20 · CMake · VS Code · Windows/WSL**

> Modular, testable, deterministic CFD solver with a CPU reference implementation and optional acceleration.

---

## 🟢 Complete

- Core C++20/CMake infrastructure
- Mesh and field systems
- Boundary conditions
- Case configuration/loading
- Finite-volume discretization
- Sparse matrix infrastructure
- CG / BiCGSTAB solvers
- Momentum and pressure equations
- Baseline SIMPLE coupling
- Unit/integration testing foundation
- CLI workflow
- Basic CSV/JSON/VTK export
- 20×20 stability validation
- 40×40 validation
- 80×80 benchmark/release gate
- Formal grid-refinement analysis
- Analytical/published validation
- 80×80 performance profiling
- Measured bottleneck optimization
- Row-major assembly optimization
- 80×80 performance regression benchmark
- CTest performance gate
- Complete debug regression suite
- Deterministic CSV/JSON/VTK result packages
- Complete CSV/JSON/VTK validation
- OpenMP field/flux operations
- OpenMP matrix assembly
- Deterministic parallel residual accumulation
- 1/2/4/8-thread scaling benchmark
- Parallel correctness verification
- Full clean-build verification
- CUDA backend architecture
- CUDA kernel infrastructure
- GPU field operations
- GPU matrix assembly
- GPU linear algebra
- CPU/GPU numerical equivalence
- GPU correctness tests
- GPU performance benchmark
- CPU/GPU scaling analysis
- Qt GUI architecture
- Qt case editor
- Mesh/case loading
- Solver controls
- Residual monitoring
- Live simulation status
- CPU/OpenMP/CUDA backend selection
- VTK result visualization
- Export controls
- Error/status reporting
- Release preset build with CLI version verification and release performance gates
- Release-mode regression suite with assertions enabled for test targets
- Clean-machine validation of isolated CPack ZIP extraction
- Portable CPack ZIP package with executable, cases, and documentation validation
- Isolated CMake installation test with executable, cases, and documentation validation
- User documentation for build, validation, cases, benchmarks, packaging, and optional GUI workflows
- Developer architecture, testing, concurrency, CUDA, and release workflow documentation
- Windows CI pipeline for debug/release regression, packaging, installation validation, and ZIP artifacts
- Static-linked MinGW runtime (`-static-libgcc -static-libstdc++ -static`) removing dependence on PATH-resolved `libstdc++-6.dll`/`libgcc_s_seh-1.dll`/`libwinpthread-1.dll` at load time
- Final regression suite: debug and release configurations both 20/20 passing (GPU SpMV benchmark skips cleanly without CUDA hardware), 2026-09-07
- Release evidence package: version, regression suite, 20×20/40×40/80×80 evidence, packaging/install/smoke validation, and checksum collected under `results/release/0.1.0/` (2026-09-07); fixed a latent `release-smoke-test.ps1` bug found in the process (PowerShell array vs. `-notmatch` semantics)
- GitHub Release `v0.1.0` published (2026-09-07): `https://github.com/matlabuser123/cfd_app/releases/tag/v0.1.0`, with the CI-built ZIP attached (Actions run `34039782169`, the run triggered by the `v0.1.0` tag push itself at commit `56c20c5` — not a local build)

---

## CUDA / GPU Acceleration

### v0.1.0 Release Scope — COMPLETE

CFDApp v0.1.0 includes CUDA infrastructure and validated GPU numerical primitives.

Completed:

- [x] CUDA project infrastructure.
- [x] CUDA device detection.
- [x] GPU field-operation kernels.
- [x] GPU CSR matrix conversion/assembly support.
- [x] GPU sparse matrix-vector multiplication (SpMV).
- [x] CPU/GPU numerical-equivalence tests.
- [x] CUDA test infrastructure with clean skipping when CUDA hardware is unavailable.
- [x] CUDA components integrated as acceleration layers beneath the CPU numerical stack.
- [x] Documented limitation that complete CUDA SIMPLE coupling is not part of v0.1.0.

### v0.1.0 CUDA Limitation

The complete SIMPLE solver remains CPU-based for v0.1.0.

Supported SIMPLE execution backends:

- Serial CPU.
- OpenMP CPU.

CUDA is **not currently a complete SIMPLE solver backend**. The existing CUDA implementation provides validated numerical primitives only, including field operations, CSR assembly/conversion, and SpMV.

The GUI may detect CUDA hardware, but CUDA selection does not execute the complete SIMPLE coupling on the GPU. When CUDA SIMPLE coupling is unavailable, the application explicitly falls back to the serial CPU implementation.

This is an intentional release boundary rather than an unresolved v0.1.0 release blocker — see `TODO.md`'s CUDA scope decision (item #4) and `README.md`'s "CUDA / GPU Scope in v0.1.0" for the full record.

### Post-v0.1.0 CUDA Development

- [ ] Port momentum-equation assembly to CUDA.
- [ ] Port pressure-correction assembly to CUDA.
- [ ] Implement GPU-compatible SIMPLE coupling.
- [ ] Implement GPU residual/convergence reductions.
- [ ] Minimize CPU↔GPU transfers within the SIMPLE iteration loop.
- [ ] Establish complete CPU/GPU SIMPLE numerical-equivalence tests.
- [ ] Add end-to-end GPU SIMPLE regression tests.
- [ ] Benchmark GPU SIMPLE against Serial and OpenMP CPU implementations.
- [ ] Add larger-mesh GPU scaling studies.
- [ ] Only advertise CUDA as a complete solver backend after end-to-end validation.

### Backend Policy

The backend architecture must distinguish between:

1. **Supported solver backends** — capable of executing the complete SIMPLE algorithm.
2. **Acceleration primitives** — GPU/parallel numerical operations that can be used underneath the solver but do not constitute a complete solver backend.

For v0.1.0, Serial CPU and OpenMP are supported SIMPLE solver backends. CUDA belongs to the acceleration-primitives category until complete GPU SIMPLE coupling is implemented and validated.

### Backend Completion Gate

CUDA must not be advertised as a complete SIMPLE solver backend until all of the following are satisfied:

- [ ] Complete SIMPLE iteration executes on CUDA.
- [ ] Momentum equations execute through the GPU path.
- [ ] Pressure correction executes through the GPU path.
- [ ] Residual/convergence evaluation executes correctly.
- [ ] CPU/GPU numerical equivalence is demonstrated.
- [ ] End-to-end regression tests pass.
- [ ] GPU benchmarks demonstrate measured performance.
- [ ] GUI backend selection genuinely executes the CUDA solver.

---

## 🟢 Release Engineering — COMPLETE

> **v0.1.0 shipped (2026-09-07):** repo initialized, tagged, and pushed to `origin`
> (`github.com/matlabuser123/cfd_app`); `windows-ci.yml` confirmed green on GitHub Actions;
> debug **and** release builds both pass **20/20 tests** (hardware-gated GPU benchmark skips
> cleanly without CUDA); release evidence package assembled under `results/release/0.1.0/`;
> `CHANGELOG.md` written; and the GitHub Release itself is published —
> `https://github.com/matlabuser123/cfd_app/releases/tag/v0.1.0`, with the CI-built ZIP attached
> (Actions run `34039782169` at commit `56c20c5`, matching the tag exactly). See `TODO.md` item #8.
>
> **v0.1.0 GPU scope (decided 2026-09-07):** CUDA backend selection (CLI and GUI) intentionally
> falls back to the CPU solver, because full SIMPLE coupling is not yet ported to GPU — see the
> "CUDA / GPU Acceleration" section above for the full decision record.

- `cfdapp --case <path>` CLI wiring (2026-09-07): general case-directory execution, reusing
  `CFDController::loadValidationCase`/`ValidationRunner` (the same path the GUI already
  exercises), emitting the same evidence artifacts `--validate-20` does under a case-specific
  path; covered by `CFDCLICaseTests`, which spawns the real built executable. See `TODO.md`
  item #3.

---

## 🔴 Current Focus

### Investigate SIMPLE's apparent numerical instability at real (non-degenerate) tolerances

> **High-severity finding, discovered 2026-09-07 while implementing `--case` (`TODO.md` item
> #3a) — read the full record there before starting.** Summary: every existing caller that
> drives `SIMPLE` through `ValidationCase`/`ValidationRunner` (`--validate-20`,
> `test_validation.cpp`, `GridRefinementAnalyzer.cpp`) uses a momentum/pressure tolerance
> (`10.0`–`50.0`) far looser than any case's own declared `numerics.json` (typically `1e-8`).
> Tracing why revealed the inner linear solver's convergence check trivially short-circuits at
> that looseness, performing **zero actual iterations** and leaving the velocity field frozen at
> its initial value — confirmed directly: residuals were bit-identical across all 1000 SIMPLE
> iterations of a lid-driven cavity solve, with center-cell velocity exactly `(0, 0)` throughout.
> Tightening the tolerance enough to force genuine iterative work instead causes the outer SIMPLE
> iteration to visibly **diverge** (continuity residual growing ~2 orders of magnitude in a single
> iteration) for the same case, mesh, and relaxation factors every existing test already uses.
>
> This calls into question whether any of the "🟢 Complete" validation entries above that run
> through this same pipeline (20×20/40×40/80×80 validation, grid-refinement analysis, performance
> benchmarks) reflect a genuinely converged, physically meaningful CPU solve, or the same
> degenerate zero-iteration artifact — **not confirmed either way for most of them**, only
> spot-checked for tolerance values on `GridRefinementAnalyzer.cpp`. This directly concerns design
> rule #1 below ("CPU solver = numerical reference") and should be resolved, or at least
> conclusively scoped, before further validation claims are trusted at face value or before item
> #9's CUDA/CPU equivalence work resumes (equivalence to an unvalidated reference proves nothing).

---

## 🟡 Next

### Post-Release

- Larger-mesh benchmarks
- Performance tuning
- Additional CFD validation cases
- Advanced turbulence/physics models
- Full CUDA SIMPLE coupling (GPU-accelerated end-to-end solve — see "CUDA / GPU Acceleration → Post-v0.1.0 CUDA Development" above for the detailed task breakdown)
- Additional visualization features

---

## Critical Path

```text
Numerical Validation
        ↓
Profiling
        ↓
Optimization
        ↓
Performance Regression
        ↓
Reliability
        ↓
CPU Parallelism
        ↓
CUDA / GPU
        ↓
Qt GUI
        ↓
Release ← CURRENT
```

## Engineering Order

```text
Correctness
    ↓
Validation
    ↓
Profiling
    ↓
Optimization
    ↓
Regression
    ↓
CPU Parallelism
    ↓
GPU
    ↓
GUI
    ↓
Release
```

## Design Rules

- **CPU solver = numerical reference**
- **GUI contains no CFD mathematics**
- **Optimize only from measured evidence**
- **Every major change requires tests**
- **Results must be deterministic and reproducible**
- **CPU/GPU implementations must produce equivalent results**
- **Performance claims require repeatable benchmark evidence**
- **GPU acceleration must not compromise CPU correctness**
- **GUI must remain decoupled from the CFD core**
- **CLI and GUI must use the same solver backend**
- **Release builds must pass the complete regression suite**
- **Release artifacts must be reproducible and installable**

### 🚀 Current position

```text
CPU CFD Solver       ✅†
Validation           ✅†
Performance          ✅
Reliability          ✅
OpenMP               ✅
CUDA                 ✅*
Qt GUI               ✅
Release              ✅
CLI Case Runner      ✅
SIMPLE @ real tol.    🔴 CURRENT
```

\* CUDA = validated acceleration primitives (field ops, CSR assembly, SpMV) only.
Full CUDA SIMPLE coupling is scoped post-release — see "CUDA / GPU Acceleration" above.

† Confirmed correct only in the loose-tolerance regime every existing caller uses (see Current
Focus above) — genuine convergence at a case's own declared tolerance is an open question, not
yet confirmed either way for most of the validation entries this checkmark covers.

**v0.1.0 is released. Current stage: investigating SIMPLE's apparent numerical instability at
real tolerances (`TODO.md` item #3a) — a higher-priority finding than the CUDA backlog below.**
