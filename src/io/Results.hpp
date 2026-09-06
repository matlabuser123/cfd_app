#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace cfd
{

struct ResidualRecord
{
    std::size_t iteration{0};
    double continuity{0.0};
    double uMomentum{0.0};
    double vMomentum{0.0};
};

struct CFDResults
{
    std::string caseName{"cfd_case"};
    std::size_t nx{0};
    std::size_t ny{0};
    double length{1.0};
    double height{1.0};
    double density{1.0};
    double viscosity{0.01};
    double time{0.0};
    std::size_t iterations{0};
    double continuityResidual{0.0};
    double uResidual{0.0};
    double vResidual{0.0};
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> u;
    std::vector<double> v;
    std::vector<double> pressure;
    std::vector<double> velocityMagnitude;
    std::vector<double> vorticity;
    std::vector<ResidualRecord> residualHistory;
};

}
