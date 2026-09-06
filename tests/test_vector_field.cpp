#include <cassert>
#include <cmath>
#include <stdexcept>

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
    cfd::VectorField velocity(2, cfd::Vector2{1.0, -2.0});
    assert(velocity.size() == 2);
    assert(!velocity.empty());
    assert(nearlyEqual(velocity[0].x, 1.0));
    assert(nearlyEqual(velocity[0].y, -2.0));

    velocity.at(1) = cfd::Vector2{3.0, 4.0};
    assert(nearlyEqual(velocity.magnitude(0), std::sqrt(5.0)));
    assert(nearlyEqual(velocity.magnitude(1), 5.0));
    assert(nearlyEqual(velocity.minMagnitude(), std::sqrt(5.0)));
    assert(nearlyEqual(velocity.maxMagnitude(), 5.0));
    assert(nearlyEqual(velocity.l2Norm(), std::sqrt(30.0)));
    assert(nearlyEqual(velocity.componentMinX(), 1.0));
    assert(nearlyEqual(velocity.componentMaxX(), 3.0));
    assert(nearlyEqual(velocity.componentMinY(), -2.0));
    assert(nearlyEqual(velocity.componentMaxY(), 4.0));

    const cfd::VectorField copy = velocity;
    assert(nearlyEqual(copy[1].x, 3.0));

    cfd::VectorField moved = std::move(velocity);
    assert(moved.size() == 2);
    moved.fill(cfd::Vector2{5.0, 6.0});
    assert(nearlyEqual(moved[0].x, 5.0));
    assert(nearlyEqual(moved[1].y, 6.0));

    bool threw = false;
    try
    {
        moved.at(2);
    }
    catch (const std::out_of_range&)
    {
        threw = true;
    }
    assert(threw);

    threw = false;
    try
    {
        cfd::VectorField empty;
        static_cast<void>(empty.componentMinX());
    }
    catch (const std::logic_error&)
    {
        threw = true;
    }
    assert(threw);

    return 0;
}
