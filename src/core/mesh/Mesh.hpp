#pragma once

#include <cstddef>
#include <optional>
#include <vector>

namespace cfd
{

struct Cell
{
    std::size_t id{};
};

class Mesh
{
public:
    Mesh() = default;

    void generateUniform(
        std::size_t nx,
        std::size_t ny,
        double width = 1.0,
        double height = 1.0
    );

    [[nodiscard]] std::size_t cellCount() const noexcept;
    [[nodiscard]] std::size_t nx() const noexcept;
    [[nodiscard]] std::size_t ny() const noexcept;
    [[nodiscard]] double dx() const noexcept;
    [[nodiscard]] double dy() const noexcept;
    [[nodiscard]] std::optional<std::size_t> east(std::size_t cell) const noexcept;
    [[nodiscard]] std::optional<std::size_t> west(std::size_t cell) const noexcept;
    [[nodiscard]] std::optional<std::size_t> north(std::size_t cell) const noexcept;
    [[nodiscard]] std::optional<std::size_t> south(std::size_t cell) const noexcept;
    [[nodiscard]] const std::vector<Cell>& cells() const noexcept;

private:
    std::size_t nx_{0};
    std::size_t ny_{0};
    double width_{1.0};
    double height_{1.0};
    std::vector<Cell> cells_;
};

}
