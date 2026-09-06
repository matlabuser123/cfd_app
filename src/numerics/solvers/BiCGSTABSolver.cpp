#include "numerics/solvers/BiCGSTABSolver.hpp"

#include "numerics/solvers/Residual.hpp"
#include "numerics/solvers/VectorOperations.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace cfd
{

BiCGSTABSolver::BiCGSTABSolver(SolverSettings settings)
    : LinearSolver(settings)
{
}

LinearSolverResult BiCGSTABSolver::solve(LinearSystem& system)
{
    const std::size_t size = system.size();
    if (system.matrix().rows() != size || system.matrix().cols() != size)
    {
        throw std::invalid_argument("BiCGSTAB requires a square matrix");
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

    const std::vector<double> shadowResidual = residual;
    std::vector<double> direction(size, 0.0);
    std::vector<double> product(size, 0.0);
    std::vector<double> intermediate(size, 0.0);
    std::vector<double> intermediateProduct(size, 0.0);
    double rhoOld = 1.0;
    double alpha = 1.0;
    double omega = 1.0;
    constexpr double breakdownTolerance = 1e-30;

    for (std::size_t iteration = 1; iteration <= settings_.maxIterations; ++iteration)
    {
        const double rhoNew = dot(shadowResidual, residual);
        if (std::abs(rhoNew) <= breakdownTolerance)
        {
            result.iterations = iteration;
            result.failureReason = "BiCGSTAB breakdown: shadow residual became orthogonal";
            return result;
        }

        const double beta = (rhoNew / rhoOld) * (alpha / omega);
        for (std::size_t index = 0; index < size; ++index)
        {
            direction[index] = residual[index] + beta * (direction[index] - omega * product[index]);
        }

        system.matrix().multiplyInto(direction, product);
        const double denominator = dot(shadowResidual, product);
        if (std::abs(denominator) <= breakdownTolerance)
        {
            result.iterations = iteration;
            result.failureReason = "BiCGSTAB breakdown: zero alpha denominator";
            return result;
        }

        alpha = rhoNew / denominator;
        for (std::size_t index = 0; index < size; ++index)
        {
            intermediate[index] = residual[index] - alpha * product[index];
        }

        const double intermediateNorm = norm(intermediate);
        if (intermediateNorm <= target)
        {
            axpy(alpha, direction, system.solution());
            result.converged = true;
            result.iterations = iteration;
            result.finalResidual = intermediateNorm;
            return result;
        }

        system.matrix().multiplyInto(intermediate, intermediateProduct);
        const double denominatorOmega = dot(intermediateProduct, intermediateProduct);
        if (denominatorOmega <= breakdownTolerance)
        {
            result.iterations = iteration;
            result.failureReason = "BiCGSTAB breakdown: zero omega denominator";
            return result;
        }

        omega = dot(intermediateProduct, intermediate) / denominatorOmega;
        if (std::abs(omega) <= breakdownTolerance)
        {
            result.iterations = iteration;
            result.failureReason = "BiCGSTAB breakdown: omega became zero";
            return result;
        }

        axpy(alpha, direction, system.solution());
        axpy(omega, intermediate, system.solution());
        for (std::size_t index = 0; index < size; ++index)
        {
            residual[index] = intermediate[index] - omega * intermediateProduct[index];
        }

        result.iterations = iteration;
        result.finalResidual = norm(residual);
        if (result.finalResidual <= target)
        {
            result.converged = true;
            return result;
        }
        rhoOld = rhoNew;
    }

    result.failureReason = "Maximum iterations reached";
    return result;
}

}
