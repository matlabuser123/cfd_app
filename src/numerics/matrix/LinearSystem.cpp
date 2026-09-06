#include "numerics/matrix/LinearSystem.hpp"

namespace cfd
{

LinearSystem::LinearSystem(std::size_t size, std::size_t)
    : matrix_(size, size),
      rhs_(size, 0.0),
      solution_(size, 0.0)
{
}

SparseMatrix& LinearSystem::matrix() noexcept
{
    return matrix_;
}

const SparseMatrix& LinearSystem::matrix() const noexcept
{
    return matrix_;
}

std::vector<double>& LinearSystem::rhs() noexcept
{
    return rhs_;
}

const std::vector<double>& LinearSystem::rhs() const noexcept
{
    return rhs_;
}

std::vector<double>& LinearSystem::solution() noexcept
{
    return solution_;
}

const std::vector<double>& LinearSystem::solution() const noexcept
{
    return solution_;
}

std::size_t LinearSystem::size() const noexcept
{
    return rhs_.size();
}

}
