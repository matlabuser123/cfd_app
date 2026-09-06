#pragma once

#include <cstddef>
#include <string>

namespace cfd
{

struct OptimizationResult
{
    std::string caseName;
    std::size_t nx{0};
    std::size_t ny{0};
    double baselineMilliseconds{0.0};
    double optimizedMilliseconds{0.0};
    double speedup{1.0};
    double improvementPercent{0.0};
    bool solutionPreserved{false};
};

}
