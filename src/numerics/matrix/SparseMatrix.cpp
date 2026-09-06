#include "numerics/matrix/SparseMatrix.hpp"

#include <cmath>
#include <stdexcept>

namespace cfd
{
namespace
{
constexpr double zeroTolerance = 1e-15;
}

SparseMatrix::SparseMatrix(std::size_t rows, std::size_t cols)
    : rows_(rows),
      cols_(cols),
      data_(rows)
{
}

std::size_t SparseMatrix::rows() const noexcept
{
    return rows_;
}

std::size_t SparseMatrix::cols() const noexcept
{
    return cols_;
}

void SparseMatrix::clear() noexcept
{
    for (auto& row : data_)
    {
        row.clear();
    }
    nonZeroCount_ = 0;
}

void SparseMatrix::clearRow(std::size_t row)
{
    if (row >= rows_)
    {
        throw std::out_of_range("Sparse matrix row index out of range");
    }

    nonZeroCount_ -= data_[row].size();
    data_[row].clear();
}

void SparseMatrix::scaleRow(std::size_t row, double factor)
{
    if (row >= rows_) throw std::out_of_range("Sparse matrix row index out of range");
    if (!std::isfinite(factor)) throw std::invalid_argument("Sparse matrix row scale must be finite");
    for (auto& [column, value] : data_[row])
    {
        value *= factor;
    }
}

void SparseMatrix::checkIndex(std::size_t row, std::size_t col) const
{
    if (row >= rows_ || col >= cols_)
    {
        throw std::out_of_range("Sparse matrix index out of range");
    }
}

void SparseMatrix::set(std::size_t row, std::size_t col, double value)
{
    checkIndex(row, col);
    auto& matrixRow = data_[row];
    const auto it = matrixRow.find(col);

    if (std::abs(value) <= zeroTolerance)
    {
        if (it != matrixRow.end())
        {
            matrixRow.erase(it);
            --nonZeroCount_;
        }
        return;
    }

    if (it == matrixRow.end())
    {
        matrixRow.emplace(col, value);
        ++nonZeroCount_;
    }
    else
    {
        it->second = value;
    }
}

void SparseMatrix::add(std::size_t row, std::size_t col, double value)
{
    checkIndex(row, col);
    if (std::abs(value) <= zeroTolerance)
    {
        return;
    }

    auto& matrixRow = data_[row];
    const auto it = matrixRow.find(col);
    if (it == matrixRow.end())
    {
        matrixRow.emplace(col, value);
        ++nonZeroCount_;
        return;
    }

    it->second += value;
    if (std::abs(it->second) <= zeroTolerance)
    {
        matrixRow.erase(it);
        --nonZeroCount_;
    }
}

double SparseMatrix::get(std::size_t row, std::size_t col) const
{
    checkIndex(row, col);
    const auto& matrixRow = data_[row];
    const auto it = matrixRow.find(col);
    return it == matrixRow.end() ? 0.0 : it->second;
}

const std::unordered_map<std::size_t, double>& SparseMatrix::rowEntries(std::size_t row) const
{
    if (row >= rows_) throw std::out_of_range("Sparse matrix row index out of range");
    return data_[row];
}

double SparseMatrix::diagonal(std::size_t row) const
{
    if (row >= rows_ || row >= cols_)
    {
        throw std::out_of_range("Sparse matrix diagonal index out of range");
    }
    return get(row, row);
}

std::size_t SparseMatrix::nonZeroCount() const noexcept
{
    return nonZeroCount_.load();
}

std::vector<double> SparseMatrix::multiply(const std::vector<double>& x) const
{
    std::vector<double> result;
    multiplyInto(x, result);
    return result;
}

void SparseMatrix::multiplyInto(const std::vector<double>& x, std::vector<double>& result) const
{
    if (x.size() != cols_) throw std::invalid_argument("Vector size does not match matrix columns");
    if (result.size() != rows_) result.resize(rows_);
    std::fill(result.begin(), result.end(), 0.0);
    for (std::size_t row = 0; row < rows_; ++row)
    {
        for (const auto& [column, value] : data_[row])
        {
            result[row] += value * x[column];
        }
    }
}

}
