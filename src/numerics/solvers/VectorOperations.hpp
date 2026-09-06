#pragma once

#include <vector>

namespace cfd
{

double dot(const std::vector<double>& a, const std::vector<double>& b);
double norm(const std::vector<double>& values);
void axpy(double alpha, const std::vector<double>& x, std::vector<double>& y);

}
