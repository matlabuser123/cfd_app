#include <cassert>
#include <atomic>
#include <cmath>
#include <vector>

#include "core/boundary/DirichletBC.hpp"
#include "core/boundary/WallBC.hpp"
#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "parallel/ParallelRuntime.hpp"
#include "solver/simple/SIMPLE.hpp"

int main()
{
    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);
    cfd::VectorField serialVelocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField serialPressure(mesh.cellCount(), 0.0);
    cfd::VectorField parallelVelocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField parallelPressure(mesh.cellCount(), 0.0);
    const std::vector<std::size_t> top{380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399};
    const std::vector<std::size_t> bottom{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    const std::vector<std::size_t> left{0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240, 260, 280, 300, 320, 340, 360, 380};
    const std::vector<std::size_t> right{19, 39, 59, 79, 99, 119, 139, 159, 179, 199, 219, 239, 259, 279, 299, 319, 339, 359, 379, 399};
    cfd::DirichletBC lid("top", top, cfd::Vector2{1.0, 0.0});
    cfd::WallBC bottomWall("bottom", bottom);
    cfd::WallBC leftWall("left", left);
    cfd::WallBC rightWall("right", right);

    cfd::ParallelRuntime::initialize({false, 1});
    cfd::SIMPLE serialSimple(mesh, 1.0, 0.01, {200, 10.0, 10.0, 100.0, 0.7, 0.3});
    serialSimple.setBoundaryConditions({&lid, &bottomWall, &leftWall, &rightWall});
    const cfd::SIMPLEResult serialResult = serialSimple.solve(serialVelocity, serialPressure);

    for (const int threadCount : {1, 2, 4, 8})
    {
        parallelVelocity.fill({0.0, 0.0});
        parallelPressure.fill(0.0);
        cfd::ParallelRuntime::initialize({true, threadCount});
        cfd::SIMPLE parallelSimple(mesh, 1.0, 0.01, {200, 10.0, 10.0, 100.0, 0.7, 0.3});
        parallelSimple.setBoundaryConditions({&lid, &bottomWall, &leftWall, &rightWall});
        const cfd::SIMPLEResult parallelResult = parallelSimple.solve(parallelVelocity, parallelPressure);

        assert(parallelResult.iterations > 0);
        assert(parallelResult.iterations <= 200);
        assert(parallelResult.history.size() == parallelResult.iterations);
        assert(serialResult.iterations == parallelResult.iterations);
        assert(std::abs(serialResult.continuityResidual - parallelResult.continuityResidual) < 1e-12);
        assert(std::abs(serialResult.uResidual - parallelResult.uResidual) < 1e-12);
        assert(std::abs(serialResult.vResidual - parallelResult.vResidual) < 1e-12);
        for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell)
        {
            assert(std::isfinite(parallelVelocity[cell].x));
            assert(std::isfinite(parallelVelocity[cell].y));
            assert(std::isfinite(parallelPressure[cell]));
            assert(std::abs(serialVelocity[cell].x - parallelVelocity[cell].x) < 1e-12);
            assert(std::abs(serialVelocity[cell].y - parallelVelocity[cell].y) < 1e-12);
            assert(std::abs(serialPressure[cell] - parallelPressure[cell]) < 1e-12);
        }
        assert(std::abs(parallelVelocity[381].x - 1.0) < 1e-8);
        assert(std::abs(parallelVelocity[390].x - 1.0) < 1e-8);
    }

    cfd::VectorField cancelledVelocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField cancelledPressure(mesh.cellCount(), 0.0);
    std::atomic_bool cancelRequested{true};
    cfd::SIMPLE cancelledSimple(mesh, 1.0, 0.01, {200, 10.0, 10.0, 100.0, 0.7, 0.3});
    cancelledSimple.setBoundaryConditions({&lid, &bottomWall, &leftWall, &rightWall});
    const cfd::SIMPLEResult cancelledResult = cancelledSimple.solve(cancelledVelocity, cancelledPressure, &cancelRequested);
    assert(cancelledResult.cancelled);
    assert(!cancelledResult.converged);
    assert(cancelledResult.iterations == 0);
    assert(cancelledResult.history.empty());

    cfd::VectorField monitoredVelocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField monitoredPressure(mesh.cellCount(), 0.0);
    std::vector<cfd::SIMPLEIteration> reportedIterations;
    cfd::SIMPLE monitoredSimple(mesh, 1.0, 0.01, {200, 10.0, 10.0, 100.0, 0.7, 0.3});
    monitoredSimple.setBoundaryConditions({&lid, &bottomWall, &leftWall, &rightWall});
    const cfd::SIMPLEResult monitoredResult = monitoredSimple.solve(
        monitoredVelocity,
        monitoredPressure,
        nullptr,
        [&](const cfd::SIMPLEIteration& iteration) { reportedIterations.push_back(iteration); }
    );
    assert(reportedIterations.size() == monitoredResult.history.size());
    for (std::size_t index = 0; index < reportedIterations.size(); ++index)
    {
        assert(reportedIterations[index].iteration == monitoredResult.history[index].iteration);
        assert(std::abs(reportedIterations[index].continuityResidual - monitoredResult.history[index].continuityResidual) < 1e-12);
    }
    return 0;
}
