#include <cassert>
#include <cmath>
#include <vector>

#include "numerics/matrix/SparseMatrix.hpp"
#include "optimization/OptimizationRunner.hpp"

int main()
{
    cfd::SparseMatrix matrix(3, 3);
    matrix.set(0, 0, 4.0);
    matrix.set(0, 1, 1.0);
    matrix.set(1, 1, 5.0);
    matrix.set(1, 2, 2.0);
    matrix.set(2, 0, 3.0);
    matrix.set(2, 2, 6.0);

    const std::vector<double> input{1.0, 2.0, 3.0};
    const std::vector<double> expected = matrix.multiply(input);
    std::vector<double> reusable(3, 0.0);
    matrix.multiplyInto(input, reusable);
    assert(reusable == expected);

    std::vector<double> optimizedResult;
    const auto baseline = [&] { static_cast<void>(matrix.multiply(input)); };
    const auto optimized = [&] { matrix.multiplyInto(input, optimizedResult); };
    const cfd::OptimizationResult result = cfd::OptimizationRunner::compare(
        "spmv_3x3", 3, 3, baseline, optimized,
        [&](double tolerance)
        {
            if (optimizedResult.size() != expected.size()) return false;
            for (std::size_t index = 0; index < expected.size(); ++index)
            {
                if (std::abs(optimizedResult[index] - expected[index]) > tolerance) return false;
            }
            return true;
        },
        {100, 1e-12}
    );
    assert(result.solutionPreserved);
    assert(result.baselineMilliseconds >= 0.0);
    assert(result.optimizedMilliseconds >= 0.0);
    return 0;
}
