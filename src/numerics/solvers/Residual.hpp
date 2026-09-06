#pragma once

#include "numerics/matrix/LinearSystem.hpp"

#include <vector>

namespace cfd
{

[[nodiscard]] std::vector<double> calculateResidual(const LinearSystem& system);
[[nodiscard]] double calculateResidualNorm(const LinearSystem& system);

}
