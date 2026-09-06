#include "numerics/discretization/Laplacian.hpp"

#include "core/fields/ScalarField.hpp"

#include <stdexcept>

namespace cfd
{

double Laplacian::centralDifference(
    const ScalarField& field,
    const Mesh& mesh,
    std::size_t cell
)
{
    if (cell >= field.size() || cell >= mesh.cellCount()) throw std::out_of_range("Laplacian cell index out of range");
    const auto east = mesh.east(cell);
    const auto west = mesh.west(cell);
    const auto north = mesh.north(cell);
    const auto south = mesh.south(cell);
    if (!east || !west || !north || !south) throw std::invalid_argument("Central Laplacian requires an interior cell");
    if (mesh.dx() == 0.0 || mesh.dy() == 0.0) throw std::invalid_argument("Laplacian requires a non-empty mesh");

    const double center = field[cell];
    return (field[*east] - 2.0 * center + field[*west]) / (mesh.dx() * mesh.dx())
        + (field[*north] - 2.0 * center + field[*south]) / (mesh.dy() * mesh.dy());
}

}
