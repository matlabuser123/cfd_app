#include "gpu/GPULinearAlgebra.hpp"

#if !CFDAPP_HAS_CUDA

#include "gpu/GPUBackend.hpp"

#include <stdexcept>

namespace cfd
{

std::vector<double> GPULinearAlgebra::multiply(
    const GPUCSRMatrix& matrix,
    const std::vector<double>& values,
    const GPUBackend& backend
)
{
    if (values.size() != matrix.columns) throw std::invalid_argument("GPU SpMV vector size does not match matrix columns");
    if (backend.activeBackend() != ComputeBackend::CUDA)
    {
        throw std::runtime_error("GPU linear algebra requires an active CUDA backend");
    }
    throw std::runtime_error("GPU linear algebra was not compiled");
}

}

#endif