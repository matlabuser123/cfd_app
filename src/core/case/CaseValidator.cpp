#include "core/case/CaseValidator.hpp"

#include <stdexcept>

namespace cfd
{

void CaseValidator::validate(const CaseConfig& config)
{
    if (config.name.empty()) throw std::invalid_argument("Case name must not be empty");
    if (config.dimensions != 2) throw std::invalid_argument("Only two-dimensional cases are supported");
    if (config.mesh.nx == 0 || config.mesh.ny == 0) throw std::invalid_argument("Mesh dimensions must be positive");
    if (config.mesh.width <= 0.0 || config.mesh.height <= 0.0) throw std::invalid_argument("Mesh dimensions must be positive");
    if (config.fluid.density <= 0.0) throw std::invalid_argument("Fluid density must be positive");
    if (config.fluid.dynamicViscosity <= 0.0) throw std::invalid_argument("Dynamic viscosity must be positive");
    if (config.numerics.algorithm.empty()) throw std::invalid_argument("Numerical algorithm must not be empty");
    if (config.numerics.momentumTolerance <= 0.0 || config.numerics.pressureTolerance <= 0.0) throw std::invalid_argument("Solver tolerances must be positive");
    if (config.numerics.maxIterations == 0) throw std::invalid_argument("Maximum iterations must be positive");
    if (config.numerics.velocityUnderRelaxation <= 0.0 || config.numerics.velocityUnderRelaxation > 1.0) throw std::invalid_argument("Velocity under-relaxation must be in (0, 1]");
    if (config.numerics.pressureUnderRelaxation <= 0.0 || config.numerics.pressureUnderRelaxation > 1.0) throw std::invalid_argument("Pressure under-relaxation must be in (0, 1]");
    if (config.boundaries.empty()) throw std::invalid_argument("At least one boundary condition is required");

    for (const auto& [patch, boundary] : config.boundaries)
    {
        if (patch.empty()) throw std::invalid_argument("Boundary patch name must not be empty");
        if (boundary.type != "wall" && boundary.type != "velocity" && boundary.type != "pressure" && boundary.type != "neumann" && boundary.type != "dirichlet")
        {
            throw std::invalid_argument("Unknown boundary type: " + boundary.type);
        }
        if ((boundary.type == "velocity") != boundary.vectorValue.has_value())
        {
            throw std::invalid_argument("Velocity boundaries require a two-dimensional value");
        }
        if ((boundary.type == "pressure" || boundary.type == "dirichlet") && !boundary.value.has_value())
        {
            throw std::invalid_argument("Scalar boundary requires a value");
        }
        if (boundary.type == "neumann" && !boundary.gradient.has_value())
        {
            throw std::invalid_argument("Neumann boundaries require a gradient");
        }
    }
}

}
