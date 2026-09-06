#include "numerics/solvers/VectorOperations.hpp"

#include <cmath>
#include <stdexcept>

namespace cfd
{

double dot(const std::vector<double>& a, const std::vector<double>& b)
{
    if (a.size() != b.size()) throw std::invalid_argument("Vector sizes do not match");
    double result = 0.0;
    for (std::size_t index = 0; index < a.size(); ++index) result += a[index] * b[index];
    return result;
}

double norm(const std::vector<double>& values)
{
    return std::sqrt(dot(values, values));
}

void axpy(double alpha, const std::vector<double>& x, std::vector<double>& y)
{
    if (x.size() != y.size()) throw std::invalid_argument("Vector sizes do not match");
    for (std::size_t index = 0; index < x.size(); ++index) y[index] += alpha * x[index];
}

}
