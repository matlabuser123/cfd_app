#pragma once

#include "numerics/solvers/LinearSolver.hpp"

namespace cfd
{

class BiCGSTABSolver final : public LinearSolver
{
public:
    explicit BiCGSTABSolver(SolverSettings settings = {});
    LinearSolverResult solve(LinearSystem& system) override;
};

}
