#pragma once

#include "physics/pressure/PressureCoefficients.hpp"

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "numerics/matrix/LinearSystem.hpp"

#include <cstddef>

namespace cfd
{

class PressureAssembler
{
public:
    PressureAssembler(
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

private:
    [[nodiscard]] PressureCoefficients coefficients(
        std::size_t cell,
        const VectorField& velocity,
        const ScalarField& momentumDiagonalU,
        const ScalarField& momentumDiagonalV
    ) const;

    void validateInputs(
        const VectorField& velocity,
        const ScalarField& pressure,
        const ScalarField& momentumDiagonalU,
        const ScalarField& momentumDiagonalV,
        const LinearSystem& system
    ) const;

    const Mesh& mesh_;
    double density_;
    std::size_t referenceCell_;
    double referencePressure_;
};

}
