#pragma once

#include <cstddef>
#include <vector>

namespace cfd
{

struct SIMPLEIteration
{
    std::size_t iteration{0};
    double continuityResidual{0.0};
    double uResidual{0.0};
    double vResidual{0.0};
    double pressureResidual{0.0};
};

struct SIMPLEResult
{
    bool converged{false};
    bool cancelled{false};
    std::size_t iterations{0};
    double continuityResidual{0.0};
    double uResidual{0.0};
    double vResidual{0.0};
    double pressureResidual{0.0};
    std::vector<SIMPLEIteration> history;
};

}
