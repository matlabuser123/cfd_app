# CFDApp — Todo List for GitHub Copilot

Generated from a project progress review on 2026-09-07. Ordered by priority — work top to bottom.
Each item has context and acceptance criteria so it can be picked up independently.

---

## P0 — Blockers for anything called "release"

### 1. Initialize version control and verify CI actually runs
**Context:** This directory has no `.git` — `.github/workflows/windows-ci.yml` exists but has
never executed. There is no commit history, no way to diff "what changed," and no place to
attach a tagged release.
**Do:**
- [ ] `git init`, add a sensible initial commit (respecting `.gitignore` — `build/`, `.vscode/*.log`, etc.)
- [ ] Push to a GitHub remote
- [ ] Confirm `Windows CI` workflow runs green on the initial push (configure → build → test for both debug and release, package, clean-machine validate, install test)
**Acceptance:** GitHub Actions shows a green run of `windows-ci.yml` on the repo's default branch.

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
- [ ] Implement `cfdapp --case <path>` to load a case directory (reusing the existing case-loading
      code already exercised by `CFDCaseTests` / the Qt `CaseEditor`) and run the SIMPLE solver to
      convergence or the configured iteration limit
- [ ] Emit the same evidence artifacts `--validate-20` does (report/JSON/residuals) under a
      case-specific results path
- [ ] Add a CLI-level test (or extend `tests/test_case.cpp`) covering a full `--case` run against
      `cases/cavity_20x20`
**Acceptance:** `cfdapp --case cases/cavity_20x20` runs to completion and writes result artifacts,
with a passing test covering it.

### 4. Decide and document GPU (CUDA) solve scope for this release
**Context:** CUDA kernels, field ops, matrix assembly, and linear algebra are implemented and
equivalence-tested against CPU, but SIMPLE coupling itself has not been ported to GPU — per
`README.md`, GUI CUDA backend selection "falls back to the CPU solver until CUDA SIMPLE coupling
is available." `roadmap.md` marks GPU as fully ✅, which overstates the current state.
**Do:** pick one:
- [ ] **Option A (scope it in):** Port SIMPLE coupling to the GPU backend so CUDA selection in the
      GUI performs a real GPU-accelerated solve, with CPU/GPU numerical equivalence tests extended
      to cover the full solve loop, not just individual kernels
- [ ] **Option B (scope it out):** Update `roadmap.md` and `README.md` to accurately state that
      v0.1.0 ships CPU/OpenMP solving with GPU-accelerated linear-algebra primitives only, and that
      full GPU SIMPLE coupling is a post-release item
**Acceptance:** `roadmap.md`, `README.md`, and the GUI's backend-selection UI/behavior all agree
on what CUDA selection actually does in this release.

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

### 7. Versioned release
- [ ] Bump `project(CFDApp VERSION ...)` in `CMakeLists.txt` from the placeholder `0.1.0` if appropriate
- [ ] Tag the release in git, attach the CPack ZIP produced by `scripts/package-release.ps1`
- [ ] Confirm `cfdapp.exe --version` output matches the tag

### 8. Final release
- [ ] Cut the GitHub release from the tag, attach the CI-built artifact from the `Windows CI` workflow
- [ ] Update `roadmap.md` to move "Release Engineering" into 🟢 Complete and define the next
      post-release milestone from the roadmap's existing "🟡 Next" list (performance tuning,
      larger-mesh benchmarks, additional validation cases, turbulence models, expanded GPU coverage,
      more visualization features)

---

## Notes for whoever (human or Copilot) picks this up
- Design rules already in force (`roadmap.md`) — keep following them: CPU solver is the numerical
  reference, GUI has no CFD math, optimize only from measured evidence, every major change needs
  tests, results must be deterministic/reproducible, CPU/GPU must stay numerically equivalent.
- Don't trust `roadmap.md`'s "🟢 Complete" section at face value for GPU and test-suite health
  until items #2 and #4 above are resolved — this list was generated specifically because those
  two claims didn't hold up under a live check.
