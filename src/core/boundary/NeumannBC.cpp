#include "core/boundary/NeumannBC.hpp"

#include "core/fields/ScalarField.hpp"

#include <utility>

namespace cfd
{

NeumannBC::NeumannBC(
    std::string patchName,
    std::vector<std::size_t> cellIndices,
    double gradient
)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices)),
      gradient_(gradient)
{
}

void NeumannBC::apply(ScalarField& field) const
{
    validateScalarField(field);
}

double NeumannBC::gradient() const noexcept
{
    return gradient_;
}

}
