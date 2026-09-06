#pragma once

#include "solver/simple/SIMPLEResult.hpp"

#include <atomic>
#include <functional>
#include <vector>

namespace cfd
{

class BoundaryCondition;
class Profiler;
class ScalarField;
class VectorField;

// Executes the SIMPLE pressure-velocity coupling loop for one compute backend.
//
// CpuSimpleBackend (used for both the Serial and OpenMP ComputeBackend values —
// they differ only in ParallelRuntime configuration, not in solver code) is the
// numerical reference implementation. Any future backend (e.g. a CUDA
// implementation) must produce results equivalent to the CPU reference within
// the project's documented tolerances, and pass the full Backend Completion
// Gate, before it is considered a supported SIMPLE solver backend rather than
// an acceleration primitive. See TODO.md item #9 and roadmap.md's
// "CUDA / GPU Acceleration" section for the full record.
class SimpleBackend
{
public:
    virtual ~SimpleBackend() = default;

    virtual void setBoundaryConditions(std::vector<const BoundaryCondition*> conditions) = 0;
    virtual void setProfiler(Profiler* profiler) noexcept = 0;
    [[nodiscard]] virtual SIMPLEResult solve(
        VectorField& velocity,
        ScalarField& pressure,
        const std::atomic_bool* cancelRequested = nullptr,
        std::function<void(const SIMPLEIteration&)> iterationCallback = {}
    ) = 0;
};

}
