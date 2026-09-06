#include "numerics/discretization/Flux.hpp"

namespace cfd
{

double Flux::massFlux(
    double density,
    const Vector2& velocity,
    const Vector2& faceArea
) noexcept
{
    return density * (velocity.x * faceArea.x + velocity.y * faceArea.y);
}

}
