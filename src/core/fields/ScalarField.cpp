#include "core/fields/ScalarField.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace cfd
{

ScalarField::ScalarField(std::size_t size, double value)
    : values_(size, value)
{
}

std::size_t ScalarField::size() const noexcept
{
    return values_.size();
}

bool ScalarField::empty() const noexcept
{
    return values_.empty();
}

double& ScalarField::operator[](std::size_t index) noexcept
{
    return values_[index];
}

const double& ScalarField::operator[](std::size_t index) const noexcept
{
    return values_[index];
}

double& ScalarField::at(std::size_t index)
{
    return values_.at(index);
}

const double& ScalarField::at(std::size_t index) const
{
    return values_.at(index);
}

void ScalarField::fill(double value) noexcept
{
    std::fill(values_.begin(), values_.end(), value);
}

double ScalarField::min() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute the minimum of an empty field");
    }

    return *std::min_element(values_.begin(), values_.end());
}

double ScalarField::max() const
{
    if (values_.empty())
    {
        throw std::logic_error("Cannot compute the maximum of an empty field");
    }

    return *std::max_element(values_.begin(), values_.end());
}

double ScalarField::l2Norm() const noexcept
{
    double sumOfSquares = 0.0;

    for (const double value : values_)
    {
        sumOfSquares += value * value;
    }

    return std::sqrt(sumOfSquares);
}

ScalarField& ScalarField::operator+=(const ScalarField& other)
{
    if (values_.size() != other.values_.size())
    {
        throw std::invalid_argument("Scalar fields must have the same size");
    }

    for (std::size_t index = 0; index < values_.size(); ++index)
    {
        values_[index] += other.values_[index];
    }

    return *this;
}

ScalarField& ScalarField::operator-=(const ScalarField& other)
{
    if (values_.size() != other.values_.size())
    {
        throw std::invalid_argument("Scalar fields must have the same size");
    }

    for (std::size_t index = 0; index < values_.size(); ++index)
    {
        values_[index] -= other.values_[index];
    }

    return *this;
}

ScalarField& ScalarField::operator*=(double value) noexcept
{
    for (double& fieldValue : values_)
    {
        fieldValue *= value;
    }

    return *this;
}

ScalarField& ScalarField::operator/=(double value)
{
    if (value == 0.0)
    {
        throw std::invalid_argument("Cannot divide a scalar field by zero");
    }

    for (double& fieldValue : values_)
    {
        fieldValue /= value;
    }

    return *this;
}

ScalarField operator+(ScalarField left, const ScalarField& right)
{
    return left += right;
}

ScalarField operator-(ScalarField left, const ScalarField& right)
{
    return left -= right;
}

ScalarField operator*(ScalarField field, double value)
{
    return field *= value;
}

ScalarField operator*(double value, ScalarField field)
{
    return field *= value;
}

ScalarField operator/(ScalarField field, double value)
{
    return field /= value;
}

}
