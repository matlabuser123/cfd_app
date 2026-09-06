#include <cassert>
#include <cmath>
#include <stdexcept>

#include "core/fields/ScalarField.hpp"

namespace
{

bool nearlyEqual(double left, double right)
{
    return std::abs(left - right) < 1e-12;
}

}

int main()
{
    cfd::ScalarField pressure(3, 2.0);
    assert(pressure.size() == 3);
    assert(!pressure.empty());
    assert(nearlyEqual(pressure[0], 2.0));

    pressure.at(1) = 4.0;
    pressure.fill(3.0);
    pressure[2] = 5.0;
    assert(nearlyEqual(pressure.min(), 3.0));
    assert(nearlyEqual(pressure.max(), 5.0));
    assert(nearlyEqual(pressure.l2Norm(), std::sqrt(43.0)));

    const cfd::ScalarField copy = pressure;
    assert(copy.size() == pressure.size());
    assert(nearlyEqual(copy[2], 5.0));

    cfd::ScalarField moved = std::move(pressure);
    assert(moved.size() == 3);
    assert(nearlyEqual(moved[2], 5.0));

    cfd::ScalarField other(3, 1.0);
    const cfd::ScalarField sum = moved + other;
    assert(nearlyEqual(sum[0], 4.0));
    assert(nearlyEqual((sum * 2.0)[1], 8.0));
    assert(nearlyEqual((sum / 2.0)[2], 3.0));

    bool threw = false;
    try
    {
        moved.at(3);
    }
    catch (const std::out_of_range&)
    {
        threw = true;
    }
    assert(threw);

    threw = false;
    try
    {
        cfd::ScalarField empty;
        static_cast<void>(empty.min());
    }
    catch (const std::logic_error&)
    {
        threw = true;
    }
    assert(threw);

    threw = false;
    try
    {
        moved += cfd::ScalarField(2);
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    assert(threw);

    return 0;
}
