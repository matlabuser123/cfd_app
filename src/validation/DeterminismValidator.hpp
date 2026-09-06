#pragma once

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"

namespace cfd
{

class DeterminismValidator
{
public:
    [[nodiscard]] static bool scalarEqual(
        const ScalarField& first,
        const ScalarField& second,
        double tolerance
    ) noexcept;

    [[nodiscard]] static bool vectorEqual(
        const VectorField& first,
        const VectorField& second,
        double tolerance
    ) noexcept;
};

}
