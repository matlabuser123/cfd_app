# CFDApp Roadmap

**C++20 · CMake · VS Code · Windows/WSL**

Modular, deterministic CFD solver with a CPU reference implementation and optional GPU acceleration.

---

## 🟢 Complete

* C++20/CMake project infrastructure
* Mesh, fields and boundary conditions
* Case configuration/loading
* Finite-volume discretization
* Sparse matrices
* CG / BiCGSTAB solvers
* Momentum and pressure equations
* SIMPLE coupling
* CLI and GUI workflows
* CSV/JSON/VTK export
* Unit/integration testing
* OpenMP acceleration
* CUDA acceleration primitives
* CPU/GPU primitive equivalence tests
* Qt GUI
* Release packaging and installation validation
* Windows CI
* Debug and Release regression: **20/20 passing**
* Deterministic validation/evidence packages
* **v0.1.0 GitHub Release published**

> **CUDA scope:** v0.1.0 provides validated GPU primitives only. Full CUDA SIMPLE is not yet implemented.

---

## 🔴 Current Focus

### SIMPLE Numerical Convergence

Investigate SIMPLE at realistic numerical tolerances.

**Goals:**

* Verify CG/BiCGSTAB tolerance semantics.
* Ensure inner solvers perform genuine iterations.
* Test the **20×20 cavity** at realistic tolerances.
* Determine whether SIMPLE converges or diverges.
* Identify and fix the numerical root cause.
* Add regression tests for the discovered behaviour.
* Re-run the complete regression suite.

**Do not benchmark larger meshes or optimize performance until this is resolved.**

---

## 🟡 Next

After SIMPLE convergence is confirmed:

1. Larger-mesh benchmarks
2. Profiling and measured performance analysis
3. Performance optimization
4. Additional CFD validation cases
5. Performance regression gates
6. Additional physics/models
7. Full CUDA SIMPLE coupling
8. GPU end-to-end validation and benchmarking
9. Additional GUI/visualization improvements

---

## 🚧 Full CUDA SIMPLE

Future CUDA work:

* GPU momentum assembly
* GPU pressure-correction assembly
* GPU SIMPLE coupling
* GPU residual/convergence reductions
* Minimize CPU↔GPU transfers
* CPU/GPU SIMPLE numerical equivalence
* End-to-end CUDA regression tests
* GPU performance/scaling benchmarks
* Remove CPU fallback only after validation

---

## Engineering Order

```text
SIMPLE Correctness
       ↓
Numerical Validation
       ↓
Benchmarking
       ↓
Profiling
       ↓
Optimization
       ↓
Regression
       ↓
Additional Physics
       ↓
Full CUDA SIMPLE
       ↓
Further GUI / Visualization
       ↓
Future Releases
```

---

## Design Rules

* CPU solver is the numerical reference.
* Correctness before performance.
* Optimize only from measured evidence.
* Every major change requires tests.
* Results must be deterministic and reproducible.
* CPU/GPU results must be numerically equivalent.
* GUI contains no CFD mathematics.
* CLI and GUI use the same solver backend.
* GPU acceleration must not compromise CPU correctness.
* Performance claims require repeatable evidence.
* Release builds must pass regression tests.

---

## 🚀 Current Position

```text
v0.1.0 Release          ✅
CLI Case Runner         ✅
SIMPLE Implementation   ✅
SIMPLE Real Convergence 🔴 CURRENT
Larger Benchmarks       ⏸️
Performance Tuning      ⏸️
Additional Validation   ⏸️
Full CUDA SIMPLE        ⏸️
```

**Current priority: #3a — prove and fix genuine SIMPLE convergence at realistic tolerances.**
