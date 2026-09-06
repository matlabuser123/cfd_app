#pragma once

#include "gpu/CUDAContext.hpp"

namespace cfd
{

class GPUBackend
{
public:
    explicit GPUBackend(GPUSettings settings = {});

    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] ComputeBackend requestedBackend() const noexcept;
    [[nodiscard]] ComputeBackend activeBackend() const noexcept;
    [[nodiscard]] bool usingCpuFallback() const noexcept;
    [[nodiscard]] const std::string& status() const noexcept;
    [[nodiscard]] const CUDAContext& cuda() const noexcept;

private:
    CUDAContext cuda_;
    ComputeBackend requestedBackend_;
    ComputeBackend activeBackend_;
    std::string status_;
};

}
