#pragma once

#include <cstddef>

namespace cfd
{

class CUDAKernels
{
public:
    [[nodiscard]] static bool compiled() noexcept;
    static void scale(double* deviceValues, std::size_t count, double factor);
    static void multiplyCSR(
        const std::size_t* deviceRowOffsets,
        const std::size_t* deviceColumnIndices,
        const double* deviceMatrixValues,
        const double* deviceInput,
        double* deviceOutput,
        std::size_t rows
    );
};

}