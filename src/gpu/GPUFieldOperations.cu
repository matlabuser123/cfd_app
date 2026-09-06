#include "gpu/GPUFieldOperations.hpp"

#include "gpu/CUDAKernels.hpp"
#include "gpu/GPUBackend.hpp"

#include <cuda_runtime.h>

#include <stdexcept>
#include <string>

namespace cfd
{
namespace
{

void checkCUDA(cudaError_t error, const char* operation)
{
    if (error != cudaSuccess) throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(error));
}

}

void GPUFieldOperations::scale(std::vector<double>& values, double factor, const GPUBackend& backend)
{
    if (backend.activeBackend() != ComputeBackend::CUDA)
    {
        throw std::runtime_error("GPU field operations require an active CUDA backend");
    }
    if (values.empty()) return;

    double* deviceValues = nullptr;
    const std::size_t bytes = values.size() * sizeof(double);
    checkCUDA(cudaMalloc(&deviceValues, bytes), "CUDA field allocation failed");
    try
    {
        checkCUDA(cudaMemcpy(deviceValues, values.data(), bytes, cudaMemcpyHostToDevice), "CUDA field upload failed");
        CUDAKernels::scale(deviceValues, values.size(), factor);
        checkCUDA(cudaMemcpy(values.data(), deviceValues, bytes, cudaMemcpyDeviceToHost), "CUDA field download failed");
    }
    catch (...)
    {
        cudaFree(deviceValues);
        throw;
    }
    checkCUDA(cudaFree(deviceValues), "CUDA field release failed");
}

}