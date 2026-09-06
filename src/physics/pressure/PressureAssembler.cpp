#include "physics/pressure/PressureAssembler.hpp"
#include "parallel/ParallelFor.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cfd
{

PressureAssembler::PressureAssembler(
    const Mesh& mesh,
    double density,
    std::size_t referenceCell,
    double referencePressure
)
    : mesh_(mesh),
      density_(density),
      referenceCell_(referenceCell),
      referencePressure_(referencePressure)
{
    if (density_ <= 0.0) throw std::invalid_argument("Density must be positive");
    if (referenceCell_ >= mesh_.cellCount()) throw std::out_of_range("Pressure reference cell out of range");
}

void PressureAssembler::validateInputs(
    const VectorField& velocity,
    const ScalarField& pressure,
    const ScalarField& momentumDiagonalU,
    const ScalarField& momentumDiagonalV,
    const LinearSystem& system
) const
{
    const std::size_t cells = mesh_.cellCount();
    if (velocity.size() != cells || pressure.size() != cells || momentumDiagonalU.size() != cells || momentumDiagonalV.size() != cells)
    {
        throw std::invalid_argument("Pressure fields must match mesh size");
    }
    if (system.size() != cells || system.matrix().rows() != cells || system.matrix().cols() != cells)
    {
        throw std::invalid_argument("Pressure system must match mesh size");
    }
    if (mesh_.dx() <= 0.0 || mesh_.dy() <= 0.0) throw std::invalid_argument("Pressure assembly requires a non-empty mesh");
}

PressureCoefficients PressureAssembler::coefficients(
    std::size_t cell,
    const VectorField& velocity,
    const ScalarField& momentumDiagonalU,
    const ScalarField& momentumDiagonalV
) const
{
    if (momentumDiagonalU[cell] <= 0.0 || momentumDiagonalV[cell] <= 0.0) throw std::invalid_argument("Momentum diagonals must be positive");

    const auto east = mesh_.east(cell);
    const auto west = mesh_.west(cell);
    const auto north = mesh_.north(cell);
    const auto south = mesh_.south(cell);
    const double eastArea = mesh_.dy();
    const double northArea = mesh_.dx();
    const auto faceDiagonal = [](double current, double neighbor)
    {
        const double average = 0.5 * (current + neighbor);
        if (average <= 0.0) throw std::invalid_argument("Momentum diagonals must be positive");
        return average;
    };
    const double aE = east ? density_ * eastArea * eastArea / faceDiagonal(momentumDiagonalU[cell], momentumDiagonalU[*east]) : 0.0;
    const double aW = west ? density_ * eastArea * eastArea / faceDiagonal(momentumDiagonalU[cell], momentumDiagonalU[*west]) : 0.0;
    const double aN = north ? density_ * northArea * northArea / faceDiagonal(momentumDiagonalV[cell], momentumDiagonalV[*north]) : 0.0;
    const double aS = south ? density_ * northArea * northArea / faceDiagonal(momentumDiagonalV[cell], momentumDiagonalV[*south]) : 0.0;

    double fluxE = 0.0;
    double fluxW = 0.0;
    double fluxN = 0.0;
    double fluxS = 0.0;
    if (east) fluxE = density_ * 0.5 * (velocity[cell].x + velocity[*east].x) * eastArea;
    if (west) fluxW = density_ * 0.5 * (velocity[cell].x + velocity[*west].x) * eastArea;
    if (north) fluxN = density_ * 0.5 * (velocity[cell].y + velocity[*north].y) * northArea;
    if (south) fluxS = density_ * 0.5 * (velocity[cell].y + velocity[*south].y) * northArea;

    return {aE + aW + aN + aS, aE, aW, aN, aS, fluxW - fluxE + fluxS - fluxN};
}

void PressureAssembler::assemble(
    const VectorField& velocity,
    const ScalarField& pressure,
    const ScalarField& momentumDiagonalU,
    const ScalarField& momentumDiagonalV,
    LinearSystem& system
) const
{
    validateInputs(velocity, pressure, momentumDiagonalU, momentumDiagonalV, system);
    system.matrix().clear();
    std::fill(system.rhs().begin(), system.rhs().end(), 0.0);
    const std::size_t nx = mesh_.nx();
    const std::size_t ny = mesh_.ny();
    const double eastArea = mesh_.dy();
    const double northArea = mesh_.dx();
    auto& matrix = system.matrix();
    auto& rhs = system.rhs();
    const auto faceCoefficient = [](double current, double neighbor, double area, double density)
    {
        const double average = 0.5 * (current + neighbor);
        if (average <= 0.0) throw std::invalid_argument("Momentum diagonals must be positive");
        return density * area * area / average;
    };

    parallelFor(0, ny, [&](std::size_t row)
    {
        for (std::size_t column = 0; column < nx; ++column)
        {
            const std::size_t cell = row * nx + column;
            if (cell == referenceCell_)
            {
                matrix.clearRow(cell);
                matrix.set(cell, cell, 1.0);
                rhs[cell] = referencePressure_;
                continue;
            }

            const double diagonalU = momentumDiagonalU[cell];
            const double diagonalV = momentumDiagonalV[cell];
            if (diagonalU <= 0.0 || diagonalV <= 0.0) throw std::invalid_argument("Momentum diagonals must be positive");

            const bool hasEast = column + 1 < nx;
            const bool hasWest = column > 0;
            const bool hasNorth = row + 1 < ny;
            const bool hasSouth = row > 0;
            const std::size_t east = cell + 1;
            const std::size_t west = cell - 1;
            const std::size_t north = cell + nx;
            const std::size_t south = cell - nx;
            const double aE = hasEast ? faceCoefficient(diagonalU, momentumDiagonalU[east], eastArea, density_) : 0.0;
            const double aW = hasWest ? faceCoefficient(diagonalU, momentumDiagonalU[west], eastArea, density_) : 0.0;
            const double aN = hasNorth ? faceCoefficient(diagonalV, momentumDiagonalV[north], northArea, density_) : 0.0;
            const double aS = hasSouth ? faceCoefficient(diagonalV, momentumDiagonalV[south], northArea, density_) : 0.0;
            const double fluxE = hasEast ? density_ * 0.5 * (velocity[cell].x + velocity[east].x) * eastArea : 0.0;
            const double fluxW = hasWest ? density_ * 0.5 * (velocity[cell].x + velocity[west].x) * eastArea : 0.0;
            const double fluxN = hasNorth ? density_ * 0.5 * (velocity[cell].y + velocity[north].y) * northArea : 0.0;
            const double fluxS = hasSouth ? density_ * 0.5 * (velocity[cell].y + velocity[south].y) * northArea : 0.0;

            matrix.set(cell, cell, aE + aW + aN + aS);
            rhs[cell] = fluxW - fluxE + fluxS - fluxN;
            const auto addNeighbor = [&](bool exists, std::size_t neighbor, double coefficient)
            {
                if (exists && neighbor != referenceCell_) matrix.set(cell, neighbor, -coefficient);
            };
            addNeighbor(hasEast, east, aE);
            addNeighbor(hasWest, west, aW);
            addNeighbor(hasNorth, north, aN);
            addNeighbor(hasSouth, south, aS);
        }
    });
}

}
