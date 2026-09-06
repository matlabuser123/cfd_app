#include "parallel/ParallelSparseMatrix.hpp"

#include "parallel/ParallelFor.hpp"

#include <stdexcept>

namespace cfd
{

std::vector<double> parallelMultiply(
    const SparseMatrix& matrix,
    const std::vector<double>& values,
    const ParallelSettings& settings
)
{
    if (values.size() != matrix.cols()) throw std::invalid_argument("Parallel SpMV vector size does not match matrix columns");
    ParallelRuntime::initialize(settings);
    std::vector<double> result(matrix.rows(), 0.0);
    parallelFor(0, matrix.rows(), [&](std::size_t row)
    {
        for (const auto& [column, value] : matrix.rowEntries(row)) result[row] += value * values[column];
    });
    return result;
}

}
