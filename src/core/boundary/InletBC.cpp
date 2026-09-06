#include "core/boundary/InletBC.hpp"

#include "core/fields/ScalarField.hpp"

#include <stdexcept>
#include <utility>

namespace cfd
{

InletBC::InletBC(
    std::string patchName,
    std::vector<std::size_t> cellIndices,
    double value
)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices)),
      scalarValue_(value)
{
}

InletBC::InletBC(
    std::string patchName,
    std::vector<std::size_t> cellIndices,
    Vector2 value
)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices)),
      vectorValue_(value),
      isVector_(true)
{
}

void InletBC::apply(ScalarField& field) const
{
    if (isVector_)
    {
        throw std::logic_error("Vector inlet condition cannot apply to a scalar field");
    }

    validateScalarField(field);
    for (const std::size_t index : cellIndices())
    {
        field[index] = scalarValue_;
    }
}

void InletBC::apply(VectorField& field) const
{
    if (!isVector_)
    {
        throw std::logic_error("Scalar inlet condition cannot apply to a vector field");
    }

    validateVectorField(field);
    for (const std::size_t index : cellIndices())
    {
        field[index] = vectorValue_;
    }
}

}
