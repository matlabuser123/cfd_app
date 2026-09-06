#pragma once

#include <cstddef>

namespace cfd
{

struct SIMPLESettings
{
    std::size_t maxIterations{1000};
    double momentumTolerance{1e-8};
    double pressureTolerance{1e-8};
    double continuityTolerance{1e-8};
    double velocityRelaxation{0.7};
    double pressureRelaxation{0.3};
};

}
