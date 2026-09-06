#include "numerics/discretization/Interpolation.hpp"

namespace cfd
{

double Interpolation::linear(double phiP, double phiN) noexcept
{
    return 0.5 * (phiP + phiN);
}

double Interpolation::upwind(double phiP, double phiN, double massFlux) noexcept
{
    return massFlux >= 0.0 ? phiP : phiN;
}

}
