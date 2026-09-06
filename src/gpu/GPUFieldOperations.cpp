#include "gpu/GPUFieldOperations.hpp"

#if !CFDAPP_HAS_CUDA

#include "gpu/GPUBackend.hpp"

#include <stdexcept>

namespace cfd
{

void GPUFieldOperations::scale(std::vector<double>&, double, const GPUBackend& backend)
{
    if (backend.activeBackend() != ComputeBackend::CUDA)
    {
        throw std::runtime_error("GPU field operations require an active CUDA backend");
    }
    throw std::runtime_error("GPU field operations were not compiled");
}

}

#endif