#pragma once

#include <cstddef>
#include <vector>

namespace cfd
{

struct Vector2
{
    double x{};
    double y{};
};

class VectorField
{
public:
    VectorField() = default;
    explicit VectorField(std::size_t size, Vector2 value = {});

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;

    Vector2& operator[](std::size_t index) noexcept;
    const Vector2& operator[](std::size_t index) const noexcept;

    Vector2& at(std::size_t index);
    const Vector2& at(std::size_t index) const;

    void fill(Vector2 value) noexcept;

    [[nodiscard]] double magnitude(std::size_t index) const;
    [[nodiscard]] double minMagnitude() const;
    [[nodiscard]] double maxMagnitude() const;
    [[nodiscard]] double l2Norm() const noexcept;
    [[nodiscard]] double componentMinX() const;
    [[nodiscard]] double componentMaxX() const;
    [[nodiscard]] double componentMinY() const;
    [[nodiscard]] double componentMaxY() const;

private:
    std::vector<Vector2> values_;
};

}
