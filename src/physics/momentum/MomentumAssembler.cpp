#include "physics/momentum/MomentumAssembler.hpp"
#include "parallel/ParallelFor.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cfd
{

namespace
{

template <bool XComponent>
void assembleMomentum(
    const Mesh& mesh,
    double density,
    double viscosity,
    const VectorField& velocity,
    const ScalarField& pressure,
    LinearSystem& system
)
{
    const std::size_t nx = mesh.nx();
    const std::size_t ny = mesh.ny();
    const double dx = mesh.dx();
    const double dy = mesh.dy();
    const double diffusionX = viscosity * dy / dx;
    const double diffusionY = viscosity * dx / dy;
    auto& matrix = system.matrix();
    auto& rhs = system.rhs();

    parallelFor(0, ny, [&](std::size_t row)
    {
        for (std::size_t column = 0; column < nx; ++column)
        {
            const std::size_t cell = row * nx + column;
            const bool boundary = row == 0 || row + 1 == ny || column == 0 || column + 1 == nx;
            if (boundary)
            {
                matrix.set(cell, cell, 1.0);
                rhs[cell] = XComponent ? velocity[cell].x : velocity[cell].y;
                continue;
            }

            const std::size_t east = cell + 1;
            const std::size_t west = cell - 1;
            const std::size_t north = cell + nx;
            const std::size_t south = cell - nx;
            const double fluxE = density * 0.5 * (velocity[cell].x + velocity[east].x) * dy;
            const double fluxW = density * 0.5 * (velocity[cell].x + velocity[west].x) * dy;
            const double fluxN = density * 0.5 * (velocity[cell].y + velocity[north].y) * dx;
            const double fluxS = density * 0.5 * (velocity[cell].y + velocity[south].y) * dx;
            const double aE = diffusionX + std::max(-fluxE, 0.0);
            const double aW = diffusionX + std::max(fluxW, 0.0);
            const double aN = diffusionY + std::max(-fluxN, 0.0);
            const double aS = diffusionY + std::max(fluxS, 0.0);
            const double source = XComponent
                ? -(pressure[east] - pressure[west]) / (2.0 * dx)
                : -(pressure[north] - pressure[south]) / (2.0 * dy);

            matrix.set(cell, cell, aE + aW + aN + aS);
            rhs[cell] = source;
            const auto addNeighbor = [&](std::size_t neighbor, double coefficient)
            {
                const std::size_t neighborRow = neighbor / nx;
                const std::size_t neighborColumn = neighbor % nx;
                const bool neighborBoundary = neighborRow == 0 || neighborRow + 1 == ny || neighborColumn == 0 || neighborColumn + 1 == nx;
                if (neighborBoundary)
                {
                    rhs[cell] += coefficient * (XComponent ? velocity[neighbor].x : velocity[neighbor].y);
                }
                else
                {
                    matrix.set(cell, neighbor, -coefficient);
                }
            };
            addNeighbor(east, aE);
            addNeighbor(west, aW);
            addNeighbor(north, aN);
            addNeighbor(south, aS);
        }
    });
}

}

MomentumAssembler::MomentumAssembler(const Mesh& mesh, double density, double viscosity)
    : mesh_(mesh),
      density_(density),
      viscosity_(viscosity)
{
    if (density_ <= 0.0) throw std::invalid_argument("Density must be positive");
    if (viscosity_ <= 0.0) throw std::invalid_argument("Viscosity must be positive");
}

void MomentumAssembler::validateInputs(
    const VectorField& velocity,
    const ScalarField& pressure,
    const LinearSystem& system
) const
{
    if (velocity.size() != mesh_.cellCount() || pressure.size() != mesh_.cellCount())
    {
        throw std::invalid_argument("Momentum fields must match mesh size");
    }
    if (system.size() != mesh_.cellCount() || system.matrix().rows() != mesh_.cellCount() || system.matrix().cols() != mesh_.cellCount())
    {
        throw std::invalid_argument("Momentum system must match mesh size");
    }
    if (mesh_.dx() <= 0.0 || mesh_.dy() <= 0.0)
    {
        throw std::invalid_argument("Momentum assembly requires a non-empty mesh");
    }
}

MomentumCoefficients MomentumAssembler::coefficientsU(
    std::size_t cell,
    const VectorField& velocity,
    const ScalarField& pressure
) const
{
    const auto east = mesh_.east(cell);
    const auto west = mesh_.west(cell);
    const auto north = mesh_.north(cell);
    const auto south = mesh_.south(cell);
    const bool interior = east && west && north && south;
    if (!interior)
    {
        return {1.0, 0.0, 0.0, 0.0, 0.0, velocity[cell].x};
    }

    const double dx = mesh_.dx();
    const double dy = mesh_.dy();
    const double diffusionX = viscosity_ * dy / dx;
    const double diffusionY = viscosity_ * dx / dy;
    const double fluxE = density_ * 0.5 * (velocity[cell].x + velocity[*east].x) * dy;
    const double fluxW = density_ * 0.5 * (velocity[cell].x + velocity[*west].x) * dy;
    const double fluxN = density_ * 0.5 * (velocity[cell].y + velocity[*north].y) * dx;
    const double fluxS = density_ * 0.5 * (velocity[cell].y + velocity[*south].y) * dx;

    return {
        diffusionX + std::max(-fluxE, 0.0) + diffusionX + std::max(fluxW, 0.0)
            + diffusionY + std::max(-fluxN, 0.0) + diffusionY + std::max(fluxS, 0.0),
        diffusionX + std::max(-fluxE, 0.0),
        diffusionX + std::max(fluxW, 0.0),
        diffusionY + std::max(-fluxN, 0.0),
        diffusionY + std::max(fluxS, 0.0),
        -(pressure[*east] - pressure[*west]) / (2.0 * dx)
    };
}

MomentumCoefficients MomentumAssembler::coefficientsV(
    std::size_t cell,
    const VectorField& velocity,
    const ScalarField& pressure
) const
{
    const auto east = mesh_.east(cell);
    const auto west = mesh_.west(cell);
    const auto north = mesh_.north(cell);
    const auto south = mesh_.south(cell);
    const bool interior = east && west && north && south;
    if (!interior)
    {
        return {1.0, 0.0, 0.0, 0.0, 0.0, velocity[cell].y};
    }

    const double dx = mesh_.dx();
    const double dy = mesh_.dy();
    const double diffusionX = viscosity_ * dy / dx;
    const double diffusionY = viscosity_ * dx / dy;
    const double fluxE = density_ * 0.5 * (velocity[cell].x + velocity[*east].x) * dy;
    const double fluxW = density_ * 0.5 * (velocity[cell].x + velocity[*west].x) * dy;
    const double fluxN = density_ * 0.5 * (velocity[cell].y + velocity[*north].y) * dx;
    const double fluxS = density_ * 0.5 * (velocity[cell].y + velocity[*south].y) * dx;

    return {
        diffusionX + std::max(-fluxE, 0.0) + diffusionX + std::max(fluxW, 0.0)
            + diffusionY + std::max(-fluxN, 0.0) + diffusionY + std::max(fluxS, 0.0),
        diffusionX + std::max(-fluxE, 0.0),
        diffusionX + std::max(fluxW, 0.0),
        diffusionY + std::max(-fluxN, 0.0),
        diffusionY + std::max(fluxS, 0.0),
        -(pressure[*north] - pressure[*south]) / (2.0 * dy)
    };
}

void MomentumAssembler::writeRow(
    std::size_t cell,
    const MomentumCoefficients& coefficients,
    const VectorField& velocity,
    bool xComponent,
    LinearSystem& system
) const
{
    system.matrix().set(cell, cell, coefficients.aP);
    system.rhs()[cell] = coefficients.source;
    const auto isBoundaryCell = [this](std::size_t index)
    {
        return !mesh_.east(index) || !mesh_.west(index) || !mesh_.north(index) || !mesh_.south(index);
    };
    const auto addNeighbor = [&](const std::optional<std::size_t> neighbor, double coefficient)
    {
        if (!neighbor) return;
        if (isBoundaryCell(*neighbor))
        {
            system.rhs()[cell] += coefficient * (xComponent ? velocity[*neighbor].x : velocity[*neighbor].y);
        }
        else
        {
            system.matrix().set(cell, *neighbor, -coefficient);
        }
    };
    addNeighbor(mesh_.east(cell), coefficients.aE);
    addNeighbor(mesh_.west(cell), coefficients.aW);
    addNeighbor(mesh_.north(cell), coefficients.aN);
    addNeighbor(mesh_.south(cell), coefficients.aS);
}

void MomentumAssembler::assembleU(
    const VectorField& velocity,
    const ScalarField& pressure,
    LinearSystem& system
) const
{
    validateInputs(velocity, pressure, system);
    system.matrix().clear();
    std::fill(system.rhs().begin(), system.rhs().end(), 0.0);
    assembleMomentum<true>(mesh_, density_, viscosity_, velocity, pressure, system);
}

void MomentumAssembler::assembleV(
    const VectorField& velocity,
    const ScalarField& pressure,
    LinearSystem& system
) const
{
    validateInputs(velocity, pressure, system);
    system.matrix().clear();
    std::fill(system.rhs().begin(), system.rhs().end(), 0.0);
    assembleMomentum<false>(mesh_, density_, viscosity_, velocity, pressure, system);
}

}
