# CFDApp 0.1.0 — Release Evidence Package

**Generated:** 2026-09-07
**Version under evaluation:** `CFDApp 0.1.0` (from `project(CFDApp VERSION 0.1.0 ...)` in `CMakeLists.txt`)
**Commit:** none — this repository has not been initialized under git yet (see Caveats). All
evidence below is tied to the working tree as it stood on the date above, not to a commit hash.

## Environment

| Component | Value |
|---|---|
| OS | Windows 11 Pro 10.0.26200 |
| CMake | 4.4.2 |
| Generator | Ninja 1.13.2 |
| Compiler | MinGW-W64 x86_64-ucrt-posix-seh, GCC 16.1.0 (WinLibs POSIX build) |
| OpenMP | enabled (`CFDAPP_HAS_OPENMP=1`) |
| CUDA | not built (`CFDAPP_BUILD_CUDA=OFF`); no CUDA-capable device present on this machine |

## Summary

| # | Check | Result | Evidence |
|---|---|---|---|
| 1 | Version string (debug + release) | `CFDApp 0.1.0`, both configs | [01_version.txt](01_version.txt) |
| 2 | Debug regression suite (CTest) | **20/20 passed**, 1 hardware-gated test skipped | [02_regression_debug.txt](02_regression_debug.txt) |
| 3 | Release regression suite (CTest) | **20/20 passed**, 1 hardware-gated test skipped | [03_regression_release.txt](03_regression_release.txt) |
| 4 | 20×20 lid-driven-cavity validation | smoke gate passed; finite solution; see note below | [04_validation_20x20/](04_validation_20x20/) |
| 5 | 40×40 benchmark | ran to completion, 1600 cells | [05_benchmark_40x40.txt](05_benchmark_40x40.txt) |
| 6 | 80×80 benchmark + thread scaling | converged, finite, threads 1/2/4/8 all measured | [06_benchmark_80x80.txt](06_benchmark_80x80.txt) |
| 7 | Release package build (CPack ZIP) | built successfully | [08_package_build.txt](08_package_build.txt) |
| 8 | Clean-machine validation (extracted ZIP) | passed | [09_clean_machine_validate.txt](09_clean_machine_validate.txt) |
| 9 | Fresh CMake install test | passed | [10_installation_test.txt](10_installation_test.txt) |
| 10 | Release smoke test (staged package + benchmark) | passed *(script bug fixed during this pass — see below)* | [11_release_smoke_test.txt](11_release_smoke_test.txt) |
| 11 | Package checksum | SHA256 recorded | [12_package_checksum.txt](12_package_checksum.txt) |
| 12 | Shipped artifact | included in this bundle | [CFDApp-0.1.0-Windows-x64.zip](CFDApp-0.1.0-Windows-x64.zip) (704 KB) |

**Overall: every check that can run without CUDA hardware passes.**

## Notes on what "passed" means here

- **Regression suite:** re-run twice (once to fix a toolchain issue, once for this package) with
  identical 20/20 results both times — see the "static-linked MinGW runtime" fix in `roadmap.md`'s
  🟢 Complete section and `TODO.md` item #2 for why 4 of these were failing before this pass.
- **20×20 validation:** `strict_converged: false`, `smoke_gate_passed: true`, `iterations: 1`.
  This is expected and documented (see `README.md`'s "Validation Evidence" section): the CLI's
  exit code reflects a strict 1e-8 residual check that a single-iteration smoke run does not meet,
  while the separate, looser smoke-gate check (continuity ≤ 100, momentum ≤ 10) does pass. This
  evidence demonstrates the solver runs, produces a finite result, and is deterministic — it is
  **not** a claim of full physical convergence. A converged, physically meaningful run would need
  the strict path exercised separately (not yet wired into the CLI — see `TODO.md` item #3).
- **Benchmarks:** each is a single local run (median of 5 samples internally, per the benchmark's
  own report). Timings are indicative for this machine only, not a cross-machine performance claim.

## Fixed during this pass

`scripts/release-smoke-test.ps1` was throwing `"Unexpected benchmark output"` on every run despite
the benchmark succeeding. Root cause: `& $benchmark 2>&1` returns a PowerShell array (one element
per output line); matching that array directly with `-notmatch` matches per-element and returns the
non-matching *lines* (e.g. the hotspot breakdown), which is a non-empty — therefore truthy — array
even though the benchmark's own summary line matched. Fixed by joining the array to a single string
before matching. This script is also not currently invoked by `.github/workflows/windows-ci.yml`,
so nothing had caught the bug — worth wiring it into CI (see Caveats).

## Caveats — read before treating this as a shippable release

1. **No git repository yet.** This evidence is not tied to a commit hash or tag, and
   `.github/workflows/windows-ci.yml` has never actually executed (no repo/remote to trigger it
   from). Treat this package as pre-release, single-machine evidence, not CI-verified evidence.
2. **`release-smoke-test.ps1` still isn't wired into `windows-ci.yml`** — it was run manually for
   this package. Consider adding it as a CI step now that it's fixed.
3. **CLI `--case` remains a stub** — only `--validate-20` and the benchmark binaries produce
   evidence today; there's no way yet to generate this kind of evidence for an arbitrary case.
4. **CUDA/GPU:** not exercised at all on this machine (no device, not built). The roadmap's "GPU ✅"
   covers kernel/field-op/matrix-assembly/linear-algebra infrastructure and CPU/GPU equivalence
   tests, not a full GPU-accelerated SIMPLE solve (GUI falls back to CPU for CUDA selection).
5. **Single machine, single run.** No multi-machine reproducibility check has been performed yet.

See `TODO.md` for the full prioritized list this evidence package feeds into (items #7 "Versioned
release" and #8 "Final release" are next, but items #1, #3, and #4 above are still open).
