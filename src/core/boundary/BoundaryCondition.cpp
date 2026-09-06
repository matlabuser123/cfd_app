#include "core/boundary/BoundaryCondition.hpp"

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"

#include <stdexcept>
#include <utility>

namespace cfd
{

BoundaryCondition::BoundaryCondition(
    std::string patchName,
    std::vector<std::size_t> cellIndices
)
    : patchName_(std::move(patchName)),
      cellIndices_(std::move(cellIndices))
{
}

const std::string& BoundaryCondition::patchName() const noexcept
{
    return patchName_;
}

const std::vector<std::size_t>& BoundaryCondition::cellIndices() const noexcept
{
    return cellIndices_;
}

void BoundaryCondition::apply(VectorField&) const
{
    throw std::logic_error("This boundary condition does not apply to vector fields");
}

void BoundaryCondition::validateScalarField(const ScalarField& field) const
{
    for (const std::size_t index : cellIndices_)
    {
        if (index >= field.size())
        {
            throw std::out_of_range("Boundary cell index exceeds scalar field size");
        }
    }
}

void BoundaryCondition::validateVectorField(const VectorField& field) const
{
    for (const std::size_t index : cellIndices_)
    {
        if (index >= field.size())
        {
            throw std::out_of_range("Boundary cell index exceeds vector field size");
        }
    }
}

}
