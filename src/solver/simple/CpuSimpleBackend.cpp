#include "solver/simple/CpuSimpleBackend.hpp"

namespace cfd
{

CpuSimpleBackend::CpuSimpleBackend(const Mesh& mesh, double density, double viscosity, SIMPLESettings settings)
    : simple_(mesh, density, viscosity, settings)
{
}

void CpuSimpleBackend::setBoundaryConditions(std::vector<const BoundaryCondition*> conditions)
{
    simple_.setBoundaryConditions(std::move(conditions));
}

void CpuSimpleBackend::setProfiler(Profiler* profiler) noexcept
{
    simple_.setProfiler(profiler);
}

SIMPLEResult CpuSimpleBackend::solve(
    VectorField& velocity,
    ScalarField& pressure,
    const std::atomic_bool* cancelRequested,
    std::function<void(const SIMPLEIteration&)> iterationCallback
)
{
    return simple_.solve(velocity, pressure, cancelRequested, std::move(iterationCallback));
}

}
