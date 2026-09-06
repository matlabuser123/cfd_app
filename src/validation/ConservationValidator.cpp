#include "validation/ConservationValidator.hpp"

#include "physics/flux/MassFluxCalculator.hpp"

#include <stdexcept>

namespace cfd
{

double ConservationValidator::continuityResidual(
    const Mesh& mesh,
    const VectorField& velocity,
    double density
)
{
    if (velocity.size() != mesh.cellCount()) throw std::invalid_argument("Velocity field must match mesh size");
    return MassFluxCalculator(mesh, density).continuityResidual(velocity);
}

}
