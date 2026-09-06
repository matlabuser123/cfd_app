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

---

## 🔴 Current Focus

> **Verified baseline (2026-09-07):** Debug **and** release builds both pass **20/20 tests**; the hardware-gated GPU benchmark is skipped cleanly without CUDA. Release evidence package assembled under `results/release/0.1.0/` (see its `EVIDENCE_MANIFEST.md` for caveats — notably no git repo/tag yet, so this is pre-release single-machine evidence). Numerical validation, performance, reliability, CPU parallelism, GPU infrastructure, and application features are complete.

### Release Engineering

- Final release

> Versioned release complete: repo initialized locally, initial commit tagged `v0.1.0`
> (`CHANGELOG.md` has the full v0.1.0 notes). Not yet pushed to a remote — see `TODO.md` item #1.

---

## 🟡 Next

### Post-Release

- Performance tuning
- Larger-mesh benchmarks
- Additional CFD validation cases
- Advanced turbulence/physics models
- Expanded GPU solver coverage
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
CPU CFD Solver       ✅
Validation           ✅
Performance          ✅
Reliability          ✅
OpenMP               ✅
CUDA                 ✅
Qt GUI               ✅
Release              🔴 CURRENT
```

**You have reached the final engineering stage: Release Engineering.**