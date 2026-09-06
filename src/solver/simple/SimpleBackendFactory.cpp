#include "solver/simple/SimpleBackendFactory.hpp"

#include "gpu/GPUBackend.hpp"
#include "solver/simple/CpuSimpleBackend.hpp"

namespace cfd
{

SimpleBackendFactory::Creation SimpleBackendFactory::create(
    const Mesh& mesh,
    double density,
    double viscosity,
    SIMPLESettings settings,
    ComputeBackend requestedBackend,
    GPUSettings gpuSettings
)
{
    Creation creation;
    creation.selection.requested = requestedBackend;

    if (requestedBackend != ComputeBackend::CUDA)
    {
        creation.backend = std::make_unique<CpuSimpleBackend>(mesh, density, viscosity, settings);
        creation.selection.active = requestedBackend;
        creation.selection.status = requestedBackend == ComputeBackend::OpenMP
            ? "OpenMP CPU backend selected"
            : "Serial CPU backend selected";
        return creation;
    }

    gpuSettings.requestedBackend = ComputeBackend::CUDA;
    const GPUBackend cudaBackend(gpuSettings);
    creation.backend = std::make_unique<CpuSimpleBackend>(mesh, density, viscosity, settings);
    creation.selection.active = ComputeBackend::Serial;
    creation.selection.status = cudaBackend.available()
        ? "CUDA device available, but CUDA SIMPLE coupling is not yet implemented (see TODO.md item #9); serial CPU backend selected"
        : "CUDA unavailable (" + cudaBackend.status() + "); serial CPU backend selected";
    return creation;
}

}
