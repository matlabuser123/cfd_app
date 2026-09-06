#pragma once

namespace cfd
{

class Interpolation
{
public:
    [[nodiscard]] static double linear(double phiP, double phiN) noexcept;
    [[nodiscard]] static double upwind(double phiP, double phiN, double massFlux) noexcept;
};

}
