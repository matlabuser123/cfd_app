#include "gpu/GPUBackend.hpp"

#include <stdexcept>

namespace cfd
{

GPUBackend::GPUBackend(GPUSettings settings)
    : cuda_(settings),
      requestedBackend_(settings.requestedBackend),
      activeBackend_(settings.requestedBackend)
{
    if (requestedBackend_ != ComputeBackend::CUDA)
    {
        status_ = requestedBackend_ == ComputeBackend::OpenMP
            ? "OpenMP CPU backend selected"
            : "Serial CPU backend selected";
        return;
    }

    if (cuda_.available())
    {
        status_ = "CUDA backend selected";
        return;
    }

    if (!settings.allowCpuFallback) throw std::runtime_error("CUDA backend requested but unavailable: " + cuda_.status());
    activeBackend_ = ComputeBackend::Serial;
    status_ = "CUDA unavailable; serial CPU fallback selected";
}

bool GPUBackend::available() const noexcept
{
    return cuda_.available();
}

ComputeBackend GPUBackend::requestedBackend() const noexcept
{
    return requestedBackend_;
}

ComputeBackend GPUBackend::activeBackend() const noexcept
{
    return activeBackend_;
}

bool GPUBackend::usingCpuFallback() const noexcept
{
    return requestedBackend_ == ComputeBackend::CUDA && activeBackend_ != ComputeBackend::CUDA;
}

const std::string& GPUBackend::status() const noexcept
{
    return status_;
}

const CUDAContext& GPUBackend::cuda() const noexcept
{
    return cuda_;
}

}
