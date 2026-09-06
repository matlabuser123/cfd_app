#pragma once

#include "optimization/OptimizationResult.hpp"
#include "optimization/OptimizationSettings.hpp"

#include <cstddef>
#include <functional>
#include <string>

namespace cfd
{

class OptimizationRunner
{
public:
    [[nodiscard]] static double measure(
        const std::function<void()>& workload,
        std::size_t repetitions
    );

    [[nodiscard]] static OptimizationResult compare(
        const std::string& caseName,
        std::size_t nx,
        std::size_t ny,
        const std::function<void()>& baseline,
        const std::function<void()>& optimized,
        const std::function<bool(double)>& solutionsEqual,
        OptimizationSettings settings = {}
    );
};

}
