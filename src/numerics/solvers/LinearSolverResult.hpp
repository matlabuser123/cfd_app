#pragma once

#include <cstddef>
#include <string>

namespace cfd
{

struct LinearSolverResult
{
    bool converged{false};
    std::size_t iterations{0};
    double initialResidual{0.0};
    double finalResidual{0.0};
    std::string failureReason;
};

}
