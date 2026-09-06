#pragma once

#include "numerics/matrix/SparseMatrix.hpp"
#include "parallel/ParallelSettings.hpp"

#include <vector>

namespace cfd
{

[[nodiscard]] std::vector<double> parallelMultiply(
    const SparseMatrix& matrix,
    const std::vector<double>& values,
    const ParallelSettings& settings = {}
);

}
