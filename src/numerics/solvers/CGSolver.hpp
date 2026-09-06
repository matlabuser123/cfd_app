#pragma once

#include "numerics/solvers/LinearSolver.hpp"

namespace cfd
{

class CGSolver final : public LinearSolver
{
public:
    explicit CGSolver(SolverSettings settings = {});
    LinearSolverResult solve(LinearSystem& system) override;
};

}
