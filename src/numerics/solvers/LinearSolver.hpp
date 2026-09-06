#pragma once

#include "numerics/matrix/LinearSystem.hpp"
#include "numerics/solvers/LinearSolverResult.hpp"
#include "numerics/solvers/SolverSettings.hpp"

namespace cfd
{

class LinearSolver
{
public:
    explicit LinearSolver(SolverSettings settings = {})
        : settings_(settings)
    {
    }

    virtual ~LinearSolver() = default;
    virtual LinearSolverResult solve(LinearSystem& system) = 0;

protected:
    SolverSettings settings_;
};

}
