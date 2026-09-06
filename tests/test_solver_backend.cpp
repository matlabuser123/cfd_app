#include <cassert>
#include <cmath>
#include <memory>
#include <vector>

#include "core/boundary/DirichletBC.hpp"
#include "core/boundary/WallBC.hpp"
#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "gpu/GPUSettings.hpp"
#include "parallel/ParallelRuntime.hpp"
#include "profiling/Profiler.hpp"
#include "solver/simple/SIMPLE.hpp"
#include "solver/simple/SimpleBackendFactory.hpp"

// Covers the SimpleBackend seam introduced for TODO.md item #9 (full CUDA
// SIMPLE coupling): proves the CpuSimpleBackend adapter is behavior-preserving
// relative to using SIMPLE directly, and proves a CUDA request honestly falls
// back to a fully-functional CPU solve rather than silently doing nothing or
// claiming an unimplemented GPU path ran.

namespace
{

cfd::Mesh makeCavityMesh()
{
    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);
    return mesh;
}

struct CavityBoundaries
{
    std::vector<std::size_t> top{380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399};
    std::vector<std::size_t> bottom{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<std::size_t> left;
    std::vector<std::size_t> right;

    CavityBoundaries()
    {
        for (std::size_t row = 0; row < 20; ++row)
        {
            left.push_back(row * 20);
            right.push_back(row * 20 + 19);
        }
    }
};

constexpr cfd::SIMPLESettings kSettings{200, 10.0, 10.0, 100.0, 0.7, 0.3};

void assertFieldsMatch(const cfd::VectorField& a, const cfd::ScalarField& pa, const cfd::VectorField& b, const cfd::ScalarField& pb, double tolerance)
{
    assert(a.size() == b.size());
    for (std::size_t cell = 0; cell < a.size(); ++cell)
    {
        assert(std::isfinite(b[cell].x));
        assert(std::isfinite(b[cell].y));
        assert(std::isfinite(pb[cell]));
        assert(std::abs(a[cell].x - b[cell].x) < tolerance);
        assert(std::abs(a[cell].y - b[cell].y) < tolerance);
        assert(std::abs(pa[cell] - pb[cell]) < tolerance);
    }
}

}

int main()
{
    const cfd::Mesh mesh = makeCavityMesh();
    const CavityBoundaries boundaries;
    cfd::DirichletBC lid("top", boundaries.top, cfd::Vector2{1.0, 0.0});
    cfd::WallBC bottomWall("bottom", boundaries.bottom);
    cfd::WallBC leftWall("left", boundaries.left);
    cfd::WallBC rightWall("right", boundaries.right);
    const std::vector<const cfd::BoundaryCondition*> conditions{&lid, &bottomWall, &leftWall, &rightWall};

    // Reference: SIMPLE used directly, exactly as every existing call site does today.
    cfd::ParallelRuntime::initialize({false, 1});
    cfd::VectorField referenceVelocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField referencePressure(mesh.cellCount(), 0.0);
    cfd::SIMPLE referenceSimple(mesh, 1.0, 0.01, kSettings);
    referenceSimple.setBoundaryConditions(conditions);
    const cfd::SIMPLEResult referenceResult = referenceSimple.solve(referenceVelocity, referencePressure);
    assert(referenceResult.converged);

    // 1. Serial request through the factory must be bit-identical to the reference:
    //    CpuSimpleBackend must be a behavior-preserving adapter, not a reimplementation.
    {
        cfd::ParallelRuntime::initialize({false, 1});
        auto creation = cfd::SimpleBackendFactory::create(mesh, 1.0, 0.01, kSettings, cfd::ComputeBackend::Serial);
        assert(creation.backend != nullptr);
        assert(creation.selection.requested == cfd::ComputeBackend::Serial);
        assert(creation.selection.active == cfd::ComputeBackend::Serial);
        assert(!creation.selection.status.empty());

        cfd::VectorField velocity(mesh.cellCount(), {0.0, 0.0});
        cfd::ScalarField pressure(mesh.cellCount(), 0.0);
        creation.backend->setBoundaryConditions(conditions);
        const cfd::SIMPLEResult result = creation.backend->solve(velocity, pressure);
        assert(result.converged);
        assert(result.iterations == referenceResult.iterations);
        assert(std::abs(result.continuityResidual - referenceResult.continuityResidual) < 1e-12);
        assertFieldsMatch(referenceVelocity, referencePressure, velocity, pressure, 1e-12);
    }

    // 2. OpenMP request must select ComputeBackend::OpenMP and still match the
    //    serial reference within the existing parallel-equivalence tolerance
    //    (test_parallel.cpp / test_simple.cpp already cover this in depth; this
    //    only proves the factory/adapter route it correctly).
    {
        cfd::ParallelRuntime::initialize({true, 4});
        auto creation = cfd::SimpleBackendFactory::create(mesh, 1.0, 0.01, kSettings, cfd::ComputeBackend::OpenMP);
        assert(creation.selection.requested == cfd::ComputeBackend::OpenMP);
        assert(creation.selection.active == cfd::ComputeBackend::OpenMP);

        cfd::VectorField velocity(mesh.cellCount(), {0.0, 0.0});
        cfd::ScalarField pressure(mesh.cellCount(), 0.0);
        creation.backend->setBoundaryConditions(conditions);
        const cfd::SIMPLEResult result = creation.backend->solve(velocity, pressure);
        assert(result.converged);
        assert(result.iterations == referenceResult.iterations);
        assertFieldsMatch(referenceVelocity, referencePressure, velocity, pressure, 1e-12);
        cfd::ParallelRuntime::initialize({false, 1});
    }

    // 3. CUDA request: there is no CudaSimpleBackend yet (TODO.md item #9 is not
    //    started). The factory must NOT claim CUDA is active, and must still
    //    hand back a fully-functional, numerically-correct CPU solve — never a
    //    silent no-op or a partially-wired stub.
    {
        auto creation = cfd::SimpleBackendFactory::create(mesh, 1.0, 0.01, kSettings, cfd::ComputeBackend::CUDA);
        assert(creation.backend != nullptr);
        assert(creation.selection.requested == cfd::ComputeBackend::CUDA);
        assert(creation.selection.active == cfd::ComputeBackend::Serial);
        assert(!creation.selection.status.empty());

        cfd::VectorField velocity(mesh.cellCount(), {0.0, 0.0});
        cfd::ScalarField pressure(mesh.cellCount(), 0.0);
        cfd::Profiler profiler;
        creation.backend->setBoundaryConditions(conditions);
        creation.backend->setProfiler(&profiler);
        const cfd::SIMPLEResult result = creation.backend->solve(velocity, pressure);
        assert(result.converged);
        assertFieldsMatch(referenceVelocity, referencePressure, velocity, pressure, 1e-12);
        // setProfiler must have actually been forwarded to the underlying SIMPLE
        // instance, not silently dropped by the adapter.
        assert(profiler.entries().contains("simple_iteration"));
    }

    return 0;
}
