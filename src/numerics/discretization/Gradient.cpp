#include "numerics/discretization/Gradient.hpp"

#include "core/fields/ScalarField.hpp"

#include <stdexcept>

namespace cfd
{

Vector2 Gradient::centralDifference(
    const ScalarField& field,
    const Mesh& mesh,
    std::size_t cell
)
{
    if (cell >= field.size() || cell >= mesh.cellCount()) throw std::out_of_range("Gradient cell index out of range");
    const auto east = mesh.east(cell);
    const auto west = mesh.west(cell);
    const auto north = mesh.north(cell);
    const auto south = mesh.south(cell);
    if (!east || !west || !north || !south) throw std::invalid_argument("Central gradient requires an interior cell");
    if (mesh.dx() == 0.0 || mesh.dy() == 0.0) throw std::invalid_argument("Gradient requires a non-empty mesh");

    return {
        (field[*east] - field[*west]) / (2.0 * mesh.dx()),
        (field[*north] - field[*south]) / (2.0 * mesh.dy())
    };
}

}
