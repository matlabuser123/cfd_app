#include "core/boundary/WallBC.hpp"

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"

#include <utility>

namespace cfd
{

WallBC::WallBC(std::string patchName, std::vector<std::size_t> cellIndices)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices))
{
}

void WallBC::apply(ScalarField& field) const
{
    validateScalarField(field);
}

void WallBC::apply(VectorField& field) const
{
    validateVectorField(field);
    for (const std::size_t index : cellIndices())
    {
        field[index] = Vector2{};
    }
}

}
