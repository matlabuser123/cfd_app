#include <cassert>
#include <cmath>
#include <vector>

#include "numerics/matrix/SparseMatrix.hpp"
#include "parallel/ParallelReduction.hpp"
#include "parallel/ParallelRuntime.hpp"
#include "parallel/ParallelSettings.hpp"
#include "parallel/ParallelSparseMatrix.hpp"

int main()
{
    cfd::SparseMatrix matrix(4, 4);
    for (std::size_t row = 0; row < 4; ++row)
    {
        matrix.set(row, row, 2.0);
        if (row > 0) matrix.set(row, row - 1, -1.0);
        if (row + 1 < 4) matrix.set(row, row + 1, -1.0);
    }
    const std::vector<double> values{1.0, 2.0, 3.0, 4.0};
    const auto serial = matrix.multiply(values);
    const auto parallel = cfd::parallelMultiply(matrix, values, {true, 2});
    assert(serial.size() == parallel.size());
    for (std::size_t index = 0; index < serial.size(); ++index) assert(std::abs(serial[index] - parallel[index]) < 1e-12);

    const std::vector<double> reductionValues(1000, 1.0);
    cfd::ParallelRuntime::initialize({true, 2});
    assert(std::abs(cfd::parallelSum(reductionValues) - 1000.0) < 1e-12);
    assert(std::abs(cfd::parallelDot(reductionValues, reductionValues) - 1000.0) < 1e-12);
    assert(cfd::ParallelRuntime::threadCount() >= 1);
    return 0;
}
