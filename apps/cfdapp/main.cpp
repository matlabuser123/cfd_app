#include <iostream>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string_view>

#include "core/mesh/Mesh.hpp"
#include "validation/ValidationRunner.hpp"

int main(int argc, char* argv[])
{
    if (argc > 1 && std::string_view(argv[1]) == "--version")
    {
        std::cout << "CFDApp " << CFDAPP_VERSION << '\n';
        return 0;
    }

    if (argc > 1 && std::string_view(argv[1]) == "--case")
    {
        std::cout << "Case execution is not wired into the CLI yet.\n";
        return 2;
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
        const std::filesystem::path evidenceDirectory = "results/validation/lid_driven_cavity/20x20";
        std::filesystem::create_directories(evidenceDirectory);

        {
            std::ofstream report(evidenceDirectory / "validation_report.txt");
            report << std::setprecision(17)
                << "case: " << result.caseName << '\n'
                << "mesh: " << result.nx << "x" << result.ny << '\n'
                << "rho: 1.0\nmu: 0.01\nreynolds: 100\n"
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
                << "  \"reynolds_number\": 100.0,\n"
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
