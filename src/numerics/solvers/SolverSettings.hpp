#pragma once

#include <cstddef>

namespace cfd
{

struct SolverSettings
{
    std::size_t maxIterations{1000};
    double absoluteTolerance{1e-10};
    double relativeTolerance{1e-8};
};

}
