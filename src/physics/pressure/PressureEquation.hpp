#pragma once

#include "physics/pressure/PressureAssembler.hpp"

namespace cfd
{

class PressureEquation
{
public:
    PressureEquation(
        const Mesh& mesh,
        double density,
        std::size_t referenceCell = 0,
        double referencePressure = 0.0
    );

    void assemble(
        const VectorField& velocity,
        const ScalarField& pressure,
        const ScalarField& momentumDiagonalU,
        const ScalarField& momentumDiagonalV,
        LinearSystem& system
    ) const;

    void applyPressureCorrection(
        ScalarField& pressure,
        const ScalarField& correction,
        double relaxation
    ) const;

    void correctVelocity(
        VectorField& velocity,
        const ScalarField& correction,
        const ScalarField& momentumDiagonalU,
        const ScalarField& momentumDiagonalV
    ) const;

private:
    const Mesh& mesh_;
    PressureAssembler assembler_;
};

}
