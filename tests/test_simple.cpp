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

    // Characterization test for TODO.md item #3a's "frozen field" finding -- pins the
    // CURRENT, still-unresolved behavior so it is visible and must be deliberately updated
    // (not silently left stale) once the root cause is fixed, rather than being an
    // undocumented gap no test exercises at all.
    //
    // At the loose per-iteration tolerance every caller in this codebase uses (10.0/20.0,
    // same as every other case in this file), the inner momentum/pressure linear solves
    // trivially "converge" at their initial (zero-guess) residual -- which SIMPLE.cpp's
    // relativeTolerance = 0 fix (see SIMPLE.cpp) prevents from being *inflated* further,
    // but does not by itself make small enough to force genuine iterative work here, since
    // 10.0/20.0 already exceeds this problem's natural per-iteration residual on their own.
    // The result: SIMPLE "converges" in a single outer iteration with the interior velocity
    // field left exactly at its initial (0, 0) -- only boundary cells show motion, and only
    // because DirichletBC/WallBC apply their prescribed values directly, not because the
    // momentum equation was actually solved.
    //
    // Tightening the tolerance enough to force real per-iteration work was tried while
    // investigating this item and instead causes the outer SIMPLE iteration to visibly
    // diverge for this same case/mesh/relaxation-factor combination (see TODO.md item #3a)
    // -- a separate, deeper, not-yet-understood issue. There is currently no tolerance
    // setting that produces both genuine iterative solving and a stable result here, so
    // this test can only honestly assert the current (frozen) behavior, not the eventually
    // -correct one.
    {
        cfd::VectorField velocity(mesh.cellCount(), {0.0, 0.0});
        cfd::ScalarField pressure(mesh.cellCount(), 0.0);
        cfd::ParallelRuntime::initialize({false, 1});
        cfd::SIMPLE frozenFieldGuard(mesh, 1.0, 0.01, {50, 10.0, 20.0, 100.0, 0.7, 0.3});
        frozenFieldGuard.setBoundaryConditions({&lid, &bottomWall, &leftWall, &rightWall});
        const cfd::SIMPLEResult guardResult = frozenFieldGuard.solve(velocity, pressure);
        assert(guardResult.iterations == 1);
        assert(guardResult.converged);
        // Row 18 (one cell below the moving lid on row 19) has no momentum equation
        // reason to be exactly zero if real diffusion/advection from the lid occurred.
        for (std::size_t column = 0; column < 20; ++column)
        {
            const std::size_t belowLid = 18 * 20 + column;
            assert(velocity[belowLid].x == 0.0 && velocity[belowLid].y == 0.0);
        }
    }

    return 0;
}
