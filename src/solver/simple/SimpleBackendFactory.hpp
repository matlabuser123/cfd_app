#pragma once

#include "gpu/GPUSettings.hpp"
#include "solver/simple/SIMPLESettings.hpp"
#include "solver/simple/SimpleBackend.hpp"

#include <memory>
#include <string>

namespace cfd
{

class Mesh;

// Reports what was actually selected, distinct from what was requested — the
// same shape as CFDController's SolverBackendSelection, but usable from
// solver-level code without depending on the gui module.
struct SimpleBackendSelection
{
    ComputeBackend requested{ComputeBackend::Serial};
    ComputeBackend active{ComputeBackend::Serial};
    std::string status;
};

// Builds a SimpleBackend for a requested ComputeBackend.
//
// Serial and OpenMP both construct a CpuSimpleBackend (they differ only in
// ParallelRuntime configuration, which the caller is responsible for having
// set — this factory does not touch ParallelRuntime).
//
// CUDA does NOT yet have a SimpleBackend implementation (see TODO.md item #9
// — full CUDA SIMPLE coupling is scoped as post-v0.1.0 work, and must not be
// implemented without access to CUDA hardware to verify CPU/GPU numerical
// equivalence). Requesting CUDA here honestly falls back to CpuSimpleBackend,
// same as CFDController::configureBackend does today, but the returned
// selection.status distinguishes "CUDA hardware available, but CUDA SIMPLE
// coupling isn't implemented yet" from "no CUDA hardware available" — the
// existing GUI status string conflates the two.
class SimpleBackendFactory
{
public:
    struct Creation
    {
        std::unique_ptr<SimpleBackend> backend;
        SimpleBackendSelection selection;
    };

    [[nodiscard]] static Creation create(
        const Mesh& mesh,
        double density,
        double viscosity,
        SIMPLESettings settings,
        ComputeBackend requestedBackend,
        GPUSettings gpuSettings = {}
    );
};

}
