#pragma once

#include <cstddef>
#include <vector>

namespace cfd
{

class ScalarField
{
public:
    ScalarField() = default;
    explicit ScalarField(std::size_t size, double value = 0.0);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    double& operator[](std::size_t index) noexcept;
    const double& operator[](std::size_t index) const noexcept;

    double& at(std::size_t index);
    const double& at(std::size_t index) const;

    void fill(double value) noexcept;

    [[nodiscard]] double min() const;
    [[nodiscard]] double max() const;
    [[nodiscard]] double l2Norm() const noexcept;

    ScalarField& operator+=(const ScalarField& other);
    ScalarField& operator-=(const ScalarField& other);
    ScalarField& operator*=(double value) noexcept;
    ScalarField& operator/=(double value);

private:
    std::vector<double> values_;
};

ScalarField operator+(ScalarField left, const ScalarField& right);
ScalarField operator-(ScalarField left, const ScalarField& right);
ScalarField operator*(ScalarField field, double value);
ScalarField operator*(double value, ScalarField field);
ScalarField operator/(ScalarField field, double value);

}
