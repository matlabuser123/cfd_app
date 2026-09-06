#include "physics/momentum/MomentumEquation.hpp"

namespace cfd
{

MomentumEquation::MomentumEquation(const Mesh& mesh, double density, double viscosity)
    : assembler_(mesh, density, viscosity)
{
}

void MomentumEquation::assembleU(const VectorField& velocity, const ScalarField& pressure, LinearSystem& system) const
{
    assembler_.assembleU(velocity, pressure, system);
}

void MomentumEquation::assembleV(const VectorField& velocity, const ScalarField& pressure, LinearSystem& system) const
{
    assembler_.assembleV(velocity, pressure, system);
}

}
