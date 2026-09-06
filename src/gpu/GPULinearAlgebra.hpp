#pragma once

#include "gpu/GPUMatrixAssembly.hpp"

#include <vector>

namespace cfd
{

class GPUBackend;

class GPULinearAlgebra
{
public:
    [[nodiscard]] static std::vector<double> multiply(
        const GPUCSRMatrix& matrix,
        const std::vector<double>& values,
        const GPUBackend& backend
    );
};

}