#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "gpu/GPUBackend.hpp"
#include "gpu/GPUFieldOperations.hpp"
#include "gpu/GPULinearAlgebra.hpp"
#include "gpu/GPUMatrixAssembly.hpp"
#include "gpu/CUDAKernels.hpp"
#include "numerics/matrix/SparseMatrix.hpp"

namespace
{

void assertEquivalent(const std::vector<double>& expected, const std::vector<double>& actual)
{
    assert(expected.size() == actual.size());
    for (std::size_t index = 0; index < expected.size(); ++index)
    {
        assert(std::abs(expected[index] - actual[index]) < 1e-12);
    }
}

}

int main()
{
#if CFDAPP_HAS_CUDA
    assert(cfd::CUDAKernels::compiled());
#else
    assert(!cfd::CUDAKernels::compiled());
    bool rejectedKernelLaunch = false;
    try
    {
        cfd::CUDAKernels::scale(nullptr, 1, 2.0);
    }
    catch (const std::runtime_error&)
    {
        rejectedKernelLaunch = true;
    }
    assert(rejectedKernelLaunch);
#endif

    cfd::GPUSettings serialSettings;
    serialSettings.requestedBackend = cfd::ComputeBackend::Serial;
    const cfd::GPUBackend serialBackend(serialSettings);
    assert(serialBackend.requestedBackend() == cfd::ComputeBackend::Serial);
    assert(serialBackend.activeBackend() == cfd::ComputeBackend::Serial);
    assert(!serialBackend.usingCpuFallback());
    assert(!serialBackend.status().empty());

    cfd::GPUSettings openMPSettings;
    openMPSettings.requestedBackend = cfd::ComputeBackend::OpenMP;
    const cfd::GPUBackend openMPBackend(openMPSettings);
    assert(openMPBackend.requestedBackend() == cfd::ComputeBackend::OpenMP);
    assert(openMPBackend.activeBackend() == cfd::ComputeBackend::OpenMP);
    assert(!openMPBackend.usingCpuFallback());

    const cfd::GPUBackend cudaBackend;
    assert(cudaBackend.requestedBackend() == cfd::ComputeBackend::CUDA);
    assert(!cudaBackend.cuda().status().empty());
    cfd::SparseMatrix hostMatrix(2, 2);
    hostMatrix.set(0, 0, 2.0);
    hostMatrix.set(0, 1, 1.0);
    hostMatrix.set(1, 0, 3.0);
    hostMatrix.set(1, 1, 4.0);
    const cfd::GPUCSRMatrix gpuMatrix = cfd::GPUMatrixAssembly::toCSR(hostMatrix);
    const std::vector<double> input{1.0, 2.0};
    bool rejectedMismatchedInput = false;
    try
    {
        static_cast<void>(cfd::GPULinearAlgebra::multiply(gpuMatrix, {1.0}, cudaBackend));
    }
    catch (const std::invalid_argument&)
    {
        rejectedMismatchedInput = true;
    }
    assert(rejectedMismatchedInput);
    if (cudaBackend.available())
    {
        assert(cudaBackend.activeBackend() == cfd::ComputeBackend::CUDA);
        assert(!cudaBackend.usingCpuFallback());
        std::vector<double> field{1.0, 2.0, 3.0};
        std::vector<double> cpuField = field;
        for (double& value : cpuField) value *= 2.0;
        cfd::GPUFieldOperations::scale(field, 2.0, cudaBackend);
        assertEquivalent(cpuField, field);
        cfd::GPUFieldOperations::scale(field, -0.5, cudaBackend);
        assertEquivalent({-1.0, -2.0, -3.0}, field);
        std::vector<double> emptyField;
        cfd::GPUFieldOperations::scale(emptyField, 2.0, cudaBackend);
        assert(emptyField.empty());
        assertEquivalent(hostMatrix.multiply(input), cfd::GPULinearAlgebra::multiply(gpuMatrix, input, cudaBackend));

        cfd::SparseMatrix irregularMatrix(3, 3);
        irregularMatrix.set(0, 0, 0.25);
        irregularMatrix.set(0, 2, -2.0);
        irregularMatrix.set(1, 1, 3.5);
        irregularMatrix.set(1, 2, 0.75);
        irregularMatrix.set(2, 0, -1.0);
        irregularMatrix.set(2, 1, 2.0);
        const std::vector<double> irregularInput{2.0, -1.0, 4.0};
        assertEquivalent(
            irregularMatrix.multiply(irregularInput),
            cfd::GPULinearAlgebra::multiply(cfd::GPUMatrixAssembly::toCSR(irregularMatrix), irregularInput, cudaBackend)
        );

        cfd::SparseMatrix zeroMatrix(2, 2);
        assertEquivalent({0.0, 0.0}, cfd::GPULinearAlgebra::multiply(cfd::GPUMatrixAssembly::toCSR(zeroMatrix), input, cudaBackend));
    }
    else
    {
        assert(cudaBackend.activeBackend() == cfd::ComputeBackend::Serial);
        assert(cudaBackend.usingCpuFallback());

        std::vector<double> field{1.0, 2.0, 3.0};
        bool rejectedFieldOperation = false;
        try
        {
            cfd::GPUFieldOperations::scale(field, 2.0, cudaBackend);
        }
        catch (const std::runtime_error&)
        {
            rejectedFieldOperation = true;
        }
        assert(rejectedFieldOperation);
        assert(field == std::vector<double>({1.0, 2.0, 3.0}));

        bool rejectedMatrixOperation = false;
        try
        {
            static_cast<void>(cfd::GPULinearAlgebra::multiply(gpuMatrix, input, cudaBackend));
        }
        catch (const std::runtime_error&)
        {
            rejectedMatrixOperation = true;
        }
        assert(rejectedMatrixOperation);

        cfd::GPUSettings requiredCUDA;
        requiredCUDA.allowCpuFallback = false;
        bool rejectedUnavailableCUDA = false;
        try
        {
            static_cast<void>(cfd::GPUBackend(requiredCUDA));
        }
        catch (const std::runtime_error&)
        {
            rejectedUnavailableCUDA = true;
        }
        assert(rejectedUnavailableCUDA);
    }
    return 0;
}
