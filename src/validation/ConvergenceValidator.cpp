#include "validation/ConvergenceValidator.hpp"

namespace cfd
{

bool ConvergenceValidator::converged(
    const ValidationResult& result,
    const ValidationCase& validationCase
) noexcept
{
    return result.iterations > 0
        && result.iterations <= validationCase.maxIterations
        && result.continuityResidual <= validationCase.continuityTolerance
        && result.uResidual <= validationCase.momentumTolerance
        && result.vResidual <= validationCase.momentumTolerance;
}

}
