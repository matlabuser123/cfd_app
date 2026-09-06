#include "physics/flux/MassFluxCalculator.hpp"
#include "parallel/ParallelFor.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace cfd
{

MassFluxCalculator::MassFluxCalculator(const Mesh& mesh, double density)
    : mesh_(mesh),
      density_(density)
{
    if (density_ <= 0.0) throw std::invalid_argument("Density must be positive");
}

FaceFlux MassFluxCalculator::calculate(std::size_t cell, const VectorField& velocity) const
{
    if (cell >= mesh_.cellCount() || velocity.size() != mesh_.cellCount()) throw std::invalid_argument("Flux inputs must match mesh size");
    FaceFlux flux;
    if (const auto east = mesh_.east(cell)) flux.east = density_ * 0.5 * (velocity[cell].x + velocity[*east].x) * mesh_.dy();
    if (const auto west = mesh_.west(cell)) flux.west = density_ * 0.5 * (velocity[cell].x + velocity[*west].x) * mesh_.dy();
    if (const auto north = mesh_.north(cell)) flux.north = density_ * 0.5 * (velocity[cell].y + velocity[*north].y) * mesh_.dx();
    if (const auto south = mesh_.south(cell)) flux.south = density_ * 0.5 * (velocity[cell].y + velocity[*south].y) * mesh_.dx();
    return flux;
}

FaceFlux MassFluxCalculator::corrected(
    std::size_t cell,
    const VectorField& velocity,
    const ScalarField& correction,
    const ScalarField& momentumDiagonalU,
    const ScalarField& momentumDiagonalV
) const
{
    if (correction.size() != mesh_.cellCount() || momentumDiagonalU.size() != mesh_.cellCount() || momentumDiagonalV.size() != mesh_.cellCount()) throw std::invalid_argument("Flux correction fields must match mesh size");
    FaceFlux flux = calculate(cell, velocity);
    const double pressure = correction[cell];
    if (const auto east = mesh_.east(cell)) flux.east += density_ * mesh_.dy() * (mesh_.dy() / momentumDiagonalU[cell]) * (pressure - correction[*east]);
    if (const auto west = mesh_.west(cell)) flux.west += density_ * mesh_.dy() * (mesh_.dy() / momentumDiagonalU[cell]) * (correction[*west] - pressure);
    if (const auto north = mesh_.north(cell)) flux.north += density_ * mesh_.dx() * (mesh_.dx() / momentumDiagonalV[cell]) * (pressure - correction[*north]);
    if (const auto south = mesh_.south(cell)) flux.south += density_ * mesh_.dx() * (mesh_.dx() / momentumDiagonalV[cell]) * (correction[*south] - pressure);
    return flux;
}

double MassFluxCalculator::continuityResidual(const VectorField& velocity) const
{
    std::vector<double> cellResiduals(mesh_.cellCount());
    parallelFor(0, mesh_.cellCount(), [&](std::size_t cell)
    {
        const FaceFlux flux = calculate(cell, velocity);
        cellResiduals[cell] = std::abs(flux.west - flux.east + flux.south - flux.north);
    });
    double residual = 0.0;
    for (const double cellResidual : cellResiduals)
    {
        residual += cellResidual;
    }
    return residual;
}

double MassFluxCalculator::correctedContinuityResidual(
    const VectorField& velocity,
    const ScalarField& correction,
    const ScalarField& momentumDiagonalU,
    const ScalarField& momentumDiagonalV
) const
{
    std::vector<double> cellResiduals(mesh_.cellCount());
    parallelFor(0, mesh_.cellCount(), [&](std::size_t cell)
    {
        const FaceFlux flux = corrected(cell, velocity, correction, momentumDiagonalU, momentumDiagonalV);
        cellResiduals[cell] = std::abs(flux.west - flux.east + flux.south - flux.north);
    });
    double residual = 0.0;
    for (const double cellResidual : cellResiduals)
    {
        residual += cellResidual;
    }
    return residual;
}

}
