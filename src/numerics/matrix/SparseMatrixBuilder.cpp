#include "numerics/matrix/SparseMatrixBuilder.hpp"

#include <stdexcept>

namespace cfd
{

SparseMatrixBuilder::SparseMatrixBuilder(SparseMatrix& matrix) noexcept
    : matrix_(matrix)
{
}

void SparseMatrixBuilder::addCoefficient(std::size_t row, std::size_t col, double value)
{
    matrix_.add(row, col, value);
}

void SparseMatrixBuilder::setCoefficient(std::size_t row, std::size_t col, double value)
{
    matrix_.set(row, col, value);
}

void SparseMatrixBuilder::addSource(std::vector<double>& rhs, std::size_t row, double value) const
{
    if (row >= rhs.size())
    {
        throw std::out_of_range("Source row index out of range");
    }
    rhs[row] += value;
}

}
