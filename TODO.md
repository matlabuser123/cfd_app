# CFDApp — Todo List for GitHub Copilot

Generated from a project progress review on 2026-09-07. Ordered by priority — work top to bottom.
Each item has context and acceptance criteria so it can be picked up independently.

**Agreed sequencing (2026-09-07):**

1. ✅ GPU scope documentation — this file, `roadmap.md`, `README.md` (item #4)
2. ✅ Resolve the recurring CRLF churn, uncommitted (item #1's note)
3. ✅ Verify `windows-ci.yml` runs green on GitHub (item #1) — confirmed via GitHub's public
   Actions API: run `34039534422` on `main` @ `5e9e7ba` (current HEAD) succeeded, 2026-09-07
4. ✅ Cut the GitHub Release `v0.1.0`, attach the ZIP (item #8) — published 2026-09-07
5. Wire up `cfdapp --case <path>` (item #3) — **deliberately not started yet**, first post-release task
6. Add `.gitattributes` to prevent future CRLF churn (item #1's note) — deferred until after the release is cut, to avoid touching repo config pre-release
7. Post-release CFD backlog: full CUDA SIMPLE coupling (item #9) plus the rest of `roadmap.md`'s "Post-Release" list

---

## P0 — Blockers for anything called "release"

### 1. ✅ DONE (2026-09-07, remote push + green CI both confirmed) — Initialize version control and verify CI actually runs
**Context:** This directory had no `.git` — `.github/workflows/windows-ci.yml` existed but had
never executed. There was no commit history, no way to diff "what changed," and no place to
attach a tagged release.
- [x] `git init -b main` (local only), repo-local identity set (`user.name`/`user.email`)
- [x] `.gitignore` extended to exclude `package/staging/` (build output) and
      `results/release/*/*.zip` (binary release archives — better attached to a GitHub Release
      than kept in git history; the evidence text/json/csv alongside them is still tracked)
- [x] Initial commit made (213 files) and tagged `v0.1.0`
- [x] Pushed to a GitHub remote: `origin` → `https://github.com/matlabuser123/cfd_app.git`.
      Confirmed `main` is up to date with `origin/main`, and `git ls-remote --tags origin` shows
      `v0.1.0` (and its dereferenced commit) present on the remote (2026-09-07).
- [x] Confirmed `Windows CI` workflow runs green on GitHub — queried GitHub's public Actions API
      directly (`api.github.com/repos/matlabuser123/cfd_app/actions/workflows/windows-ci.yml/runs`,
      no `gh` CLI or auth needed for a public repo): run `34039534422` on branch `main` at commit
      `5e9e7ba` (current HEAD) — `status: completed`, `conclusion: success`. A second run on the
      `v0.1.0` tag ref (commit `56c20c5`) also succeeded. (2026-09-07)
**Acceptance:** GitHub Actions shows a green run of `windows-ci.yml` on the repo's default branch.
— **met.**

**Also noted in passing (2026-09-07, recurred twice):** a transient whole-tree staged diff showed
up locally on two separate occasions (~190 files, then 209 files), caused by `core.autocrlf`
differing between the Windows git and WSL git sessions used against this same (OneDrive-synced)
working tree — confirmed both times via `git diff --cached --ignore-cr-at-eol --stat` to be 100%
line-ending churn with zero real content changes (the only files with genuine staged content on
the second occurrence were this file and `roadmap.md`, from the push-status and GPU-scope doc
updates). Both times it was resolved by discarding the CRLF-only changes (`git restore --staged
--worktree` back to `HEAD`) **without committing anything**, per the agreed sequencing above —
so no bad commit has resulted either time. There's still no `.gitattributes`, so this can recur a
third time; adding one (e.g. `* text=auto eol=lf` + `git add --renormalize .`) is deliberately
deferred to step 6 of the sequencing above, after the `v0.1.0` GitHub Release is cut, so a repo-config
change doesn't get mixed into the release-cutting commits.

### 2. ✅ DONE (2026-09-07) — Fix the 4 currently-failing local tests and pin the toolchain
**Context:** `roadmap.md` claimed "Verified baseline: 20 passing tests," but running
`ctest --test-dir build/debug` showed **4 failures out of 21**:
`CFDCaseTests`, `CFDGUIControllerTests`, `CFDProfilingTests`, `CFDIOTests` — all exited with
`0xC0000139` (`STATUS_ENTRYPOINT_NOT_FOUND`) and zero output, i.e. crashed before `main()` ran.
**Root cause (confirmed via `objdump -p` + `where libstdc++-6.dll`):** `C:\Program Files\Git\mingw64\bin\libstdc++-6.dll`
resolved earlier on `PATH` than the WinLibs GCC toolchain (`BrechtSanders.WinLibs.POSIX.UCRT...\mingw64\bin`)
that actually compiled these binaries. Git's bundled libstdc++ was ABI-mismatched and lacked symbols
the four filesystem-heavy test binaries needed (case loading, GUI controller, profiling report writes,
CSV/JSON/VTK I/O) — other test binaries happened not to exercise those symbols, so they passed.
**Fix applied:** added `add_link_options(-static-libgcc -static-libstdc++ -static)` for non-MSVC
builds in `CMakeLists.txt`, so built executables no longer depend on any runtime DLL resolved from
`PATH` — this removes the whole class of bug (also relevant to end users installing the packaged
ZIP on a machine with no MinGW runtime on `PATH` at all), not just this one PATH collision.
- [x] Reconfigured + rebuilt both `windows-debug` and `windows-release` presets
- [x] Re-ran `ctest --test-dir build/debug --output-on-failure` → **20/20 passed**, GPU SpMV benchmark skipped cleanly
- [x] Re-ran `ctest --test-dir build/release --output-on-failure` → **20/20 passed**, GPU SpMV benchmark skipped cleanly
- [x] Updated `roadmap.md`'s "Verified baseline" line
- [ ] Still open: document the required toolchain / PATH pitfall in `README.md` / `docs/developer-guide.md`
      so contributors understand why the static-link flags are there
**Acceptance:** Fresh `cmake --preset windows-debug && cmake --build --preset debug && ctest --test-dir build/debug`
on a clean shell passes 20/20 non-hardware-gated tests, reproducibly. — **met.**

---

## P1 — Real functionality gaps hidden by the roadmap's "✅"

### 3. Wire up `--case` in the CLI
**Context:** `apps/cfdapp/main.cpp` currently returns `"Case execution is not wired into the CLI yet."`
for `--case`. The only runnable end-to-end path today is the hardcoded `--validate-20`. General
case execution (load any `cases/<name>/` directory and run it) doesn't exist from the command line.
**Do:**
- [ ] Implement general case-directory loading through the CLI (reusing the existing case-loading
      code already exercised by `CFDCaseTests` / the Qt `CaseEditor`) — remove the current
      hardcoded-only `--validate-20` limitation and allow arbitrary supported case directories
- [ ] Run the existing SIMPLE solver through `cfd_core` to convergence or the configured iteration
      limit, with configurable convergence settings read from the case's `numerics.json`
- [ ] Produce deterministic solver output and emit the same evidence artifacts `--validate-20` does
      (report/JSON/residuals), using the same result/evidence formats as the validation workflow,
      under a case-specific results path
- [ ] Add a CLI regression test (or extend `tests/test_case.cpp`) covering a full end-to-end
      `--case` run against `cases/cavity_20x20`
**Acceptance:** `cfdapp --case cases/cavity_20x20` runs to completion and writes result artifacts,
with a passing test covering it.

### 4. ✅ DONE (2026-09-07) — Decide and document GPU (CUDA) solve scope for this release
**Context:** CUDA kernels, field ops, matrix assembly, and linear algebra are implemented and
equivalence-tested against CPU, but SIMPLE coupling itself has not been ported to GPU — per
`README.md`, GUI CUDA backend selection "falls back to the CPU solver until CUDA SIMPLE coupling
is available." `roadmap.md` marked GPU as fully ✅ with no caveat, which overstated the current state.
**Decision: Option B (scope it out)** — v0.1.0 ships CPU/OpenMP SIMPLE solving; CUDA support is
limited to validated acceleration primitives (field operations, CSR assembly, SpMV). Full CUDA
SIMPLE coupling is post-v0.1.0 work. Chosen because everything else is already release-tagged and
pushed, and the GUI/CLI code already implements this behavior (`CFDController::configureBackend`
falls back to CPU for any CUDA request) — Option B just makes the docs match the code, rather than
requiring new solver work to match an aspirational doc.
- [x] `README.md`: added a "GPU (CUDA) Scope in v0.1.0" section; updated the intro line and the
      GUI paragraph to point to it
- [x] `roadmap.md`: added a "v0.1.0 GPU scope" note under Current Focus; annotated the "🚀 Current
      position" table's `CUDA ✅` line with a footnote; retitled the vague "Expanded GPU solver
      coverage" Post-Release bullet to "Full CUDA SIMPLE coupling"
- [x] `TODO.md` (this file): closed out this item
**Acceptance:** `roadmap.md`, `README.md`, and the GUI's backend-selection UI/behavior all agree
on what CUDA selection actually does in this release. — **met.**

**Backend policy going forward:** distinguish **supported solver backends** (capable of executing
the complete SIMPLE algorithm — currently Serial CPU and OpenMP CPU) from **acceleration
primitives** (GPU/parallel numerical operations usable underneath the solver but not a complete
solver backend on their own — currently everything CUDA provides). CUDA moves from the second
category to the first only when item #9 below is complete.

---

## P2 — Then, the roadmap's own remaining steps (now meaningful)

### 5. ✅ DONE (2026-09-07) — Final regression suite
- [x] Full debug + release CTest run, clean, on the fixed toolchain (depended on #2) → both **20/20 passed**
- [x] Confirmed hardware-gated GPU test skips cleanly without CUDA
- [ ] Still open: run/confirm on a CUDA-equipped machine if one becomes available

### 6. ✅ DONE (2026-09-07) — Release evidence package
**Assembled under `results/release/0.1.0/`** (see its `EVIDENCE_MANIFEST.md` for the full index
and caveats): version output, debug + release regression suite logs (20/20 both), 20×20 validation
artifacts, 40×40/80×80 benchmark output, package build log, clean-machine validation, fresh
CMake install test, release smoke test, package SHA256 checksum, and the shipped ZIP itself.
**Bonus finding fixed in the process:** `scripts/release-smoke-test.ps1` was throwing on every
run despite the benchmark succeeding — `& $benchmark 2>&1` returns a PowerShell array (one
element per line), and `-notmatch` against an array matches per-element, returning the
non-matching lines (the hotspot breakdown) as a non-empty/truthy array. Fixed by joining to a
single string before matching.
- [x] Regression suite + validation + benchmark output collected
- [x] Packaging/install/smoke-test scripts run and their output captured
- [x] Fixed `release-smoke-test.ps1`'s array-matching bug
- [x] Checksum of the shipped ZIP recorded
- [ ] Still open: `release-smoke-test.ps1` is still not wired into `windows-ci.yml` — it was run
      manually for this package; add it as a CI step (small, do alongside #1)
- [ ] Still open: this evidence isn't tied to a commit/tag yet (depends on #1) — re-generate once
      the repo exists so the package can reference a real commit hash

### 7. ✅ DONE locally (2026-09-07) — Versioned release
- [x] Kept `project(CFDApp VERSION 0.1.0 ...)` — reasonable as the version for an initial
      semver-0.x release; no reason found to bump it
- [x] Wrote `CHANGELOG.md` (Added/Fixed/Known Limitations for v0.1.0)
- [x] Tagged the release in git: annotated tag `v0.1.0` on commit `56c20c5`
- [x] Confirmed `cfdapp.exe --version` output (`CFDApp 0.1.0`) matches the tag — see
      `results/release/0.1.0/01_version.txt`
- [ ] Still open: the CPack ZIP itself is deliberately *not* committed to git (see #1's
      `.gitignore` note) — attach `build/release/CFDApp-0.1.0-Windows-x64.zip` as a **GitHub
      Release** asset once #1's remote exists, rather than tagging-and-forgetting it locally

### 8. ✅ DONE (2026-09-07) — Final release
**Published:** `https://github.com/matlabuser123/cfd_app/releases/tag/v0.1.0` — title "CFDApp
v0.1.0", on the existing `v0.1.0` tag (commit `56c20c5`). Authenticated as `matlabuser123`
(confirmed `ADMIN` permission on the repo before publishing; a first login attempt authenticated
as the wrong account, `matlabuser3` — read-only — and was logged out before retrying).
- [x] Cut the GitHub release from the tag, attach the CI-built artifact from the `Windows CI`
      workflow — used the artifact from Actions run
      [34039782169](https://github.com/matlabuser123/cfd_app/actions/runs/34039782169) (the run
      triggered by the `v0.1.0` tag push itself, commit `56c20c5` — an exact match, not just "a
      recent green run"), not the local build; verified via `gh release view --json assets` that
      the published asset's digest (`sha256:bf0a7357fec41bb9d03f7d74898e6a572231e0099dfec5b5e4f83d42a9690fee`,
      685,344 bytes) matches the downloaded CI artifact exactly. Release notes drawn from
      `CHANGELOG.md` (Added/Fixed/Known Limitations), plus the asset provenance/checksum above.
- [x] Updated `roadmap.md`: moved Release Engineering into 🟢 Complete with the release URL; set
      Current Focus to item #3 (`--case` CLI), the next task per this file's agreed sequencing
      (item 5) rather than the generic Post-Release list, since that sequencing is more specific;
      updated the "🚀 Current position" table and closing status line accordingly
- [x] Corrected two stale lines in `CHANGELOG.md`'s Known Limitations that predated the repo/CI
      existing ("no remote repository configured yet", "CI has not yet executed against a hosted
      runner") — both now read correctly against verified state

---

## P3 — Post-v0.1.0 CUDA Development

### 9. GPU SIMPLE Coupling, Validation, and Performance

**Status:** 🟡 Started (2026-09-07) — solver-backend seam in place; no CUDA kernel work yet (see below)

**Dependency:** #8 must be completed before this task's actual CUDA kernel work begins — that
constraint stands. The seam introduced below is pure CPU C++ (no CUDA), added because it was
already identified as the prerequisite groundwork; started early only because it doesn't touch or
risk any shipped v0.1.0 behavior. Do not read this as license to skip ahead on the rest of #9.

**Context:** This is the "Option A" CUDA work that item #4 deliberately scoped out of v0.1.0.
Full task breakdown is maintained in `roadmap.md` under **CUDA / GPU Acceleration → Post-v0.1.0
CUDA Development** rather than being duplicated here.

**Architecture note (verified against the current source, not just the docs):** there was no
existing Serial/OpenMP/CUDA solver-backend seam to plug a CUDA implementation into — `SIMPLE` was
one concrete class, `SolverFactory` only selected the inner linear solver (CG vs. BiCGSTAB), and
`CFDController::configureBackend` just flipped the OpenMP toggle. **This seam is now built** (see
Progress below); a future CUDA implementation plugs into it rather than needing to invent it too.

#### Progress (2026-09-07) — solver-backend seam, CPU-only, no CUDA kernels

- [x] `src/solver/simple/SimpleBackend.hpp` — abstract interface (`solve`/`setBoundaryConditions`/
      `setProfiler`) any compute backend implements.
- [x] `src/solver/simple/CpuSimpleBackend.{hpp,cpp}` — thin adapter wrapping the existing `SIMPLE`
      class unchanged; used for both Serial and OpenMP (they differ only in `ParallelRuntime`
      configuration, not in solver code, as already established).
- [x] `src/solver/simple/SimpleBackendFactory.{hpp,cpp}` — builds a backend from a requested
      `ComputeBackend`. Serial/OpenMP → `CpuSimpleBackend`. CUDA → **honestly falls back to
      `CpuSimpleBackend`** (there is still no `CudaSimpleBackend`), but the returned
      `selection.status` now distinguishes "CUDA hardware available, but CUDA SIMPLE coupling
      isn't implemented yet" from "no CUDA hardware available" — the existing `CFDController`
      status string conflates the two.
- [x] `tests/test_solver_backend.cpp` (new `CFDSolverBackendTests`, labeled
      `parallel;gpu;correctness`) proves: Serial-via-factory is bit-identical (< 1e-12) to calling
      `SIMPLE` directly; OpenMP-via-factory matches the serial reference; a CUDA request returns
      `active == Serial` (never claims CUDA ran) and still executes a fully correct, convergent CPU
      solve — not a stub — with `setProfiler` correctly forwarded.
- [x] Full regression suite re-run on both presets after this change: **debug 21/21 passed, release
      21/21 passed** (GPU SpMV benchmark skips cleanly, as before) — zero regressions.
- [ ] **Not done, deliberately:** no CUDA kernel code (momentum assembly, pressure-correction
      assembly, GPU-resident SIMPLE loop, residual reductions) was written. This environment has no
      CUDA toolkit (`nvcc` not found) — writing GPU numerics that can't be compiled or checked
      against the CPU reference here would violate this project's own design rules (CPU/GPU
      equivalence required, every major change needs tests) and the Backend Completion Gate below.
      That work requires a CUDA-capable machine.
- [ ] **Not done, deliberately:** existing production call sites (`ValidationRunner`,
      `CFDController`, the CLI) were **not** rewired to go through `SimpleBackendFactory` — they
      still construct `SIMPLE` directly, unchanged, so nothing shipped in v0.1.0 is touched by this.
      Wiring them through the factory is a reasonable small next step, kept separate from actual
      CUDA kernel work so it can be reviewed on its own.

#### Implementation

- [ ] Port momentum-equation assembly to CUDA.
- [ ] Port pressure-correction assembly to CUDA.
- [ ] Implement GPU-compatible SIMPLE coupling.
- [ ] Implement GPU residual/convergence reductions.
- [ ] Minimize CPU↔GPU transfers inside the SIMPLE iteration loop.
- [ ] Ensure the GPU path can execute a complete SIMPLE iteration without falling back to the CPU solver.

#### Numerical Validation

- [ ] Establish complete CPU/GPU numerical-equivalence tests.
- [ ] Compare velocity fields.
- [ ] Compare pressure fields.
- [ ] Compare mass-flux balance.
- [ ] Compare residual histories.
- [ ] Establish deterministic GPU regression tolerances.
- [ ] Add end-to-end GPU SIMPLE regression coverage.

#### Performance

- [ ] Benchmark GPU SIMPLE against Serial CPU.
- [ ] Benchmark GPU SIMPLE against OpenMP CPU.
- [ ] Measure host↔device transfer overhead.
- [ ] Measure CUDA kernel execution costs.
- [ ] Measure GPU SpMV performance.
- [ ] Measure GPU matrix-assembly performance.
- [ ] Benchmark larger meshes.
- [ ] Establish GPU scaling limits.

### Backend Completion Gate

**Do not advertise CUDA as a complete SIMPLE solver backend until every gate below is satisfied:**

- [ ] Complete SIMPLE iteration executes on CUDA.
- [ ] Momentum-equation path executes on CUDA.
- [ ] Pressure-correction path executes on CUDA.
- [ ] Residual/convergence evaluation executes correctly on CUDA.
- [ ] CPU/GPU numerical equivalence is demonstrated end-to-end.
- [ ] GPU SIMPLE regression tests pass.
- [ ] GPU benchmarks demonstrate measured performance.
- [ ] GUI CUDA backend selection genuinely executes the CUDA solver.
- [ ] CUDA requests no longer silently fall back to the CPU SIMPLE solver.

### Acceptance Criteria

Task #9 is **COMPLETE** only when:

1. The complete SIMPLE iteration executes through the CUDA backend.
2. Momentum and pressure-correction paths are GPU-enabled.
3. CPU/GPU results satisfy documented numerical tolerances.
4. End-to-end GPU regression tests pass.
5. GPU performance is measured against Serial and OpenMP implementations.
6. Host↔device transfer costs and major GPU kernel costs are documented.
7. The GUI genuinely executes the CUDA solver when CUDA is selected.
8. `roadmap.md` and `README.md` are updated to declare CUDA a supported SIMPLE solver backend
   alongside Serial and OpenMP.

Until all acceptance criteria are satisfied, CUDA remains classified as **GPU acceleration
primitives only**, not a complete SIMPLE solver backend.

### Dependency chain

```text
#4 — v0.1.0 CUDA scope decision
      ↓
#8 — prerequisite / post-release gate
      ↓
#9 — full CUDA SIMPLE implementation
      ↓
Backend Completion Gate
      ↓
CUDA becomes a supported SIMPLE backend
```

Do not start #9 now. Finish #8 first, then #9 becomes the dedicated CUDA development task.

---

## Notes for whoever (human or Copilot) picks this up
- Design rules already in force (`roadmap.md`) — keep following them: CPU solver is the numerical
  reference, GUI has no CFD math, optimize only from measured evidence, every major change needs
  tests, results must be deterministic/reproducible, CPU/GPU must stay numerically equivalent.
- Don't trust `roadmap.md`'s "🟢 Complete" section at face value for GPU and test-suite health
  until items #2 and #4 above are resolved — this list was generated specifically because those
  two claims didn't hold up under a live check.
