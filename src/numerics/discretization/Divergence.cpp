#include "numerics/discretization/Divergence.hpp"

#include <stdexcept>

namespace cfd
{

double Divergence::calculate(
    const VectorField& velocity,
    const Mesh& mesh,
    std::size_t cell
)
{
    if (cell >= velocity.size() || cell >= mesh.cellCount()) throw std::out_of_range("Divergence cell index out of range");
    const auto east = mesh.east(cell);
    const auto west = mesh.west(cell);
    const auto north = mesh.north(cell);
    const auto south = mesh.south(cell);
    if (!east || !west || !north || !south) throw std::invalid_argument("Central divergence requires an interior cell");
    if (mesh.dx() == 0.0 || mesh.dy() == 0.0) throw std::invalid_argument("Divergence requires a non-empty mesh");

    return (velocity[*east].x - velocity[*west].x) / (2.0 * mesh.dx())
        + (velocity[*north].y - velocity[*south].y) / (2.0 * mesh.dy());
}

}
