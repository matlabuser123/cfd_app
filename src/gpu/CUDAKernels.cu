#include "gpu/CUDAKernels.hpp"

#include <cuda_runtime.h>

#include <stdexcept>
#include <string>

namespace cfd
{
namespace
{

constexpr unsigned int threadsPerBlock = 256;

__global__ void scaleKernel(double* values, std::size_t count, double factor)
{
    const std::size_t index = static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (index < count) values[index] *= factor;
}

__global__ void csrMultiplyKernel(
    const std::size_t* rowOffsets,
    const std::size_t* columnIndices,
    const double* matrixValues,
    const double* input,
    double* output,
    std::size_t rows
)
{
    const std::size_t row = static_cast<std::size_t>(blockIdx.x) * blockDim.x + threadIdx.x;
    if (row >= rows) return;

    double value = 0.0;
    for (std::size_t index = rowOffsets[row]; index < rowOffsets[row + 1]; ++index)
    {
        value += matrixValues[index] * input[columnIndices[index]];
    }
    output[row] = value;
}

void checkCUDA(cudaError_t error, const char* operation)
{
    if (error != cudaSuccess) throw std::runtime_error(std::string(operation) + ": " + cudaGetErrorString(error));
}

}

bool CUDAKernels::compiled() noexcept
{
    return true;
}

void CUDAKernels::scale(double* deviceValues, std::size_t count, double factor)
{
    if (count == 0) return;
    if (deviceValues == nullptr) throw std::invalid_argument("CUDA scale kernel requires device memory");

    const unsigned int blocks = static_cast<unsigned int>((count + threadsPerBlock - 1) / threadsPerBlock);
    scaleKernel<<<blocks, threadsPerBlock>>>(deviceValues, count, factor);
    checkCUDA(cudaGetLastError(), "CUDA scale kernel launch failed");
    checkCUDA(cudaDeviceSynchronize(), "CUDA scale kernel execution failed");
}

void CUDAKernels::multiplyCSR(
    const std::size_t* deviceRowOffsets,
    const std::size_t* deviceColumnIndices,
    const double* deviceMatrixValues,
    const double* deviceInput,
    double* deviceOutput,
    std::size_t rows
)
{
    if (rows == 0) return;
    if (deviceRowOffsets == nullptr || deviceColumnIndices == nullptr || deviceMatrixValues == nullptr || deviceInput == nullptr || deviceOutput == nullptr)
    {
        throw std::invalid_argument("CUDA CSR multiply requires device memory");
    }

    const unsigned int blocks = static_cast<unsigned int>((rows + threadsPerBlock - 1) / threadsPerBlock);
    csrMultiplyKernel<<<blocks, threadsPerBlock>>>(deviceRowOffsets, deviceColumnIndices, deviceMatrixValues, deviceInput, deviceOutput, rows);
    checkCUDA(cudaGetLastError(), "CUDA CSR multiply kernel launch failed");
    checkCUDA(cudaDeviceSynchronize(), "CUDA CSR multiply kernel execution failed");
}

}