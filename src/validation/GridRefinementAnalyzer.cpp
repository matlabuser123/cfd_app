#include "validation/GridRefinementAnalyzer.hpp"

#include "validation/ValidationCase.hpp"
#include "validation/ValidationRunner.hpp"

#include <string>

namespace cfd
{

GridRefinementSummary GridRefinementAnalyzer::analyze(std::initializer_list<std::size_t> gridSizes) const
{
    GridRefinementSummary summary;
    summary.samples.reserve(gridSizes.size());

    for (const std::size_t gridSize : gridSizes)
    {
        if (gridSize == 0) continue;

        ValidationCase validationCase;
        validationCase.name = "grid_refinement_" + std::to_string(gridSize) + "x" + std::to_string(gridSize);
        validationCase.nx = gridSize;
        validationCase.ny = gridSize;
        validationCase.continuityTolerance = 100.0;
        validationCase.momentumTolerance = 10.0;
        validationCase.pressureTolerance = 50.0;
        validationCase.maxIterations = 5000;

        summary.samples.push_back(ValidationRunner{}.run(validationCase));
    }

    return summary;
}

}
