#pragma once

#include <cstddef>
#include <string>

namespace cfd
{

struct ValidationResult
{
    bool passed{false};
    bool cancelled{false};
    std::string caseName;
    std::size_t nx{0};
    std::size_t ny{0};
    std::size_t iterations{0};
    double continuityResidual{0.0};
    double uResidual{0.0};
    double vResidual{0.0};
    double massImbalance{0.0};
    double runtimeSeconds{0.0};
    double centerVelocityU{0.0};
    double centerVelocityV{0.0};
    bool finiteSolution{false};
    bool deterministic{false};
};

}
