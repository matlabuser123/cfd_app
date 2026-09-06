# CFDApp — TODO

**C++20 · CMake · Qt · OpenMP · CUDA**

Current priority is **SIMPLE numerical correctness**. Do not optimize or expand CUDA until the CPU reference solver is validated.

---

## 🔴 P0 — Current Blocker

### #3a — Fix SIMPLE Convergence

**Problem:** Existing validation callers use very loose inner solver tolerances, causing CG/BiCGSTAB to sometimes perform **zero iterations**. When tighter tolerances force real linear-solver work, the 20×20 cavity currently shows **SIMPLE divergence**.

### Tasks

* [x] Inspect `CGSolver.cpp` and `BiCGSTABSolver.cpp` tolerance semantics — both target
      `max(absoluteTolerance, relativeTolerance * initialResidual)`, exactly as documented;
      not a bug in these classes themselves.
* [x] Fix relative-tolerance handling consistently — `SIMPLE.cpp` was constructing both
      solvers with `relativeTolerance` set equal to `absoluteTolerance` (momentumTolerance/
      pressureTolerance), which inflates the *effective* target to `tolerance *
      initialResidual` whenever `initialResidual > 1`. Now passes `relativeTolerance = 0`
      unconditionally, so momentumTolerance/pressureTolerance behave as pure absolute
      tolerances everywhere, matching the outer convergence check. Confirmed via full
      regression suite (debug + release, 22/22) that no existing caller's results change —
      every one already uses an absolute tolerance looser than this problem's natural
      initial residual, so this was a latent bug, not yet an active one for any shipped
      behavior. Added `test_simple.cpp`'s "frozen field" characterization test to pin the
      current (still degenerate at loose tolerances) behavior.
* [x] Confirm linear solvers perform real iterations when required — confirmed they do, once
      both (a) the tolerance is tight enough relative to the natural per-iteration residual,
      and (b) the relative-tolerance inflation above is fixed. Neither alone is sufficient:
      at the existing loose absolute tolerance (10.0/20.0), the fix above does not by itself
      unfreeze the field, since 10.0/20.0 already exceeds the natural residual regardless of
      inflation.
* [x] Inspect SIMPLE momentum/pressure coupling — root cause of the divergence **precisely
      localized** (not yet fixed): instrumented a step-by-step reproduction of SIMPLE's
      first outer iteration (`u`-solve+relax → `v`-solve+relax → pressure solve → pressure
      correction) with an inner tolerance of `1.0`. Every step through the pressure *solve*
      produces sane values (`max|u| = 1` after both momentum solves; pressure correction
      field itself is unremarkable, min/max `≈ ∓9.3`). The blow-up happens in exactly one
      step: applying that correction — `max|u|` jumps from `1` to `1183` solely from
      `PressureEquation::correctVelocity`'s `velocity -= dpdx / momentumDiagonalU[cell]`.
      `momentumDiagonalU` (the raw, *unrelaxed* momentum-equation diagonal) has a minimum of
      `0.04` in this problem (at low-convection interior cells) — dividing a
      not-unreasonable pressure-correction gradient by a diagonal that small amplifies it by
      >20×.
* [x] Inspect pressure-correction signs and boundary conditions — signs and boundary
      handling in `PressureAssembler`/`MomentumAssembler` were reviewed and appear correct
      (dimensionally consistent, standard upwind/central-difference FVM discretization,
      matching Patankar-style formulas); the likely defect is not a sign error but a
      **missing relaxation consistency**: `PressureAssembler::coefficients` and
      `PressureEquation::correctVelocity` both use the *raw* momentum diagonal
      (`momentumDiagonalU`/`V`, captured straight from momentum assembly), whereas the
      classic SIMPLE derivation (Patankar) uses the diagonal *as inflated by the momentum
      under-relaxation factor* (`a_P / α_u`) in exactly these two places, specifically to
      keep the pressure-correction step damped consistently with the (implicitly or
      explicitly) relaxed momentum predictor. This codebase relaxes the momentum predictor
      *explicitly* (post-solve: `u = α·u_new + (1-α)·u_old`) rather than *implicitly*
      (inflating the equation's own diagonal before solving, Patankar's classic approach) --
      plausible that the pressure-correction/velocity-correction formulas were written
      assuming one relaxation style while the momentum solve uses the other, leaving the
      correction step with no damping at all. **Not fixed:** the correct resolution (inflate
      `momentumDiagonalU`/`V` by `1/velocityRelaxation` before use in
      `PressureAssembler`/`correctVelocity`, relax the correction step itself, or something
      else entirely) needs a real CFD-algorithm judgment call and careful re-derivation, not
      a guess -- explicitly left for deliberate follow-up rather than a speculative fix that
      "looks stable" without being verified correct.
* [ ] Fix the identified pressure-correction/velocity-correction relaxation inconsistency
      (see above) once the correct formula is confirmed (reference-derivation or
      cross-check against a known-correct SIMPLE implementation recommended before coding).
* [ ] Inspect mass-flux/continuity calculation (not yet reached -- blocked on the above).
* [ ] Test `cases/cavity_20x20` at realistic tolerances (blocked on the above fix).
* [x] Add linear-solver regression tests — n/a: root cause was in `SIMPLE.cpp`'s calling
      convention, not `BiCGSTABSolver`/`CGSolver` themselves, which already had adequate
      coverage in `test_linear_solver.cpp`.
* [x] Add SIMPLE convergence regression test — added a characterization test in
      `test_simple.cpp` pinning the current (still-frozen-at-loose-tolerance) behavior; a
      real convergence regression test needs the fix above first, since there is currently
      no tolerance setting that produces both genuine iterative work and a stable result.
* [x] Re-run full regression suite — debug + release, 22/22 non-hardware-gated, after each
      change in this item (relative-tolerance fix, new characterization test).
* [ ] Re-run 20×20, 40×40 and 80×80 validation after the fix (blocked on the fix above).
* [ ] Determine whether previous validation/benchmark evidence was affected by the
      zero-iteration behaviour (blocked on the fix above; near-certain yes for anything
      using the 10.0-50.0-style loose tolerances, given the confirmed frozen-field behavior
      at those exact values for cavity_20x20).

**Important:** Do not weaken tolerances simply to make the solver converge.

---

## 🟡 P1 — After SIMPLE Is Validated

### Benchmarking & Profiling

* [ ] Re-run trusted 20×20 / 40×40 / 80×80 benchmarks.
* [ ] Add larger meshes: 160×160, 320×320.
* [ ] Measure runtime, iterations, residuals and memory.
* [ ] Profile matrix assembly, SpMV, linear solves and SIMPLE overhead.
* [ ] Optimize only measured bottlenecks.
* [ ] Add performance regression evidence.

### Validation

* [ ] Re-run grid-refinement analysis.
* [ ] Re-run analytical/published-case validation.
* [ ] Add additional CFD validation cases.

---

## 🟡 P2 — Repository / Release Cleanup

* [ ] Add `.gitattributes` to prevent CRLF churn.
* [ ] Document the MinGW/PATH runtime issue in developer documentation.
* [ ] Wire `release-smoke-test.ps1` into Windows CI.
* [ ] Verify CUDA tests on a CUDA-capable machine when available.

---

## 🟡 P3 — Full CUDA SIMPLE

**Only after CPU SIMPLE is validated.**

* [ ] Wire production solver paths through `SimpleBackendFactory`.
* [ ] Implement CUDA momentum assembly.
* [ ] Implement CUDA pressure correction.
* [ ] Implement GPU SIMPLE iteration loop.
* [ ] Implement GPU residual/convergence reductions.
* [ ] Minimize CPU↔GPU transfers.
* [ ] Add end-to-end CPU/GPU equivalence tests.
* [ ] Add CUDA SIMPLE regression tests.
* [ ] Benchmark CUDA vs Serial/OpenMP.
* [ ] Remove CPU fallback for genuine CUDA SIMPLE.

### CUDA Completion Gate

CUDA becomes a **supported SIMPLE backend** only when:

* Complete SIMPLE runs on GPU.
* Momentum and pressure paths run on GPU.
* Residual/convergence runs on GPU.
* CPU/GPU results are equivalent within documented tolerances.
* End-to-end tests pass.
* GPU performance is measured.
* GUI CUDA selection genuinely executes CUDA.

Until then:

**CUDA = acceleration primitives, not a complete SIMPLE solver backend.**

---

## 🟢 Completed

* v0.1.0 GitHub Release
* Windows CI
* Debug/Release regression
* CLI `--case`
* Qt GUI
* OpenMP acceleration
* CUDA primitives
* Release packaging/install validation
* CPU/GPU primitive equivalence
* Solver backend abstraction/seam

---

## 🚀 Execution Order

```text
#3a SIMPLE correctness
        ↓
Validation
        ↓
Benchmarks
        ↓
Profiling
        ↓
Optimization
        ↓
Additional CFD validation
        ↓
Full CUDA SIMPLE
        ↓
Future releases
```

**Current task: #3a — make the CPU SIMPLE solver genuinely converge at realistic tolerances.**
