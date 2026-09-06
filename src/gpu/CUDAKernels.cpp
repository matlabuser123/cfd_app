#include "gpu/CUDAKernels.hpp"

#if !CFDAPP_HAS_CUDA

#include <stdexcept>

namespace cfd
{

bool CUDAKernels::compiled() noexcept
{
    return false;
}

void CUDAKernels::scale(double*, std::size_t, double)
{
    throw std::runtime_error("CUDA kernels were not compiled");
}

void CUDAKernels::multiplyCSR(
    const std::size_t*,
    const std::size_t*,
    const double*,
    const double*,
    double*,
    std::size_t
)
{
    throw std::runtime_error("CUDA kernels were not compiled");
}

}

#endif