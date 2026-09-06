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
* [x] ~~Fix the identified pressure-correction/velocity-correction relaxation
      inconsistency~~ — **attempted and empirically falsified, all three candidates**, via a
      probe harness reproducing SIMPLE's exact iteration sequence at a moderately tight
      inner tolerance (`1e-3`, 30 iterations): (1) current behavior (raw `aP`) — diverges to
      `max|u| > 1e13` by iteration 2; (2) `aP` inflated by `1/velocityRelaxation` in
      `PressureAssembler`+`correctVelocity` (the Patankar-consistency theory above) —
      diverges *worse* (`1.46e13`); (3) relaxing the velocity correction itself by
      `velocityRelaxation` before applying — iteration 1's peak is proportionally smaller
      (as expected, ×0.7) but **still** diverges to `6.9e12` by iteration 2. All three fail
      in the same qualitative way: not a marginal instability more damping would fix, but
      exponential blow-up regardless of damping. Also notable: the blow-up is *worse*, not
      better, at a tighter linear-solver tolerance (`max|u|=7415` after iteration 1 at
      `1e-3` vs. `1183` at `1.0` from the earlier, looser-tolerance run) — backwards from
      what "solver under-convergence noise" would predict, and consistent instead with a
      genuine discretization/coupling defect that a more *accurate* linear solve exposes
      more starkly, not a relaxation-tuning nuance. **Conclusion: the relaxation-consistency
      theory above is very likely not the (or not the only) root cause.** No source files
      were changed for this attempt (probe-only). Re-scoping: this needs a systematic,
      term-by-term re-derivation of `PressureAssembler::coefficients`/`assemble` against a
      textbook collocated-grid SIMPLE reference (this codebase stores u/v/p at cell centers,
      using central-difference face gradients — e.g. `MomentumAssembler`'s
      `-(pressure[east]-pressure[west])/(2dx)` source term and `correctVelocity`'s matching
      `(correction[east]-correction[west])/(2dx)` — rather than a classic staggered grid),
      or a minimal from-scratch reproduction with a known analytical solution to isolate
      which specific coefficient is wrong, rather than another guess-and-check attempt.

### #3a — Leading root-cause hypothesis (2026-09-07): missing Rhie-Chow interpolation

Continued the systematic re-derivation above. Findings, most important first:

* **Confirmed this is not a convection-scheme issue.** Re-ran the same iteration sequence
  with viscosity raised to `100` (Re ≈ 0.01, effectively Stokes flow, negligible convective
  nonlinearity) — **still diverges**, to `max|u| > 1e6` by iteration 2. A zero-lid-velocity
  case (no forcing at all) correctly stays at exactly zero, ruling out a spontaneous /
  boundary-condition-only bug. So the defect is in the momentum-pressure *coupling* itself,
  not the upwind convection discretization.
* **Confirmed iteration 1's blow-up is entirely independent of the momentum equation's
  pressure-gradient source term.** Tried scaling that source term by the cell volume
  (`* dy` for the x-equation) to fix a suspected units mismatch — iteration 1's peak
  `max|u|` was *completely unchanged* (as expected in hindsight: pressure is uniformly
  zero at iteration 1, so that source term is exactly zero regardless of its scaling), and
  iteration 2+ diverged *faster* (`1.08e73` by iteration 3 vs. the original's `1e13`-ish by
  iteration 2). Reverted immediately (see `git diff` — clean).
* **`PressureEquation::correctVelocity`'s formula is self-consistent with
  `MomentumAssembler`'s own source term** — both use the identical
  `(neighbor_east - neighbor_west) / (2·dx)` central-difference convention, so perturbing
  the discretized momentum equation as actually coded by a pressure correction `p'`
  algebraically reproduces `correctVelocity`'s exact formula. This pairing is not
  internally inconsistent with itself.
* **Leading hypothesis: missing Rhie-Chow (or equivalent) momentum interpolation — a
  textbook collocated-grid pressure-velocity decoupling ("checkerboard") defect.** Because
  the velocity correction at cell P depends on pressure at cells *two grid points away*
  (`p'_east`, `p'_west`, i.e. neighbors-of-the-face-neighbors, not the immediate face
  values), while `PressureAssembler::assemble` builds a standard *narrow*, immediate-
  neighbor 5-point Laplacian stencil for `p'` itself, the two are derived from different
  effective stencils. This exact mismatch is the classical, textbook-documented cause of
  spurious odd/even ("checkerboard") pressure-velocity oscillations on a collocated grid
  (where velocity and pressure are stored at the same points) — normally suppressed via
  Rhie-Chow interpolation (face velocities reconstructed from momentum-equation
  coefficients rather than simple neighbor averaging) or avoided entirely by using a
  staggered grid. This hypothesis is consistent with *every* observation so far: why it's
  not a convection issue (checkerboard modes exist even in pure Stokes flow, confirmed
  above); why it's rapid/exponential rather than marginal instability (checkerboard is an
  unstable null-space-like mode, not a damping problem — consistent with all three
  relaxation-based fixes failing identically); and why a *more accurate* linear solve makes
  it *worse*, not better (a tighter solve resolves the true unstable mode more precisely
  instead of it being partially masked by solver truncation error).
* **Not yet implemented or empirically confirmed as the fix** — this is the leading,
  best-supported hypothesis from the evidence gathered, not a proven-and-fixed conclusion.
  Implementing Rhie-Chow interpolation correctly (reconstructing face velocities from
  momentum coefficients in `MassFluxCalculator`/`PressureAssembler`, replacing the current
  simple neighbor-averaged face velocities) is a substantial, structural algorithm change
  touching multiple physics classes, not a quick patch — it needs its own careful
  derivation, implementation, and full validation against the regression suite and (once
  real convergence is achievable) the published Ghia et al. cavity benchmark data already
  referenced by `PublishedValidation.cpp`, and should be scoped and executed as a dedicated
  task rather than attempted live at the tail of this investigation.
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
