#include <cassert>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "core/mesh/Mesh.hpp"
#include "gpu/GPUMatrixAssembly.hpp"
#include "numerics/matrix/LinearSystem.hpp"
#include "numerics/matrix/SparseMatrixBuilder.hpp"

namespace
{

bool nearlyEqual(double left, double right)
{
    return std::abs(left - right) < 1e-12;
}

}

int main()
{
    cfd::SparseMatrix matrix(3, 3);
    matrix.set(0, 0, 4.0);
    matrix.set(0, 1, 1.0);
    matrix.set(1, 1, 5.0);
    matrix.set(1, 2, 2.0);
    matrix.set(2, 0, 3.0);
    matrix.set(2, 2, 6.0);

    assert(matrix.rows() == 3);
    assert(matrix.cols() == 3);
    assert(matrix.nonZeroCount() == 6);
    assert(nearlyEqual(matrix.get(0, 0), 4.0));
    assert(nearlyEqual(matrix.get(0, 2), 0.0));

    matrix.add(0, 0, 2.0);
    assert(nearlyEqual(matrix.get(0, 0), 6.0));
    assert(nearlyEqual(matrix.diagonal(1), 5.0));
    assert(nearlyEqual(matrix.multiply({1.0, 2.0, 3.0})[0], 8.0));
    assert(nearlyEqual(matrix.multiply({1.0, 2.0, 3.0})[1], 16.0));
    assert(nearlyEqual(matrix.multiply({1.0, 2.0, 3.0})[2], 21.0));

    matrix.set(0, 1, 0.0);
    assert(matrix.nonZeroCount() == 5);
    matrix.add(0, 0, -6.0);
    assert(matrix.nonZeroCount() == 4);
    matrix.clear();
    assert(matrix.nonZeroCount() == 0);

    cfd::LinearSystem system(3);
    cfd::SparseMatrixBuilder builder(system.matrix());
    builder.setCoefficient(0, 0, 4.0);
    builder.addCoefficient(0, 1, 1.0);
    builder.addSource(system.rhs(), 0, 10.0);
    assert(system.size() == 3);
    assert(nearlyEqual(system.matrix().get(0, 1), 1.0));
    assert(nearlyEqual(system.rhs()[0], 10.0));

    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);
    cfd::SparseMatrix stencil(mesh.cellCount(), mesh.cellCount());
    for (std::size_t cell = 0; cell < mesh.cellCount(); ++cell)
    {
        stencil.set(cell, cell, 4.0);
        if (const auto east = mesh.east(cell)) stencil.set(cell, *east, -1.0);
        if (const auto west = mesh.west(cell)) stencil.set(cell, *west, -1.0);
        if (const auto north = mesh.north(cell)) stencil.set(cell, *north, -1.0);
        if (const auto south = mesh.south(cell)) stencil.set(cell, *south, -1.0);
    }
    const std::size_t center = 10 + 10 * mesh.nx();
    assert(stencil.rows() == 400);
    assert(stencil.cols() == 400);
    assert(stencil.nonZeroCount() > 400);
    assert(nearlyEqual(stencil.get(center, center), 4.0));
    assert(nearlyEqual(stencil.get(center, *mesh.west(center)), -1.0));
    assert(nearlyEqual(stencil.get(center, *mesh.east(center)), -1.0));
    assert(nearlyEqual(stencil.get(center, *mesh.south(center)), -1.0));
    assert(nearlyEqual(stencil.get(center, *mesh.north(center)), -1.0));

    const cfd::GPUCSRMatrix csr = cfd::GPUMatrixAssembly::toCSR(stencil);
    const cfd::GPUCSRMatrix repeatedCSR = cfd::GPUMatrixAssembly::toCSR(stencil);
    assert(csr.rows == stencil.rows());
    assert(csr.columns == stencil.cols());
    assert(csr.rowOffsets.size() == stencil.rows() + 1);
    assert(csr.rowOffsets.front() == 0);
    assert(csr.rowOffsets.back() == stencil.nonZeroCount());
    assert(csr.columnIndices.size() == stencil.nonZeroCount());
    assert(csr.values.size() == stencil.nonZeroCount());
    assert(csr.rowOffsets == repeatedCSR.rowOffsets);
    assert(csr.columnIndices == repeatedCSR.columnIndices);
    assert(csr.values == repeatedCSR.values);
    for (std::size_t row = 0; row < csr.rows; ++row)
    {
        for (std::size_t index = csr.rowOffsets[row]; index < csr.rowOffsets[row + 1]; ++index)
        {
            if (index > csr.rowOffsets[row]) assert(csr.columnIndices[index - 1] < csr.columnIndices[index]);
            assert(nearlyEqual(stencil.get(row, csr.columnIndices[index]), csr.values[index]));
        }
    }

    bool threw = false;
    try
    {
        static_cast<void>(matrix.get(3, 0));
    }
    catch (const std::out_of_range&)
    {
        threw = true;
    }
    assert(threw);

    threw = false;
    try
    {
        static_cast<void>(matrix.multiply({1.0}));
    }
    catch (const std::invalid_argument&)
    {
        threw = true;
    }
    assert(threw);

    return 0;
}
