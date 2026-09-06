#include "gpu/GPUMatrixAssembly.hpp"

#include "numerics/matrix/SparseMatrix.hpp"

#include <algorithm>
#include <utility>

namespace cfd
{

GPUCSRMatrix GPUMatrixAssembly::toCSR(const SparseMatrix& matrix)
{
    GPUCSRMatrix csr;
    csr.rows = matrix.rows();
    csr.columns = matrix.cols();
    csr.rowOffsets.reserve(csr.rows + 1);
    csr.rowOffsets.push_back(0);

    for (std::size_t row = 0; row < csr.rows; ++row)
    {
        std::vector<std::pair<std::size_t, double>> entries;
        entries.reserve(matrix.rowEntries(row).size());
        for (const auto& [column, value] : matrix.rowEntries(row)) entries.emplace_back(column, value);
        std::sort(entries.begin(), entries.end(), [](const auto& left, const auto& right) { return left.first < right.first; });
        for (const auto& [column, value] : entries)
        {
            csr.columnIndices.push_back(column);
            csr.values.push_back(value);
        }
        csr.rowOffsets.push_back(csr.values.size());
    }
    return csr;
}

}