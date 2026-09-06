#include "numerics/solvers/CGSolver.hpp"

#include "numerics/solvers/Residual.hpp"
#include "numerics/solvers/VectorOperations.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace cfd
{

CGSolver::CGSolver(SolverSettings settings)
    : LinearSolver(settings)
{
}

LinearSolverResult CGSolver::solve(LinearSystem& system)
{
    const std::size_t size = system.size();
    if (system.matrix().rows() != size || system.matrix().cols() != size)
    {
        throw std::invalid_argument("CG requires a square matrix");
    }

    LinearSolverResult result;
    if (size == 0)
    {
        result.converged = true;
        return result;
    }

    std::vector<double> residual = calculateResidual(system);
    const double initialResidual = norm(residual);
    result.initialResidual = initialResidual;
    result.finalResidual = initialResidual;
    const double target = std::max(settings_.absoluteTolerance, settings_.relativeTolerance * initialResidual);
    if (initialResidual <= target)
    {
        result.converged = true;
        return result;
    }

    std::vector<double> direction = residual;
    std::vector<double> matrixDirection(size, 0.0);
    double residualSquared = dot(residual, residual);
    constexpr double breakdownTolerance = 1e-30;

    for (std::size_t iteration = 1; iteration <= settings_.maxIterations; ++iteration)
    {
        system.matrix().multiplyInto(direction, matrixDirection);
        const double denominator = dot(direction, matrixDirection);
        if (denominator <= breakdownTolerance)
        {
            result.iterations = iteration;
            result.failureReason = "CG breakdown: matrix is not positive definite";
            return result;
        }

        const double alpha = residualSquared / denominator;
        axpy(alpha, direction, system.solution());
        axpy(-alpha, matrixDirection, residual);
        const double residualNorm = norm(residual);
        result.iterations = iteration;
        result.finalResidual = residualNorm;
        if (residualNorm <= target)
        {
            result.converged = true;
            return result;
        }

        const double nextResidualSquared = dot(residual, residual);
        const double beta = nextResidualSquared / residualSquared;
        for (std::size_t index = 0; index < size; ++index)
        {
            direction[index] = residual[index] + beta * direction[index];
        }
        residualSquared = nextResidualSquared;
    }

    result.failureReason = "Maximum iterations reached";
    return result;
}

}
