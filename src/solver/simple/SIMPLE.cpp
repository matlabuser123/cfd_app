#include "solver/simple/SIMPLE.hpp"

#include "numerics/matrix/LinearSystem.hpp"
#include "numerics/solvers/BiCGSTABSolver.hpp"
#include "numerics/solvers/CGSolver.hpp"
#include "numerics/solvers/Residual.hpp"
#include "numerics/solvers/VectorOperations.hpp"
#include "parallel/ParallelFor.hpp"
#include "profiling/ProfileScope.hpp"
#include "profiling/Profiler.hpp"
#include "physics/flux/MassFluxCalculator.hpp"
#include "physics/momentum/MomentumEquation.hpp"
#include "physics/pressure/PressureEquation.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>

namespace cfd
{

namespace
{

template <typename Function>
void profiled(Profiler* profiler, const char* name, Function&& function)
{
    if (profiler == nullptr)
    {
        function();
        return;
    }

    ProfileScope scope(*profiler, name);
    function();
}

}

SIMPLE::SIMPLE(const Mesh& mesh, double density, double viscosity, SIMPLESettings settings)
    : mesh_(mesh),
      density_(density),
      viscosity_(viscosity),
      settings_(settings)
{
    if (density_ <= 0.0 || viscosity_ <= 0.0) throw std::invalid_argument("SIMPLE properties must be positive");
    if (settings_.maxIterations == 0 || settings_.momentumTolerance <= 0.0 || settings_.pressureTolerance <= 0.0 || settings_.continuityTolerance <= 0.0) throw std::invalid_argument("SIMPLE tolerances and iterations must be positive");
    if (settings_.velocityRelaxation <= 0.0 || settings_.velocityRelaxation > 1.0 || settings_.pressureRelaxation <= 0.0 || settings_.pressureRelaxation > 1.0) throw std::invalid_argument("SIMPLE relaxation factors must be in (0, 1]");
}

void SIMPLE::setBoundaryConditions(std::vector<const BoundaryCondition*> conditions)
{
    boundaryConditions_ = std::move(conditions);
}

void SIMPLE::setProfiler(Profiler* profiler) noexcept
{
    profiler_ = profiler;
}

void SIMPLE::validateFields(const VectorField& velocity, const ScalarField& pressure) const
{
    if (velocity.size() != mesh_.cellCount() || pressure.size() != mesh_.cellCount()) throw std::invalid_argument("SIMPLE fields must match mesh size");
}

void SIMPLE::applyBoundaryConditions(VectorField& velocity, ScalarField& pressure) const
{
    for (const BoundaryCondition* condition : boundaryConditions_)
    {
        if (condition == nullptr) throw std::invalid_argument("SIMPLE boundary condition must not be null");
        try { condition->apply(velocity); } catch (const std::logic_error&) { }
        try { condition->apply(pressure); } catch (const std::logic_error&) { }
    }
}

SIMPLEResult SIMPLE::solve(
    VectorField& velocity,
    ScalarField& pressure,
    const std::atomic_bool* cancelRequested,
    std::function<void(const SIMPLEIteration&)> iterationCallback
)
{
    validateFields(velocity, pressure);
    MomentumEquation momentum(mesh_, density_, viscosity_);
    PressureEquation pressureEquation(mesh_, density_);
    MassFluxCalculator fluxes(mesh_, density_);
    BiCGSTABSolver momentumSolver({1000, settings_.momentumTolerance, settings_.momentumTolerance});
    CGSolver pressureSolver({1000, settings_.pressureTolerance, settings_.pressureTolerance});
    ScalarField diagonalU(mesh_.cellCount());
    ScalarField diagonalV(mesh_.cellCount());
    SIMPLEResult result;
    const auto scaleRowsByDiagonal = [](LinearSystem& system)
    {
        for (std::size_t row = 0; row < system.size(); ++row)
        {
            const double diagonal = system.matrix().diagonal(row);
            if (diagonal <= 0.0 || !std::isfinite(diagonal)) throw std::runtime_error("SIMPLE encountered an invalid matrix diagonal");
            system.matrix().scaleRow(row, 1.0 / diagonal);
            system.rhs()[row] /= diagonal;
        }
    };
    const auto gaussSeidelFallback = [](LinearSystem& system, double tolerance)
    {
        std::fill(system.solution().begin(), system.solution().end(), 0.0);
        double residualNorm = 0.0;
        for (std::size_t iteration = 0; iteration < 5000; ++iteration)
        {
            for (std::size_t row = 0; row < system.size(); ++row)
            {
                const double diagonal = system.matrix().diagonal(row);
                if (diagonal <= 0.0 || !std::isfinite(diagonal)) return std::numeric_limits<double>::infinity();
                double offDiagonalSum = 0.0;
                for (const auto& [column, value] : system.matrix().rowEntries(row))
                {
                    if (column != row) offDiagonalSum += value * system.solution()[column];
                }
                system.solution()[row] = (system.rhs()[row] - offDiagonalSum) / diagonal;
            }
            residualNorm = calculateResidualNorm(system);
            if (residualNorm <= tolerance) return residualNorm;
        }
        return residualNorm;
    };

    for (std::size_t iteration = 1; iteration <= settings_.maxIterations; ++iteration)
    {
        if (cancelRequested != nullptr && cancelRequested->load())
        {
            result.cancelled = true;
            return result;
        }
        std::optional<ProfileScope> iterationScope;
        if (profiler_ != nullptr) iterationScope.emplace(*profiler_, "simple_iteration");
        applyBoundaryConditions(velocity, pressure);
        LinearSystem uSystem(mesh_.cellCount());
        profiled(profiler_, "u_momentum_assembly", [&] { momentum.assembleU(velocity, pressure, uSystem); });
        parallelFor(0, mesh_.cellCount(), [&](std::size_t cell) { diagonalU[cell] = uSystem.matrix().diagonal(cell); });
        scaleRowsByDiagonal(uSystem);
        LinearSolverResult uSolve;
        profiled(profiler_, "u_linear_solve", [&] { uSolve = momentumSolver.solve(uSystem); });
        const double uResidual = uSolve.converged ? uSolve.finalResidual : gaussSeidelFallback(uSystem, settings_.momentumTolerance);
        if (uResidual > settings_.momentumTolerance) throw std::runtime_error("SIMPLE U-momentum solve failed: " + uSolve.failureReason + " (residual " + std::to_string(uResidual) + ")");
        parallelFor(0, mesh_.cellCount(), [&](std::size_t cell)
        {
            velocity[cell].x = settings_.velocityRelaxation * uSystem.solution()[cell] + (1.0 - settings_.velocityRelaxation) * velocity[cell].x;
        });
        applyBoundaryConditions(velocity, pressure);

        LinearSystem vSystem(mesh_.cellCount());
        profiled(profiler_, "v_momentum_assembly", [&] { momentum.assembleV(velocity, pressure, vSystem); });
        parallelFor(0, mesh_.cellCount(), [&](std::size_t cell) { diagonalV[cell] = vSystem.matrix().diagonal(cell); });
        scaleRowsByDiagonal(vSystem);
        LinearSolverResult vSolve;
        profiled(profiler_, "v_linear_solve", [&] { vSolve = momentumSolver.solve(vSystem); });
        const double vResidual = vSolve.converged ? vSolve.finalResidual : gaussSeidelFallback(vSystem, settings_.momentumTolerance);
        if (vResidual > settings_.momentumTolerance) throw std::runtime_error("SIMPLE V-momentum solve failed: " + vSolve.failureReason + " (residual " + std::to_string(vResidual) + ")");
        parallelFor(0, mesh_.cellCount(), [&](std::size_t cell)
        {
            velocity[cell].y = settings_.velocityRelaxation * vSystem.solution()[cell] + (1.0 - settings_.velocityRelaxation) * velocity[cell].y;
        });
        applyBoundaryConditions(velocity, pressure);

        LinearSystem pressureSystem(mesh_.cellCount());
        profiled(profiler_, "pressure_assembly", [&] { pressureEquation.assemble(velocity, pressure, diagonalU, diagonalV, pressureSystem); });
        scaleRowsByDiagonal(pressureSystem);
        LinearSolverResult pressureSolve;
        profiled(profiler_, "pressure_linear_solve", [&] { pressureSolve = pressureSolver.solve(pressureSystem); });
        const double pressureResidual = pressureSolve.converged ? pressureSolve.finalResidual : gaussSeidelFallback(pressureSystem, settings_.pressureTolerance);
        if (pressureResidual > settings_.pressureTolerance) throw std::runtime_error("SIMPLE pressure solve failed: " + pressureSolve.failureReason + " (residual " + std::to_string(pressureResidual) + ")");
        ScalarField correction(mesh_.cellCount());
        parallelFor(0, mesh_.cellCount(), [&](std::size_t cell) { correction[cell] = pressureSystem.solution()[cell]; });
        profiled(profiler_, "velocity_correction", [&]
        {
            pressureEquation.applyPressureCorrection(pressure, correction, settings_.pressureRelaxation);
            pressureEquation.correctVelocity(velocity, correction, diagonalU, diagonalV);
        });
        applyBoundaryConditions(velocity, pressure);

        profiled(profiler_, "flux_correction", [&]
        {
            parallelFor(0, mesh_.cellCount(), [&](std::size_t cell)
            {
                static_cast<void>(fluxes.corrected(cell, velocity, correction, diagonalU, diagonalV));
            });
        });

        result.iterations = iteration;
        result.uResidual = uResidual;
        result.vResidual = vResidual;
        result.pressureResidual = pressureResidual;
        profiled(profiler_, "residual_calculation", [&]
        {
            result.continuityResidual = fluxes.correctedContinuityResidual(velocity, correction, diagonalU, diagonalV);
        });
        if (!std::isfinite(result.continuityResidual) || !std::isfinite(result.uResidual) || !std::isfinite(result.vResidual) || !std::isfinite(result.pressureResidual)) throw std::runtime_error("SIMPLE produced a non-finite residual");
        result.history.push_back({iteration, result.continuityResidual, result.uResidual, result.vResidual, result.pressureResidual});
        if (iterationCallback) iterationCallback(result.history.back());
        if (result.continuityResidual <= settings_.continuityTolerance && result.uResidual <= settings_.momentumTolerance && result.vResidual <= settings_.momentumTolerance && result.pressureResidual <= settings_.pressureTolerance)
        {
            result.converged = true;
            return result;
        }
    }
    return result;
}

}
