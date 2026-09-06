#include "optimization/OptimizationRunner.hpp"

#include <chrono>
#include <stdexcept>

namespace cfd
{

double OptimizationRunner::measure(
    const std::function<void()>& workload,
    std::size_t repetitions
)
{
    if (repetitions == 0) throw std::invalid_argument("Optimization repetitions must be positive");
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t repetition = 0; repetition < repetitions; ++repetition) workload();
    const auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

OptimizationResult OptimizationRunner::compare(
    const std::string& caseName,
    std::size_t nx,
    std::size_t ny,
    const std::function<void()>& baseline,
    const std::function<void()>& optimized,
    const std::function<bool(double)>& solutionsEqual,
    OptimizationSettings settings
)
{
    OptimizationResult result;
    result.caseName = caseName;
    result.nx = nx;
    result.ny = ny;
    result.baselineMilliseconds = measure(baseline, settings.repetitions);
    result.optimizedMilliseconds = measure(optimized, settings.repetitions);
    result.speedup = result.optimizedMilliseconds == 0.0 ? 0.0 : result.baselineMilliseconds / result.optimizedMilliseconds;
    result.improvementPercent = result.baselineMilliseconds == 0.0 ? 0.0 : (result.baselineMilliseconds - result.optimizedMilliseconds) / result.baselineMilliseconds * 100.0;
    result.solutionPreserved = solutionsEqual(settings.solutionTolerance);
    return result;
}

}
