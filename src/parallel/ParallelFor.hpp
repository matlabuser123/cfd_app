#pragma once

#include "parallel/ParallelRuntime.hpp"

#include <cstddef>
#include <utility>

namespace cfd
{

template <typename Function>
void parallelFor(std::size_t begin, std::size_t end, Function&& function)
{
#if CFDAPP_HAS_OPENMP
    if (ParallelRuntime::enabled())
    {
#pragma omp parallel for
        for (long long index = static_cast<long long>(begin); index < static_cast<long long>(end); ++index)
        {
            function(static_cast<std::size_t>(index));
        }
        return;
    }
#endif
    for (std::size_t index = begin; index < end; ++index) function(index);
}

}
