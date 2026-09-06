#include <cassert>
#include <cmath>
#include <stdexcept>

#include "core/mesh/Mesh.hpp"
#include "numerics/matrix/LinearSystem.hpp"
#include "numerics/solvers/BiCGSTABSolver.hpp"
#include "numerics/solvers/CGSolver.hpp"
#include "numerics/solvers/Residual.hpp"
#include "numerics/solvers/SolverFactory.hpp"
#include "numerics/solvers/VectorOperations.hpp"

namespace
{

bool nearlyEqual(double left, double right, double tolerance = 1e-9)
{
    return std::abs(left - right) < tolerance;
}

cfd::SolverSettings accurateSettings()
{
    return cfd::SolverSettings{500, 1e-12, 1e-10};
}

}

int main()
{
    assert(nearlyEqual(cfd::dot({1.0, 2.0}, {3.0, 4.0}), 11.0));
    assert(nearlyEqual(cfd::norm({3.0, 4.0}), 5.0));
    std::vector<double> y{1.0, 2.0};
    cfd::axpy(2.0, {3.0, 4.0}, y);
    assert(nearlyEqual(y[0], 7.0));
    assert(nearlyEqual(y[1], 10.0));

    cfd::LinearSystem spd(2);
    spd.matrix().set(0, 0, 4.0);
    spd.matrix().set(0, 1, 1.0);
    spd.matrix().set(1, 0, 1.0);
    spd.matrix().set(1, 1, 3.0);
    spd.rhs() = {1.0, 2.0};
    cfd::CGSolver cg(accurateSettings());
    const cfd::LinearSolverResult cgResult = cg.solve(spd);
    assert(cgResult.converged);
    assert(cfd::calculateResidualNorm(spd) < 1e-9);
    assert(nearlyEqual(4.0 * spd.solution()[0] + spd.solution()[1], 1.0));
    assert(nearlyEqual(spd.solution()[0] + 3.0 * spd.solution()[1], 2.0));

    cfd::LinearSystem nonsymmetric(3);
    nonsymmetric.matrix().set(0, 0, 4.0);
    nonsymmetric.matrix().set(0, 1, 1.0);
    nonsymmetric.matrix().set(1, 1, 5.0);
    nonsymmetric.matrix().set(1, 2, 2.0);
    nonsymmetric.matrix().set(2, 0, 3.0);
    nonsymmetric.matrix().set(2, 2, 6.0);
    nonsymmetric.rhs() = {10.0, 16.0, 21.0};
    cfd::BiCGSTABSolver bicgstab(accurateSettings());
    const cfd::LinearSolverResult bicgResult = bicgstab.solve(nonsymmetric);
    assert(bicgResult.converged);
    assert(cfd::calculateResidualNorm(nonsymmetric) < 1e-8);

    cfd::LinearSystem stencilSystem(400);
    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);
    for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell)
    {
        stencilSystem.matrix().set(cell, cell, 4.0);
        if (const auto east = mesh.east(cell)) stencilSystem.matrix().set(cell, *east, -1.0);
        if (const auto west = mesh.west(cell)) stencilSystem.matrix().set(cell, *west, -1.0);
        if (const auto north = mesh.north(cell)) stencilSystem.matrix().set(cell, *north, -1.0);
        if (const auto south = mesh.south(cell)) stencilSystem.matrix().set(cell, *south, -1.0);
        stencilSystem.rhs()[cell] = 1.0;
    }
    cfd::CGSolver stencilSolver(accurateSettings());
    const cfd::LinearSolverResult stencilResult = stencilSolver.solve(stencilSystem);
    assert(stencilResult.converged);
    assert(cfd::calculateResidualNorm(stencilSystem) < 1e-8);

    cfd::LinearSystem alreadySolved(1);
    alreadySolved.matrix().set(0, 0, 2.0);
    alreadySolved.rhs()[0] = 4.0;
    alreadySolved.solution()[0] = 2.0;
    const cfd::LinearSolverResult initialResult = cg.solve(alreadySolved);
    assert(initialResult.converged);
    assert(initialResult.iterations == 0);

    cfd::LinearSystem limited(2);
    limited.matrix().set(0, 0, 4.0);
    limited.matrix().set(1, 1, 3.0);
    limited.rhs() = {1.0, 1.0};
    cfd::BiCGSTABSolver oneStep(cfd::SolverSettings{1, 1e-14, 1e-14});
    const cfd::LinearSolverResult limitedResult = oneStep.solve(limited);
    assert(!limitedResult.converged);
    assert(limitedResult.failureReason == "Maximum iterations reached" || limitedResult.iterations == 1);

    // Relative-tolerance formula regression: target = max(absoluteTolerance,
    // relativeTolerance * initialResidual). With x0 = 0 (LinearSystem's default), the
    // initial residual is exactly ||rhs||, so a 1x1 system with matrix entry 1.0 gives an
    // initial residual equal to the chosen rhs value directly -- letting these three cases
    // exercise initialResidual < 1, ~= 1, and > 1 against the same relativeTolerance,
    // confirming CGSolver and BiCGSTABSolver scale the target from the initial residual
    // (not the current one) and never trivially "converge" at 0 iterations when the
    // initial residual genuinely exceeds that target.
    for (const double initialResidualMagnitude : {0.1, 1.0, 10.0})
    {
        const double relativeTolerance = 1e-6;
        const double absoluteTolerance = 1e-8;
        const double expectedTarget = std::max(absoluteTolerance, relativeTolerance * initialResidualMagnitude);

        cfd::LinearSystem cgSystem(1);
        cgSystem.matrix().set(0, 0, 1.0);
        cgSystem.rhs()[0] = initialResidualMagnitude;
        cfd::CGSolver toleranceCg({100, absoluteTolerance, relativeTolerance});
        const cfd::LinearSolverResult cgResult = toleranceCg.solve(cgSystem);
        assert(nearlyEqual(cgResult.initialResidual, initialResidualMagnitude, 1e-9));
        assert(cgResult.iterations >= 1);
        assert(cgResult.converged);
        assert(std::isfinite(cgResult.finalResidual));
        assert(cgResult.finalResidual <= expectedTarget + 1e-12);
        assert(cgResult.finalResidual < cgResult.initialResidual);

        cfd::LinearSystem bicgSystem(1);
        bicgSystem.matrix().set(0, 0, 1.0);
        bicgSystem.rhs()[0] = initialResidualMagnitude;
        cfd::BiCGSTABSolver toleranceBicg({100, absoluteTolerance, relativeTolerance});
        const cfd::LinearSolverResult bicgResult = toleranceBicg.solve(bicgSystem);
        assert(nearlyEqual(bicgResult.initialResidual, initialResidualMagnitude, 1e-9));
        assert(bicgResult.iterations >= 1);
        assert(bicgResult.converged);
        assert(std::isfinite(bicgResult.finalResidual));
        assert(bicgResult.finalResidual <= expectedTarget + 1e-12);
        assert(bicgResult.finalResidual < bicgResult.initialResidual);
    }

    auto factorySolver = cfd::SolverFactory::create("CG", accurateSettings());
    assert(factorySolver != nullptr);

    bool threw = false;
    try
    {
        static_cast<void>(cfd::SolverFactory::create("unknown"));
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    assert(threw);

    return 0;
}
