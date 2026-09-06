#pragma once

#include "parallel/ParallelRuntime.hpp"

#include <cstddef>
#include <vector>

namespace cfd
{

double parallelSum(const std::vector<double>& values);
double parallelDot(const std::vector<double>& left, const std::vector<double>& right);

}
