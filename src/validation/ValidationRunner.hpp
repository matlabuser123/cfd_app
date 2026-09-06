#pragma once

#include "validation/ValidationCase.hpp"
#include "validation/ValidationResult.hpp"

#include "core/fields/ScalarField.hpp"
#include "solver/simple/SIMPLEResult.hpp"

#include <atomic>
#include <functional>
#include <vector>

namespace cfd
{

class ValidationRunner
{
public:
    [[nodiscard]] ValidationResult run(
        const ValidationCase& validationCase,
        const std::atomic_bool* cancelRequested = nullptr,
        std::function<void(const SIMPLEIteration&)> iterationCallback = {}
    ) const;
};

struct CenterlinePoint
{
    double coordinate{0.0};
    double value{0.0};
};

[[nodiscard]] std::vector<CenterlinePoint> extractVerticalUProfile(
    const ScalarField& velocityU,
    std::size_t nx,
    std::size_t ny,
    double dy
);

}
