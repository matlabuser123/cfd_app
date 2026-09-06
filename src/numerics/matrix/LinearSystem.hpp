#pragma once

#include "numerics/matrix/SparseMatrix.hpp"

#include <cstddef>
#include <vector>

namespace cfd
{

class LinearSystem
{
public:
    explicit LinearSystem(std::size_t size, std::size_t nonZeros = 0);

    [[nodiscard]] SparseMatrix& matrix() noexcept;
    [[nodiscard]] const SparseMatrix& matrix() const noexcept;
    [[nodiscard]] std::vector<double>& rhs() noexcept;
    [[nodiscard]] const std::vector<double>& rhs() const noexcept;
    [[nodiscard]] std::vector<double>& solution() noexcept;
    [[nodiscard]] const std::vector<double>& solution() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    SparseMatrix matrix_;
    std::vector<double> rhs_;
    std::vector<double> solution_;
};

}
