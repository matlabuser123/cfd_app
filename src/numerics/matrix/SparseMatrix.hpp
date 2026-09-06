#pragma once

#include <atomic>
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace cfd
{

class SparseMatrix
{
public:
    SparseMatrix() = default;
    SparseMatrix(std::size_t rows, std::size_t cols);

    [[nodiscard]] std::size_t rows() const noexcept;
    [[nodiscard]] std::size_t cols() const noexcept;

    void clear() noexcept;
    void clearRow(std::size_t row);
    void scaleRow(std::size_t row, double factor);
    void set(std::size_t row, std::size_t col, double value);
    void add(std::size_t row, std::size_t col, double value);

    [[nodiscard]] double get(std::size_t row, std::size_t col) const;
    [[nodiscard]] const std::unordered_map<std::size_t, double>& rowEntries(std::size_t row) const;
    [[nodiscard]] double diagonal(std::size_t row) const;
    [[nodiscard]] std::size_t nonZeroCount() const noexcept;
    [[nodiscard]] std::vector<double> multiply(const std::vector<double>& x) const;
    void multiplyInto(const std::vector<double>& x, std::vector<double>& result) const;

private:
    void checkIndex(std::size_t row, std::size_t col) const;

    std::size_t rows_{0};
    std::size_t cols_{0};
    std::vector<std::unordered_map<std::size_t, double>> data_;
    std::atomic<std::size_t> nonZeroCount_{0};
};

}
