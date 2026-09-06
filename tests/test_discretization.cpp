#include <cassert>
#include <cmath>
#include <stdexcept>

#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "numerics/discretization/Coefficients.hpp"
#include "numerics/discretization/Divergence.hpp"
#include "numerics/discretization/Flux.hpp"
#include "numerics/discretization/Gradient.hpp"
#include "numerics/discretization/Interpolation.hpp"
#include "numerics/discretization/Laplacian.hpp"

namespace
{

bool nearlyEqual(double left, double right)
{
    return std::abs(left - right) < 1e-10;
}

}

int main()
{
    assert(nearlyEqual(cfd::Interpolation::linear(2.0, 4.0), 3.0));
    assert(nearlyEqual(cfd::Interpolation::upwind(2.0, 4.0, 1.0), 2.0));
    assert(nearlyEqual(cfd::Interpolation::upwind(2.0, 4.0, -1.0), 4.0));
    assert(nearlyEqual(cfd::Flux::massFlux(2.0, {3.0, 4.0}, {1.0, -1.0}), -2.0));

    cfd::CellCoefficients coefficients{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    assert(nearlyEqual(coefficients.aP, 1.0));
    assert(nearlyEqual(coefficients.source, 6.0));

    cfd::Mesh mesh;
    mesh.generateUniform(5, 5, 2.0, 4.0);
    cfd::ScalarField scalar(mesh.cellCount());
    cfd::VectorField velocity(mesh.cellCount());
    for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell)
    {
        const double x = static_cast<double>(cell % mesh.nx()) * mesh.dx();
        const double y = static_cast<double>(cell / mesh.nx()) * mesh.dy();
        scalar[cell] = x * x + y * y;
        velocity[cell] = {x, y};
    }

    const std::size_t center = 2 + 2 * mesh.nx();
    const cfd::Vector2 gradient = cfd::Gradient::centralDifference(scalar, mesh, center);
    assert(nearlyEqual(gradient.x, 2.0 * 2.0 * mesh.dx()));
    assert(nearlyEqual(gradient.y, 2.0 * 2.0 * mesh.dy()));
    assert(nearlyEqual(cfd::Divergence::calculate(velocity, mesh, center), 2.0));
    assert(nearlyEqual(cfd::Laplacian::centralDifference(scalar, mesh, center), 4.0));

    bool threw = false;
    try
    {
        static_cast<void>(cfd::Gradient::centralDifference(scalar, mesh, 0));
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    assert(threw);

    return 0;
}
