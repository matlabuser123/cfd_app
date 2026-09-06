#include "parallel/ParallelReduction.hpp"

#include <stdexcept>

#if CFDAPP_HAS_OPENMP
#include <omp.h>
#endif

namespace cfd
{

double parallelSum(const std::vector<double>& values)
{
    double sum = 0.0;
#if CFDAPP_HAS_OPENMP
    if (ParallelRuntime::enabled())
    {
#pragma omp parallel for reduction(+:sum)
        for (long long index = 0; index < static_cast<long long>(values.size()); ++index) sum += values[static_cast<std::size_t>(index)];
        return sum;
    }
#endif
    for (double value : values) sum += value;
    return sum;
}

double parallelDot(const std::vector<double>& left, const std::vector<double>& right)
{
    if (left.size() != right.size()) throw std::invalid_argument("Parallel dot vectors must have equal sizes");
    double result = 0.0;
#if CFDAPP_HAS_OPENMP
    if (ParallelRuntime::enabled())
    {
#pragma omp parallel for reduction(+:result)
        for (long long index = 0; index < static_cast<long long>(left.size()); ++index) result += left[static_cast<std::size_t>(index)] * right[static_cast<std::size_t>(index)];
        return result;
    }
#endif
    for (std::size_t index = 0; index < left.size(); ++index) result += left[index] * right[index];
    return result;
}

}
