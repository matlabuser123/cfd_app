#include "core/fields/VectorField.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace cfd
{

VectorField::VectorField(std::size_t size, Vector2 value)
    : values_(size, value)
{
}

std::size_t VectorField::size() const noexcept
{
    return values_.size();
}

bool VectorField::empty() const noexcept
{
    return values_.empty();
}

Vector2& VectorField::operator[](std::size_t index) noexcept
{
    return values_[index];
}

const Vector2& VectorField::operator[](std::size_t index) const noexcept
{
    return values_[index];
}

Vector2& VectorField::at(std::size_t index)
{
    return values_.at(index);
}

const Vector2& VectorField::at(std::size_t index) const
{
    return values_.at(index);
}

void VectorField::fill(Vector2 value) noexcept
{
    std::fill(values_.begin(), values_.end(), value);
}

double VectorField::magnitude(std::size_t index) const
{
    const Vector2 value = at(index);
    return std::hypot(value.x, value.y);
}

double VectorField::minMagnitude() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute extrema of an empty field");
    }

    double minimum = magnitude(0);
    for (std::size_t index = 1; index < values_.size(); ++index)
    {
        minimum = std::min(minimum, magnitude(index));
    }

    return minimum;
}

double VectorField::maxMagnitude() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute extrema of an empty field");
    }

    double maximum = magnitude(0);
    for (std::size_t index = 1; index < values_.size(); ++index)
    {
        maximum = std::max(maximum, magnitude(index));
    }

    return maximum;
}

double VectorField::l2Norm() const noexcept
{
    double sumOfSquares = 0.0;

    for (const Vector2 value : values_)
    {
        sumOfSquares += value.x * value.x + value.y * value.y;
    }

    return std::sqrt(sumOfSquares);
}

double VectorField::componentMinX() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute extrema of an empty field");
    }

    return std::min_element(values_.begin(), values_.end(),
        [](const Vector2 left, const Vector2 right)
        {
            return left.x < right.x;
        })->x;
}

double VectorField::componentMaxX() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute extrema of an empty field");
    }

    return std::max_element(values_.begin(), values_.end(),
        [](const Vector2 left, const Vector2 right)
        {
            return left.x < right.x;
        })->x;
}

double VectorField::componentMinY() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute extrema of an empty field");
    }

    return std::min_element(values_.begin(), values_.end(),
        [](const Vector2 left, const Vector2 right)
        {
            return left.y < right.y;
        })->y;
}

double VectorField::componentMaxY() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute extrema of an empty field");
    }

    return std::max_element(values_.begin(), values_.end(),
        [](const Vector2 left, const Vector2 right)
        {
            return left.y < right.y;
        })->y;
}

}
