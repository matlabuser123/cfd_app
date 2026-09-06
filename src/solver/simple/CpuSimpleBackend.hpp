#pragma once

#include "solver/simple/SimpleBackend.hpp"
#include "solver/simple/SIMPLE.hpp"

namespace cfd
{

// The CPU (Serial/OpenMP) implementation of SimpleBackend. This is a thin
// adapter over SIMPLE — the numerical reference implementation is unchanged;
// this class only exists so SIMPLE can be selected through the same
// SimpleBackend interface a future CUDA implementation would use.
class CpuSimpleBackend final : public SimpleBackend
{
public:
    CpuSimpleBackend(const Mesh& mesh, double density, double viscosity, SIMPLESettings settings = {});

    void setBoundaryConditions(std::vector<const BoundaryCondition*> conditions) override;
    void setProfiler(Profiler* profiler) noexcept override;
    [[nodiscard]] SIMPLEResult solve(
        VectorField& velocity,
        ScalarField& pressure,
        const std::atomic_bool* cancelRequested = nullptr,
        std::function<void(const SIMPLEIteration&)> iterationCallback = {}
    ) override;

private:
    SIMPLE simple_;
};

}
