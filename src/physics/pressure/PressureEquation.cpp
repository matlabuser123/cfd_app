#include "physics/pressure/PressureEquation.hpp"
#include "parallel/ParallelFor.hpp"

#include <stdexcept>

namespace cfd
{

PressureEquation::PressureEquation(
    const Mesh& mesh,
    double density,
    std::size_t referenceCell,
    double referencePressure
)
    : mesh_(mesh),
      assembler_(mesh, density, referenceCell, referencePressure)
{
}

void PressureEquation::assemble(
    const VectorField& velocity,
    const ScalarField& pressure,
    const ScalarField& momentumDiagonalU,
    const ScalarField& momentumDiagonalV,
    LinearSystem& system
) const
{
    assembler_.assemble(velocity, pressure, momentumDiagonalU, momentumDiagonalV, system);
}

void PressureEquation::applyPressureCorrection(
    ScalarField& pressure,
    const ScalarField& correction,
    double relaxation
) const
{
    if (pressure.size() != mesh_.cellCount() || correction.size() != mesh_.cellCount()) throw std::invalid_argument("Pressure fields must match mesh size");
    if (relaxation <= 0.0 || relaxation > 1.0) throw std::invalid_argument("Pressure relaxation must be in (0, 1]");
    parallelFor(0, mesh_.cellCount(), [&](std::size_t cell) { pressure[cell] += relaxation * correction[cell]; });
}

void PressureEquation::correctVelocity(
    VectorField& velocity,
    const ScalarField& correction,
    const ScalarField& momentumDiagonalU,
    const ScalarField& momentumDiagonalV
) const
{
    const std::size_t cells = mesh_.cellCount();
    if (velocity.size() != cells || correction.size() != cells || momentumDiagonalU.size() != cells || momentumDiagonalV.size() != cells) throw std::invalid_argument("Velocity correction fields must match mesh size");

    // MomentumAssembler stores integrated finite-volume coefficients (kg/s), while its
    // pressure source is the cell-volume term -V grad(p). Therefore the SIMPLE velocity
    // correction is u' = -(V/aP) grad(p'), not -(1/aP) grad(p'). Omitting V here makes
    // the correction too large by 1/V (400x on a 20x20 unit-square mesh), which was the
    // direct source of the observed first-iteration blow-up.
    const double cellVolume = mesh_.dx() * mesh_.dy();
    if (cellVolume <= 0.0) throw std::invalid_argument("Velocity correction requires positive cell volume");

    parallelFor(0, cells, [&](std::size_t cell)
    {
        const auto east = mesh_.east(cell);
        const auto west = mesh_.west(cell);
        const auto north = mesh_.north(cell);
        const auto south = mesh_.south(cell);
        if (!east || !west || !north || !south) return;
        if (momentumDiagonalU[cell] <= 0.0 || momentumDiagonalV[cell] <= 0.0) throw std::invalid_argument("Momentum diagonals must be positive");
        const double dpdx = (correction[*east] - correction[*west]) / (2.0 * mesh_.dx());
        const double dpdy = (correction[*north] - correction[*south]) / (2.0 * mesh_.dy());
        velocity[cell].x -= cellVolume * dpdx / momentumDiagonalU[cell];
        velocity[cell].y -= cellVolume * dpdy / momentumDiagonalV[cell];
    });
}

}
