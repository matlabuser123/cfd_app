#pragma once

#include "core/fields/VectorField.hpp"

namespace cfd
{

class Flux
{
public:
    [[nodiscard]] static double massFlux(
        double density,
        const Vector2& velocity,
        const Vector2& faceArea
    ) noexcept;
};

}
