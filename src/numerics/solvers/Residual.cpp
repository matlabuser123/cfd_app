#include "numerics/solvers/Residual.hpp"

#include "numerics/solvers/VectorOperations.hpp"

namespace cfd
{

std::vector<double> calculateResidual(const LinearSystem& system)
{
    const std::vector<double> Ax = system.matrix().multiply(system.solution());
    std::vector<double> residual(system.rhs().size());
    for (std::size_t index = 0; index < residual.size(); ++index)
    {
        residual[index] = system.rhs()[index] - Ax[index];
    }
    return residual;
}

double calculateResidualNorm(const LinearSystem& system)
{
    return norm(calculateResidual(system));
}

}
