#pragma once

#include "physics/momentum/MomentumCoefficients.hpp"

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "numerics/matrix/LinearSystem.hpp"

#include <cstddef>

namespace cfd
{

class MomentumAssembler
{
public:
    MomentumAssembler(const Mesh& mesh, double density, double viscosity);

    void assembleU(
        const VectorField& velocity,
        const ScalarField& pressure,
        LinearSystem& system
    ) const;

    void assembleV(
        const VectorField& velocity,
        const ScalarField& pressure,
        LinearSystem& system
    ) const;

private:
    [[nodiscard]] MomentumCoefficients coefficientsU(
        std::size_t cell,
        const VectorField& velocity,
        const ScalarField& pressure
    ) const;

    [[nodiscard]] MomentumCoefficients coefficientsV(
        std::size_t cell,
        const VectorField& velocity,
        const ScalarField& pressure
    ) const;

    void validateInputs(
        const VectorField& velocity,
        const ScalarField& pressure,
        const LinearSystem& system
    ) const;

    void writeRow(
        std::size_t cell,
        const MomentumCoefficients& coefficients,
        const VectorField& velocity,
        bool xComponent,
        LinearSystem& system
    ) const;

    const Mesh& mesh_;
    double density_;
    double viscosity_;
};

}
