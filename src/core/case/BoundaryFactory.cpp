#include "core/case/BoundaryFactory.hpp"

#include "core/boundary/DirichletBC.hpp"
#include "core/boundary/InletBC.hpp"
#include "core/boundary/NeumannBC.hpp"
#include "core/boundary/OutletBC.hpp"
#include "core/boundary/WallBC.hpp"

#include <stdexcept>

namespace cfd
{

std::vector<std::unique_ptr<BoundaryCondition>> BoundaryFactory::create(
    const CaseConfig& config,
    const PatchCells& patchCells
)
{
    std::vector<std::unique_ptr<BoundaryCondition>> result;
    for (const auto& [patch, spec] : config.boundaries)
    {
        const auto cells = patchCells.find(patch);
        if (cells == patchCells.end()) throw std::invalid_argument("Missing mesh patch: " + patch);
        if (spec.type == "wall") result.push_back(std::make_unique<WallBC>(patch, cells->second));
        else if (spec.type == "velocity") result.push_back(std::make_unique<InletBC>(patch, cells->second, *spec.vectorValue));
        else if (spec.type == "pressure") result.push_back(std::make_unique<OutletBC>(patch, cells->second, *spec.value));
        else if (spec.type == "neumann") result.push_back(std::make_unique<NeumannBC>(patch, cells->second, *spec.gradient));
        else if (spec.type == "dirichlet") result.push_back(std::make_unique<DirichletBC>(patch, cells->second, *spec.value));
        else throw std::invalid_argument("Unsupported boundary type: " + spec.type);
    }
    return result;
}

}
