#pragma once

#include <cstddef>

namespace cfd
{

struct OptimizationSettings
{
    std::size_t repetitions{10};
    double solutionTolerance{1e-10};
};

}
