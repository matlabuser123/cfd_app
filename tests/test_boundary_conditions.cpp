#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "core/boundary/DirichletBC.hpp"
#include "core/boundary/InletBC.hpp"
#include "core/boundary/NeumannBC.hpp"
#include "core/boundary/OutletBC.hpp"
#include "core/boundary/WallBC.hpp"
#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"

namespace
{

bool nearlyEqual(double left, double right)
{
    return std::abs(left - right) < 1e-12;
}

}

int main()
{
    const std::vector<std::size_t> cells{1, 3};
    cfd::ScalarField scalar(5, -1.0);
    cfd::VectorField velocity(5, cfd::Vector2{9.0, 9.0});

    cfd::DirichletBC fixedPressure("left", cells, 12.0);
    assert(fixedPressure.patchName() == "left");
    assert(fixedPressure.cellIndices() == cells);
    fixedPressure.apply(scalar);
    assert(nearlyEqual(scalar[1], 12.0));
    assert(nearlyEqual(scalar[3], 12.0));
    assert(nearlyEqual(scalar[0], -1.0));

    cfd::DirichletBC fixedVelocity("moving-wall", cells, cfd::Vector2{2.0, 3.0});
    fixedVelocity.apply(velocity);
    assert(nearlyEqual(velocity[1].x, 2.0));
    assert(nearlyEqual(velocity[3].y, 3.0));

    cfd::InletBC inlet("inlet", cells, cfd::Vector2{1.0, 0.0});
    inlet.apply(velocity);
    assert(nearlyEqual(velocity[1].x, 1.0));
    assert(nearlyEqual(velocity[3].y, 0.0));

    cfd::NeumannBC outletGradient("top", cells, 0.25);
    const cfd::ScalarField beforeNeumann = scalar;
    outletGradient.apply(scalar);
    assert(nearlyEqual(outletGradient.gradient(), 0.25));
    assert(nearlyEqual(scalar[1], beforeNeumann[1]));

    cfd::WallBC wall("wall", cells);
    wall.apply(velocity);
    assert(nearlyEqual(velocity[1].x, 0.0));
    assert(nearlyEqual(velocity[3].y, 0.0));

    cfd::OutletBC zeroGradientOutlet("outlet", cells);
    zeroGradientOutlet.apply(scalar);
    assert(!zeroGradientOutlet.hasPressure());

    cfd::OutletBC pressureOutlet("outlet", cells, 101325.0);
    pressureOutlet.apply(scalar);
    assert(pressureOutlet.hasPressure());
    assert(nearlyEqual(scalar[1], 101325.0));
    assert(nearlyEqual(*pressureOutlet.pressure(), 101325.0));

    bool threw = false;
    try
    {
        cfd::DirichletBC invalid("bad", {5}, 1.0);
        invalid.apply(scalar);
    }
    catch (const std::out_of_range&)
    {
        threw = true;
    }
    assert(threw);

    return 0;
}
