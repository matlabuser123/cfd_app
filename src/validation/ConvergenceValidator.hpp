#pragma once

#include "validation/ValidationCase.hpp"
#include "validation/ValidationResult.hpp"

namespace cfd
{

class ConvergenceValidator
{
public:
    [[nodiscard]] static bool converged(
        const ValidationResult& result,
        const ValidationCase& validationCase
    ) noexcept;
};

}
