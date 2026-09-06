#pragma once

#include "physics/momentum/MomentumAssembler.hpp"

namespace cfd
{

class MomentumEquation
{
public:
    MomentumEquation(const Mesh& mesh, double density, double viscosity);

    void assembleU(const VectorField& velocity, const ScalarField& pressure, LinearSystem& system) const;
    void assembleV(const VectorField& velocity, const ScalarField& pressure, LinearSystem& system) const;

private:
    MomentumAssembler assembler_;
};

}
