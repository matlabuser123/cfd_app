#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string_view>

#include "core/mesh/Mesh.hpp"
#include "gui/CFDController.hpp"
#include "validation/ValidationRunner.hpp"

namespace
{

// Shared by --validate-20 and --case: both run a ValidationCase through the SIMPLE
// solver and must emit the same evidence artifact set (report/JSON/residuals), just
// under different result paths and with different pass/fail semantics for
// "strict_converged" (see call sites).
void writeValidationEvidence(
    const std::filesystem::path& evidenceDirectory,
    const cfd::ValidationCase& validationCase,
    const cfd::ValidationResult& result,
    double runtimeMilliseconds,
    bool strictConverged
)
{
    std::filesystem::create_directories(evidenceDirectory);
    const double reynolds = validationCase.viscosity > 0.0
        ? validationCase.density * validationCase.lidVelocity * validationCase.length / validationCase.viscosity
        : 0.0;

    {
        std::ofstream report(evidenceDirectory / "validation_report.txt");
        report << std::setprecision(17)
            << "case: " << result.caseName << '\n'
            << "mesh: " << result.nx << "x" << result.ny << '\n'
            << "rho: " << validationCase.density << '\n'
            << "mu: " << validationCase.viscosity << '\n'
            << "reynolds: " << reynolds << '\n'
            << "strict_converged: " << (strictConverged ? "true" : "false") << '\n'
            << "smoke_gate_passed: " << (result.passed ? "true" : "false") << '\n'
            << "iterations: " << result.iterations << '\n'
            << "continuity_residual: " << result.continuityResidual << '\n'
            << "u_residual: " << result.uResidual << '\n'
            << "v_residual: " << result.vResidual << '\n'
            << "mass_imbalance: " << result.massImbalance << '\n'
            << "runtime_milliseconds: " << runtimeMilliseconds << '\n'
            << "finite_solution: " << (result.finiteSolution ? "true" : "false") << '\n';
    }

    {
        std::ofstream metadata(evidenceDirectory / "validation.json");
        metadata << std::setprecision(17)
            << "{\n"
            << "  \"case\": \"" << result.caseName << "\",\n"
            << "  \"mesh\": {\"nx\": " << result.nx << ", \"ny\": " << result.ny << "},\n"
            << "  \"reynolds_number\": " << reynolds << ",\n"
            << "  \"converged\": " << (strictConverged ? "true" : "false") << ",\n"
            << "  \"smoke_gate_passed\": " << (result.passed ? "true" : "false") << ",\n"
            << "  \"iterations\": " << result.iterations << ",\n"
            << "  \"continuity_residual\": " << result.continuityResidual << ",\n"
            << "  \"u_residual\": " << result.uResidual << ",\n"
            << "  \"v_residual\": " << result.vResidual << ",\n"
            << "  \"mass_imbalance\": " << result.massImbalance << ",\n"
            << "  \"runtime_seconds\": " << runtimeMilliseconds / 1000.0 << ",\n"
            << "  \"finite_solution\": " << (result.finiteSolution ? "true" : "false") << '\n'
            << "}\n";
    }

    {
        std::ofstream residuals(evidenceDirectory / "residuals.csv");
        residuals << "iteration,continuity,u_momentum,v_momentum,pressure\n";
        residuals << result.iterations << ',' << result.continuityResidual << ','
            << result.uResidual << ',' << result.vResidual << ",0\n";
    }
}

}

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string_view(argv[1]) == "--version")
    {
        std::cout << "CFDApp " << CFDAPP_VERSION << '\n';
        return 0;
    }

    if (argc > 1 && std::string_view(argv[1]) == "--case")
    {
        if (argc < 3)
        {
            std::cerr << "Usage: cfdapp --case <path-to-case-directory>\n";
            return 2;
        }

        const std::filesystem::path caseDirectory = argv[2];
        const cfd::CFDController controller;
        cfd::ValidationCase validationCase;
        try
        {
            validationCase = controller.loadValidationCase(caseDirectory);
        }
        catch (const std::exception& error)
        {
            std::cerr << "Failed to load case '" << caseDirectory.string() << "': " << error.what() << '\n';
            return 2;
        }

        // SIMPLE::solve requires the inner momentum/pressure linear solve to already be
        // within momentumTolerance/pressureTolerance on every single outer iteration,
        // hard-throwing otherwise (see SIMPLE.cpp) — it is not just a final convergence
        // target. No existing caller in this codebase (--validate-20, the GUI,
        // test_validation.cpp) has ever fed SIMPLE a case's literal numerics.json
        // tolerance (typically 1e-8) for that per-iteration gate: every one of them uses
        // 10.0/20.0 there, the same values used below.
        //
        // These are not just "loose enough to avoid the hard-fail exception": tightening
        // them (tried down to 1e-6 while investigating this item) either still throws, as
        // the inner CG/BiCGSTAB solver (falling back to Gauss-Seidel) isn't guaranteed to
        // hit a tight tolerance within its iteration budget on every outer iteration, or
        // once the inner solve is doing real per-iteration work at all, the outer
        // iteration visibly diverges (continuity residual growing by ~2 orders of
        // magnitude in a single iteration) rather than converging — a pre-existing SIMPLE
        // stability characteristic this codebase has apparently never exercised past a
        // trivial first iteration before (see TODO.md item #3's notes; flagged there as a
        // separate follow-up, not something this CLI-wiring change attempts to fix).
        // 10.0/20.0 are the one setting empirically confirmed to run to completion.
        //
        // innerMomentumTolerance/innerPressureTolerance loosen only this per-iteration
        // gate; the case's real declared momentumTolerance/pressureTolerance/
        // continuityTolerance are left untouched and still drive the actual outer
        // convergence check (and result.passed below), so "did this case converge" is
        // judged against numerics.json as declared -- which is why this honestly reports
        // "no" for cavity_20x20 today rather than a manufactured "yes".
        validationCase.innerMomentumTolerance = 10.0;
        validationCase.innerPressureTolerance = 20.0;

        const auto start = std::chrono::steady_clock::now();
        const cfd::ValidationResult result = controller.runValidation(validationCase);
        const auto end = std::chrono::steady_clock::now();
        const double runtimeMilliseconds = std::chrono::duration<double, std::milli>(end - start).count();

        const std::filesystem::path evidenceDirectory = std::filesystem::path("results") / "case" / validationCase.name;
        writeValidationEvidence(evidenceDirectory, validationCase, result, runtimeMilliseconds, result.passed);

        std::cout
            << "Case: " << result.caseName << '\n'
            << "Mesh: " << result.nx << "x" << result.ny << '\n'
            << "Converged: " << (result.passed ? "yes" : "no") << '\n'
            << "Iterations: " << result.iterations << " / " << validationCase.maxIterations << '\n'
            << "Final continuity residual: " << result.continuityResidual << '\n'
            << "Final U residual: " << result.uResidual << '\n'
            << "Final V residual: " << result.vResidual << '\n'
            << "Mass imbalance: " << result.massImbalance << '\n'
            << "Runtime (ms): " << runtimeMilliseconds << '\n'
            << "NaN/Inf: " << (result.finiteSolution ? "0" : "present") << '\n'
            << "Deterministic: " << (result.deterministic ? "yes" : "no") << '\n'
            << "Evidence written to: " << evidenceDirectory.string() << '\n';

        return result.passed ? 0 : 1;
    }

    if (argc > 1 && std::string_view(argv[1]) == "--validate-20")
    {
        cfd::ValidationCase validationCase;
        validationCase.name = "lid_driven_cavity_20x20";
        validationCase.continuityTolerance = 100.0;
        validationCase.momentumTolerance = 10.0;
        validationCase.maxIterations = 200;

        const auto start = std::chrono::steady_clock::now();
        const cfd::ValidationResult result = cfd::ValidationRunner{}.run(validationCase);
        const auto end = std::chrono::steady_clock::now();
        const double runtimeMilliseconds = std::chrono::duration<double, std::milli>(end - start).count();
        constexpr double strictTolerance = 1e-8;
        const bool strictConverged = result.finiteSolution
            && result.continuityResidual <= strictTolerance
            && result.uResidual <= strictTolerance
            && result.vResidual <= strictTolerance;

        writeValidationEvidence(
            "results/validation/lid_driven_cavity/20x20", validationCase, result, runtimeMilliseconds, strictConverged
        );

        std::cout
            << "Case: " << result.caseName << '\n'
            << "Strict converged: " << (strictConverged ? "yes" : "no") << '\n'
            << "Smoke gate passed: " << (result.passed ? "yes" : "no") << '\n'
            << "Iterations: " << result.iterations << '\n'
            << "Final continuity residual: " << result.continuityResidual << '\n'
            << "Final U residual: " << result.uResidual << '\n'
            << "Final V residual: " << result.vResidual << '\n'
            << "Mass imbalance: " << result.massImbalance << '\n'
            << "Runtime (ms): " << runtimeMilliseconds << '\n'
            << "NaN/Inf: " << (result.finiteSolution ? "0" : "present") << '\n';
        std::cout << "History samples: " << result.iterations << '\n';
        return strictConverged ? 0 : 1;
    }

    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);

    std::cout
        << "CFDApp C++20\n"
        << "Cells: "
        << mesh.cellCount()
        << '\n';

    return 0;
}
