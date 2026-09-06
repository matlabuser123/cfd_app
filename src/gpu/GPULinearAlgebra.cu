#include "gpu/GPULinearAlgebra.hpp"

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

void validateCSR(const GPUCSRMatrix& matrix, const std::vector<double>& input)
{
    if (input.size() != matrix.columns) throw std::invalid_argument("GPU SpMV vector size does not match matrix columns");
    if (matrix.rowOffsets.size() != matrix.rows + 1 || matrix.columnIndices.size() != matrix.values.size())
    {
        throw std::invalid_argument("GPU CSR matrix storage is inconsistent");
    }
    if (matrix.rowOffsets.empty() || matrix.rowOffsets.front() != 0 || matrix.rowOffsets.back() != matrix.values.size())
    {
        throw std::invalid_argument("GPU CSR matrix row offsets are invalid");
    }
    for (std::size_t row = 0; row < matrix.rows; ++row)
    {
        if (matrix.rowOffsets[row] > matrix.rowOffsets[row + 1]) throw std::invalid_argument("GPU CSR row offsets must be ordered");
    }
    for (const std::size_t column : matrix.columnIndices)
    {
        if (column >= matrix.columns) throw std::invalid_argument("GPU CSR column index is out of range");
    }
}

}

std::vector<double> GPULinearAlgebra::multiply(
    const GPUCSRMatrix& matrix,
    const std::vector<double>& values,
    const GPUBackend& backend
)
{
    validateCSR(matrix, values);
    if (backend.activeBackend() != ComputeBackend::CUDA)
    {
        throw std::runtime_error("GPU linear algebra requires an active CUDA backend");
    }

    std::vector<double> result(matrix.rows, 0.0);
    if (matrix.rows == 0 || matrix.values.empty()) return result;

    std::size_t* deviceRowOffsets = nullptr;
    std::size_t* deviceColumnIndices = nullptr;
    double* deviceMatrixValues = nullptr;
    double* deviceInput = nullptr;
    double* deviceOutput = nullptr;
    try
    {
        checkCUDA(cudaMalloc(&deviceRowOffsets, matrix.rowOffsets.size() * sizeof(std::size_t)), "CUDA CSR row-offset allocation failed");
        checkCUDA(cudaMalloc(&deviceColumnIndices, matrix.columnIndices.size() * sizeof(std::size_t)), "CUDA CSR column-index allocation failed");
        checkCUDA(cudaMalloc(&deviceMatrixValues, matrix.values.size() * sizeof(double)), "CUDA CSR value allocation failed");
        checkCUDA(cudaMalloc(&deviceInput, values.size() * sizeof(double)), "CUDA CSR input allocation failed");
        checkCUDA(cudaMalloc(&deviceOutput, result.size() * sizeof(double)), "CUDA CSR output allocation failed");
        checkCUDA(cudaMemcpy(deviceRowOffsets, matrix.rowOffsets.data(), matrix.rowOffsets.size() * sizeof(std::size_t), cudaMemcpyHostToDevice), "CUDA CSR row-offset upload failed");
        checkCUDA(cudaMemcpy(deviceColumnIndices, matrix.columnIndices.data(), matrix.columnIndices.size() * sizeof(std::size_t), cudaMemcpyHostToDevice), "CUDA CSR column-index upload failed");
        checkCUDA(cudaMemcpy(deviceMatrixValues, matrix.values.data(), matrix.values.size() * sizeof(double), cudaMemcpyHostToDevice), "CUDA CSR value upload failed");
        checkCUDA(cudaMemcpy(deviceInput, values.data(), values.size() * sizeof(double), cudaMemcpyHostToDevice), "CUDA CSR input upload failed");
        CUDAKernels::multiplyCSR(deviceRowOffsets, deviceColumnIndices, deviceMatrixValues, deviceInput, deviceOutput, matrix.rows);
        checkCUDA(cudaMemcpy(result.data(), deviceOutput, result.size() * sizeof(double), cudaMemcpyDeviceToHost), "CUDA CSR output download failed");
    }
    catch (...)
    {
        cudaFree(deviceOutput);
        cudaFree(deviceInput);
        cudaFree(deviceMatrixValues);
        cudaFree(deviceColumnIndices);
        cudaFree(deviceRowOffsets);
        throw;
    }

    checkCUDA(cudaFree(deviceOutput), "CUDA CSR output release failed");
    checkCUDA(cudaFree(deviceInput), "CUDA CSR input release failed");
    checkCUDA(cudaFree(deviceMatrixValues), "CUDA CSR value release failed");
    checkCUDA(cudaFree(deviceColumnIndices), "CUDA CSR column-index release failed");
    checkCUDA(cudaFree(deviceRowOffsets), "CUDA CSR row-offset release failed");
    return result;
}

}