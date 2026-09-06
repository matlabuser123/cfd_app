#include <cassert>
#include <cmath>

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "numerics/matrix/LinearSystem.hpp"
#include "numerics/solvers/BiCGSTABSolver.hpp"
#include "numerics/solvers/Residual.hpp"
#include "parallel/ParallelRuntime.hpp"
#include "physics/pressure/PressureEquation.hpp"

namespace
{

bool nearlyEqual(double left, double right, double tolerance = 1e-8)
{
    return std::abs(left - right) < tolerance;
}

void assertEqualSystems(const cfd::LinearSystem& serial, const cfd::LinearSystem& parallel)
{
    assert(serial.size() == parallel.size());
    assert(serial.matrix().nonZeroCount() == parallel.matrix().nonZeroCount());
    for (std::size_t row = 0; row < serial.size(); ++row)
    {
        assert(nearlyEqual(serial.rhs()[row], parallel.rhs()[row]));
        for (std::size_t column = 0; column < serial.size(); ++column)
        {
            assert(nearlyEqual(serial.matrix().get(row, column), parallel.matrix().get(row, column)));
        }
    }
}

}

int main()
{
    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);
    cfd::VectorField zeroVelocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField pressure(mesh.cellCount(), 0.0);
    cfd::ScalarField diagonalU(mesh.cellCount(), 4.0);
    cfd::ScalarField diagonalV(mesh.cellCount(), 4.0);
    cfd::PressureEquation equation(mesh, 1.0, 0, 0.0);
    cfd::LinearSystem system(mesh.cellCount());

    cfd::ParallelRuntime::initialize({false, 1});
    equation.assemble(zeroVelocity, pressure, diagonalU, diagonalV, system);
    cfd::ParallelRuntime::initialize({true, 2});
    cfd::LinearSystem parallelSystem(mesh.cellCount());
    equation.assemble(zeroVelocity, pressure, diagonalU, diagonalV, parallelSystem);
    assertEqualSystems(system, parallelSystem);
    assert(system.matrix().rows() == 400);
    assert(system.matrix().cols() == 400);
    assert(system.matrix().get(0, 0) == 1.0);
    for (const double value : system.rhs()) assert(nearlyEqual(value, 0.0));
    cfd::BiCGSTABSolver solver({1000, 1e-10, 1e-8});
    const cfd::LinearSolverResult zeroResult = solver.solve(system);
    assert(zeroResult.converged);
    assert(nearlyEqual(system.solution()[0], 0.0));

    cfd::VectorField divergent(mesh.cellCount());
    for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell) divergent[cell].x = static_cast<double>(cell % mesh.nx()) * mesh.dx();
    equation.assemble(divergent, pressure, diagonalU, diagonalV, system);
    const std::size_t center = 10 + 10 * mesh.nx();
    assert(system.rhs()[center] < 0.0);
    const cfd::LinearSolverResult divergentResult = solver.solve(system);
    assert(divergentResult.converged);
    assert(cfd::calculateResidualNorm(system) < 1e-7);

    cfd::ScalarField correction(mesh.cellCount(), 0.0);
    correction[center] = 2.0;
    equation.applyPressureCorrection(pressure, correction, 0.5);
    assert(nearlyEqual(pressure[center], 1.0));
    cfd::VectorField correctedVelocity(mesh.cellCount(), {0.0, 0.0});
    equation.correctVelocity(correctedVelocity, correction, diagonalU, diagonalV);
    assert(std::isfinite(correctedVelocity[center].x));
    assert(std::isfinite(correctedVelocity[center].y));

    return 0;
}
