#include "validation/DeterminismValidator.hpp"

#include <cmath>

namespace cfd
{

bool DeterminismValidator::scalarEqual(
    const ScalarField& first,
    const ScalarField& second,
    double tolerance
) noexcept
{
    if (first.size() != second.size()) return false;
    for (std::size_t index = 0; index < first.size(); ++index)
    {
        if (std::abs(first[index] - second[index]) > tolerance) return false;
    }
    return true;
}

bool DeterminismValidator::vectorEqual(
    const VectorField& first,
    const VectorField& second,
    double tolerance
) noexcept
{
    if (first.size() != second.size()) return false;
    for (std::size_t index = 0; index < first.size(); ++index)
    {
        if (std::abs(first[index].x - second[index].x) > tolerance || std::abs(first[index].y - second[index].y) > tolerance) return false;
    }
    return true;
}

}
