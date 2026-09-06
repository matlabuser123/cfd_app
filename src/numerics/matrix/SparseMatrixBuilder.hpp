#pragma once

#include "numerics/matrix/SparseMatrix.hpp"

#include <cstddef>
#include <vector>

namespace cfd
{

class SparseMatrixBuilder
{
public:
    explicit SparseMatrixBuilder(SparseMatrix& matrix) noexcept;

    void addCoefficient(std::size_t row, std::size_t col, double value);
    void setCoefficient(std::size_t row, std::size_t col, double value);
    void addSource(std::vector<double>& rhs, std::size_t row, double value) const;

private:
    SparseMatrix& matrix_;
};

}
