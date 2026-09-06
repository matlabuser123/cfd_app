#include "gpu/CUDAContext.hpp"

#if CFDAPP_HAS_CUDA
#include <cuda_runtime_api.h>
#endif

namespace cfd
{

CUDAContext::CUDAContext(GPUSettings settings)
{
#if CFDAPP_HAS_CUDA
    int deviceCount = 0;
    const cudaError_t error = cudaGetDeviceCount(&deviceCount);
    available_ = error == cudaSuccess && settings.deviceIndex >= 0 && settings.deviceIndex < deviceCount;
    status_ = available_ ? "CUDA device available" : "CUDA device unavailable";
#else
    static_cast<void>(settings);
    status_ = "CUDA support was not compiled";
#endif
}

bool CUDAContext::available() const noexcept
{
    return available_;
}

const std::string& CUDAContext::status() const noexcept
{
    return status_;
}

}
