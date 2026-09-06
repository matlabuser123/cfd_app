#include "core/mesh/Mesh.hpp"

namespace cfd
{

void Mesh::generateUniform(
    std::size_t nx,
    std::size_t ny,
    double width,
    double height
)
{
    nx_ = nx;
    ny_ = ny;
    width_ = width;
    height_ = height;

    cells_.clear();
    cells_.reserve(nx * ny);

    for (std::size_t j = 0; j < ny; ++j)
    {
        for (std::size_t i = 0; i < nx; ++i)
        {
            cells_.push_back(Cell{j * nx + i});
        }
    }
}

std::size_t Mesh::cellCount() const noexcept
{
    return cells_.size();
}

std::size_t Mesh::nx() const noexcept
{
    return nx_;
}

std::size_t Mesh::ny() const noexcept
{
    return ny_;
}

double Mesh::dx() const noexcept
{
    return nx_ == 0 ? 0.0 : width_ / static_cast<double>(nx_);
}

double Mesh::dy() const noexcept
{
    return ny_ == 0 ? 0.0 : height_ / static_cast<double>(ny_);
}

std::optional<std::size_t> Mesh::east(std::size_t cell) const noexcept
{
    if (cell >= cellCount() || nx_ == 0 || cell % nx_ == nx_ - 1) return std::nullopt;
    return cell + 1;
}

std::optional<std::size_t> Mesh::west(std::size_t cell) const noexcept
{
    if (cell >= cellCount() || nx_ == 0 || cell % nx_ == 0) return std::nullopt;
    return cell - 1;
}

std::optional<std::size_t> Mesh::north(std::size_t cell) const noexcept
{
    if (cell >= cellCount() || nx_ == 0 || cell / nx_ == ny_ - 1) return std::nullopt;
    return cell + nx_;
}

std::optional<std::size_t> Mesh::south(std::size_t cell) const noexcept
{
    if (cell >= cellCount() || nx_ == 0 || cell / nx_ == 0) return std::nullopt;
    return cell - nx_;
}

const std::vector<Cell>& Mesh::cells() const noexcept
{
    return cells_;
}

}
