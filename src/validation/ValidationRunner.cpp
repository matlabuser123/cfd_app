#include "validation/ValidationRunner.hpp"

#include "core/boundary/DirichletBC.hpp"
#include "core/boundary/WallBC.hpp"
#include "core/mesh/Mesh.hpp"
#include "solver/simple/SIMPLE.hpp"
#include "validation/ConservationValidator.hpp"
#include "validation/ConvergenceValidator.hpp"
#include "validation/DeterminismValidator.hpp"
#include "validation/MeshValidator.hpp"

#include <atomic>
#include <chrono>
#include <cmath>
#include <stdexcept>
#include <vector>

namespace cfd
{
namespace
{

struct Snapshot
{
    ValidationResult result;
    VectorField velocity;
    ScalarField pressure;
};

std::vector<std::size_t> rowCells(std::size_t nx, std::size_t row)
{
    std::vector<std::size_t> cells;
    cells.reserve(nx);
    for (std::size_t column = 0; column < nx; ++column) cells.push_back(row * nx + column);
    return cells;
}

Snapshot runOnce(
    const ValidationCase& validationCase,
    const std::atomic_bool* cancelRequested,
    std::function<void(const SIMPLEIteration&)> iterationCallback = {}
)
{
    Mesh mesh;
    mesh.generateUniform(validationCase.nx, validationCase.ny, validationCase.length, validationCase.height);
    if (!MeshValidator::valid(mesh)) throw std::invalid_argument("Validation mesh is invalid");

    VectorField velocity(mesh.cellCount(), {0.0, 0.0});
    ScalarField pressure(mesh.cellCount(), 0.0);
    const std::vector<std::size_t> top = rowCells(validationCase.nx, validationCase.ny - 1);
    const std::vector<std::size_t> bottom = rowCells(validationCase.nx, 0);
    std::vector<std::size_t> left;
    std::vector<std::size_t> right;
    for (std::size_t row = 0; row < validationCase.ny; ++row)
    {
        left.push_back(row * validationCase.nx);
        right.push_back(row * validationCase.nx + validationCase.nx - 1);
    }

    DirichletBC lid("top", top, Vector2{validationCase.lidVelocity, 0.0});
    WallBC bottomWall("bottom", bottom);
    WallBC leftWall("left", left);
    WallBC rightWall("right", right);
    SIMPLE simple(mesh, validationCase.density, validationCase.viscosity, {
        validationCase.maxIterations,
        validationCase.momentumTolerance,
        validationCase.pressureTolerance,
        validationCase.continuityTolerance,
        0.7,
        0.3,
        validationCase.innerMomentumTolerance,
        validationCase.innerPressureTolerance
    });
    simple.setBoundaryConditions({&lid, &bottomWall, &leftWall, &rightWall});

    const SIMPLEResult simpleResult = simple.solve(velocity, pressure, cancelRequested, std::move(iterationCallback));
    ValidationResult result;
    result.cancelled = simpleResult.cancelled;
    result.caseName = validationCase.name;
    result.nx = validationCase.nx;
    result.ny = validationCase.ny;
    result.iterations = simpleResult.iterations;
    result.continuityResidual = simpleResult.continuityResidual;
    result.uResidual = simpleResult.uResidual;
    result.vResidual = simpleResult.vResidual;
    result.massImbalance = ConservationValidator::continuityResidual(mesh, velocity, validationCase.density);
    const std::size_t center = (validationCase.ny / 2) * validationCase.nx + validationCase.nx / 2;
    result.centerVelocityU = velocity[center].x;
    result.centerVelocityV = velocity[center].y;
    result.finiteSolution = true;
    for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell)
    {
        result.finiteSolution = result.finiteSolution && std::isfinite(velocity[cell].x) && std::isfinite(velocity[cell].y) && std::isfinite(pressure[cell]);
    }
    result.passed = !result.cancelled && simpleResult.converged && result.finiteSolution && ConvergenceValidator::converged(result, validationCase);
    return {result, velocity, pressure};
}

}

ValidationResult ValidationRunner::run(
    const ValidationCase& validationCase,
    const std::atomic_bool* cancelRequested,
    std::function<void(const SIMPLEIteration&)> iterationCallback
) const
{
    const auto start = std::chrono::steady_clock::now();
    const Snapshot first = runOnce(validationCase, cancelRequested, std::move(iterationCallback));
    if (first.result.cancelled)
    {
        ValidationResult result = first.result;
        result.runtimeSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        return result;
    }

    const Snapshot second = runOnce(validationCase, cancelRequested);
    if (second.result.cancelled)
    {
        ValidationResult result = second.result;
        result.runtimeSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
        return result;
    }
    const auto end = std::chrono::steady_clock::now();

    ValidationResult result = first.result;
    result.runtimeSeconds = std::chrono::duration<double>(end - start).count();
    result.deterministic = DeterminismValidator::vectorEqual(first.velocity, second.velocity, 1e-12)
        && DeterminismValidator::scalarEqual(first.pressure, second.pressure, 1e-12);
    result.passed = result.passed && result.deterministic;
    return result;
}

std::vector<CenterlinePoint> extractVerticalUProfile(
    const ScalarField& velocityU,
    std::size_t nx,
    std::size_t ny,
    double dy
)
{
    if (nx == 0 || ny == 0 || velocityU.size() != nx * ny) throw std::invalid_argument("Centerline field dimensions are invalid");
    std::vector<CenterlinePoint> profile;
    profile.reserve(ny);
    const std::size_t centerColumn = nx / 2;
    for (std::size_t row = 0; row < ny; ++row)
    {
        profile.push_back({static_cast<double>(row) * dy, velocityU[row * nx + centerColumn]});
    }
    return profile;
}

}
