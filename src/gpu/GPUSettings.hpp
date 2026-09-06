#pragma once

namespace cfd
{

enum class ComputeBackend
{
    Serial,
    OpenMP,
    CUDA
};

struct GPUSettings
{
    ComputeBackend requestedBackend{ComputeBackend::CUDA};
    bool allowCpuFallback{true};
    int deviceIndex{0};
};

}
