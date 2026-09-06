#pragma once

#include <cstddef>
#include <string>

namespace cfd
{

struct ValidationCase
{
    std::string name{"lid_driven_cavity"};
    std::size_t nx{20};
    std::size_t ny{20};
    double length{1.0};
    double height{1.0};
    double density{1.0};
    double viscosity{0.01};
    double lidVelocity{1.0};
    double continuityTolerance{1e-8};
    double momentumTolerance{1e-8};
    double pressureTolerance{20.0};
    std::size_t maxIterations{5000};
};

}
