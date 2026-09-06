#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <vector>

#include "core/mesh/Mesh.hpp"
#include "gpu/GPUBackend.hpp"
#include "gpu/GPULinearAlgebra.hpp"
#include "gpu/GPUMatrixAssembly.hpp"
#include "numerics/matrix/SparseMatrix.hpp"

namespace
{

constexpr std::size_t samples = 10;
constexpr int skipped = 77;

bool equivalent(const std::vector<double>& expected, const std::vector<double>& actual)
{
    if (expected.size() != actual.size()) return false;
    for (std::size_t index = 0; index < expected.size(); ++index)
    {
        if (std::abs(expected[index] - actual[index]) > 1e-12) return false;
    }
    return true;
}

template <typename Function>
double medianMilliseconds(Function&& function)
{
    std::vector<double> durations;
    durations.reserve(samples);
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        const auto start = std::chrono::steady_clock::now();
        static_cast<void>(function());
        const auto end = std::chrono::steady_clock::now();
        durations.push_back(std::chrono::duration<double, std::milli>(end - start).count());
    }
    std::sort(durations.begin(), durations.end());
    return durations[durations.size() / 2];
}

}

int main()
{
    const cfd::GPUBackend backend;
    if (backend.activeBackend() != cfd::ComputeBackend::CUDA)
    {
        std::cout << "GPU SpMV benchmark skipped: " << backend.status() << '\n';
        return skipped;
    }

    const std::array<std::size_t, 3> gridSizes{20, 40, 80};
    std::cout << "CPU/GPU SpMV scaling: samples=" << samples << '\n';
    for (const std::size_t gridSize : gridSizes)
    {
        cfd::Mesh mesh;
        mesh.generateUniform(gridSize, gridSize);
        cfd::SparseMatrix hostMatrix(mesh.cellCount(), mesh.cellCount());
        for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell)
        {
            hostMatrix.set(cell, cell, 4.0);
            if (const auto east = mesh.east(cell)) hostMatrix.set(cell, *east, -1.0);
            if (const auto west = mesh.west(cell)) hostMatrix.set(cell, *west, -1.0);
            if (const auto north = mesh.north(cell)) hostMatrix.set(cell, *north, -1.0);
            if (const auto south = mesh.south(cell)) hostMatrix.set(cell, *south, -1.0);
        }
        const cfd::GPUCSRMatrix gpuMatrix = cfd::GPUMatrixAssembly::toCSR(hostMatrix);
        std::vector<double> input(mesh.cellCount());
        for (std::size_t index = 0; index < input.size(); ++index)
        {
            input[index] = static_cast<double>(index % mesh.nx()) / static_cast<double>(mesh.nx());
        }

        const std::vector<double> cpuResult = hostMatrix.multiply(input);
        const std::vector<double> gpuResult = cfd::GPULinearAlgebra::multiply(gpuMatrix, input, backend);
        if (!equivalent(cpuResult, gpuResult))
        {
            std::cerr << "GPU SpMV benchmark failed CPU/GPU equivalence check at " << gridSize << "x" << gridSize << '\n';
            return 1;
        }

        const double cpuMilliseconds = medianMilliseconds([&] { return hostMatrix.multiply(input); });
        const double gpuMilliseconds = medianMilliseconds([&] { return cfd::GPULinearAlgebra::multiply(gpuMatrix, input, backend); });
        std::cout << "  grid=" << gridSize << "x" << gridSize
                  << " rows=" << gpuMatrix.rows
                  << " nonzeros=" << gpuMatrix.values.size()
                  << " cpu_median_ms=" << cpuMilliseconds
                  << " gpu_median_ms=" << gpuMilliseconds
                  << " speedup=" << cpuMilliseconds / gpuMilliseconds
                  << " equivalent=true\n";
    }
    return 0;
}