#pragma once

#include <cstddef>
#include <vector>

namespace cfd
{

class SparseMatrix;

struct GPUCSRMatrix
{
    std::size_t rows{0};
    std::size_t columns{0};
    std::vector<std::size_t> rowOffsets;
    std::vector<std::size_t> columnIndices;
    std::vector<double> values;
};

class GPUMatrixAssembly
{
public:
    [[nodiscard]] static GPUCSRMatrix toCSR(const SparseMatrix& matrix);
};

}