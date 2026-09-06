#include "core/boundary/DirichletBC.hpp"

#include "core/fields/ScalarField.hpp"

#include <stdexcept>
#include <utility>

namespace cfd
{

DirichletBC::DirichletBC(
    std::string patchName,
    std::vector<std::size_t> cellIndices,
    double value
)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices)),
      scalarValue_(value)
{
}

DirichletBC::DirichletBC(
    std::string patchName,
    std::vector<std::size_t> cellIndices,
    Vector2 value
)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices)),
      vectorValue_(value),
      isVector_(true)
{
}

void DirichletBC::apply(ScalarField& field) const
{
    if (isVector_)
    {
        throw std::logic_error("Vector Dirichlet condition cannot apply to a scalar field");
    }

    validateScalarField(field);
    for (const std::size_t index : cellIndices())
    {
        field[index] = scalarValue_;
    }
}

void DirichletBC::apply(VectorField& field) const
{
    if (!isVector_)
    {
        throw std::logic_error("Scalar Dirichlet condition cannot apply to a vector field");
    }

    validateVectorField(field);
    for (const std::size_t index : cellIndices())
    {
        field[index] = vectorValue_;
    }
}

}
