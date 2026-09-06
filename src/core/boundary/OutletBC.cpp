#include "core/boundary/OutletBC.hpp"

#include "core/fields/ScalarField.hpp"

#include <utility>

namespace cfd
{

OutletBC::OutletBC(
    std::string patchName,
    std::vector<std::size_t> cellIndices
)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices))
{
}

OutletBC::OutletBC(
    std::string patchName,
    std::vector<std::size_t> cellIndices,
    double pressure
)
    : BoundaryCondition(std::move(patchName), std::move(cellIndices)),
      pressure_(pressure)
{
}

void OutletBC::apply(ScalarField& field) const
{
    validateScalarField(field);
    if (!pressure_.has_value())
    {
        return;
    }

    for (const std::size_t index : cellIndices())
    {
        field[index] = *pressure_;
    }
}

bool OutletBC::hasPressure() const noexcept
{
    return pressure_.has_value();
}

std::optional<double> OutletBC::pressure() const noexcept
{
    return pressure_;
}

}
