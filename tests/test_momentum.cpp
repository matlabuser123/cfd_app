#include <cassert>
#include <cmath>

#include "core/boundary/DirichletBC.hpp"
#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "numerics/matrix/LinearSystem.hpp"
#include "numerics/solvers/BiCGSTABSolver.hpp"
#include "parallel/ParallelRuntime.hpp"
#include "physics/momentum/MomentumEquation.hpp"

namespace
{

bool nearlyEqual(double left, double right, double tolerance = 1e-9)
{
    return std::abs(left - right) < tolerance;
}

void assertFinite(const std::vector<double>& values)
{
    for (const double value : values) assert(std::isfinite(value));
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
    cfd::VectorField velocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField pressure(mesh.cellCount(), 0.0);
    cfd::DirichletBC movingLid("top", {380, 381, 382, 383, 384}, {1.0, 0.0});
    movingLid.apply(velocity);

    cfd::MomentumEquation equation(mesh, 1.0, 0.01);
    cfd::LinearSystem uSystem(mesh.cellCount());
    cfd::LinearSystem vSystem(mesh.cellCount());
    cfd::ParallelRuntime::initialize({false, 1});
    equation.assembleU(velocity, pressure, uSystem);
    equation.assembleV(velocity, pressure, vSystem);

    cfd::ParallelRuntime::initialize({true, 2});
    cfd::LinearSystem parallelUSystem(mesh.cellCount());
    cfd::LinearSystem parallelVSystem(mesh.cellCount());
    equation.assembleU(velocity, pressure, parallelUSystem);
    equation.assembleV(velocity, pressure, parallelVSystem);
    assertEqualSystems(uSystem, parallelUSystem);
    assertEqualSystems(vSystem, parallelVSystem);

    assert(uSystem.matrix().rows() == 400);
    assert(uSystem.matrix().cols() == 400);
    assert(uSystem.matrix().nonZeroCount() > 400);
    assert(nearlyEqual(uSystem.matrix().diagonal(0), 1.0));
    const std::size_t center = 10 + 10 * mesh.nx();
    assert(uSystem.matrix().get(center, center) > 0.0);
    assert(nearlyEqual(uSystem.rhs()[0], 0.0));
    assert(nearlyEqual(uSystem.rhs()[380], 1.0));

    cfd::ScalarField pressureGradient(mesh.cellCount());
    for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell)
    {
        pressureGradient[cell] = static_cast<double>(cell % mesh.nx()) * mesh.dx();
    }
    equation.assembleU(velocity, pressureGradient, uSystem);
    assert(nearlyEqual(uSystem.rhs()[center], -1.0));

    cfd::BiCGSTABSolver solver({1000, 1e-10, 1e-8});
    const cfd::LinearSolverResult result = solver.solve(uSystem);
    assert(result.converged);
    assertFinite(uSystem.solution());
    assert(nearlyEqual(uSystem.solution()[380], 1.0, 1e-6));
    assert(nearlyEqual(uSystem.solution()[381], 1.0, 1e-6));

    cfd::LinearSystem repeatSystem(mesh.cellCount());
    equation.assembleV(velocity, pressure, repeatSystem);
    assert(repeatSystem.matrix().nonZeroCount() == vSystem.matrix().nonZeroCount());

    return 0;
}
