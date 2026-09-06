#pragma once

#include "physics/flux/FaceFlux.hpp"

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"

#include <cstddef>

namespace cfd
{

class MassFluxCalculator
{
public:
    MassFluxCalculator(const Mesh& mesh, double density);

    [[nodiscard]] FaceFlux calculate(std::size_t cell, const VectorField& velocity) const;
    [[nodiscard]] FaceFlux corrected(
        std::size_t cell,
        const VectorField& velocity,
        const ScalarField& correction,
        const ScalarField& momentumDiagonalU,
        const ScalarField& momentumDiagonalV
    ) const;

    [[nodiscard]] double continuityResidual(const VectorField& velocity) const;
    [[nodiscard]] double correctedContinuityResidual(
        const VectorField& velocity,
        const ScalarField& correction,
        const ScalarField& momentumDiagonalU,
        const ScalarField& momentumDiagonalV
    ) const;

private:
    const Mesh& mesh_;
    double density_;
};

}
