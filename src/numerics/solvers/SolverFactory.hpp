#pragma once

#include "numerics/solvers/BiCGSTABSolver.hpp"
#include "numerics/solvers/CGSolver.hpp"

#include <memory>
#include <stdexcept>
#include <string>

namespace cfd
{

class SolverFactory
{
public:
    [[nodiscard]] static std::unique_ptr<LinearSolver> create(
        const std::string& name,
        SolverSettings settings = {})
    {
        if (name == "BiCGSTAB") return std::make_unique<BiCGSTABSolver>(settings);
        if (name == "CG") return std::make_unique<CGSolver>(settings);
        throw std::invalid_argument("Unknown linear solver: " + name);
    }
};

}
