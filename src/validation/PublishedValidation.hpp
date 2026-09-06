#pragma once

#include "validation/ValidationRunner.hpp"

#include <vector>

namespace cfd
{

struct PublishedReferencePoint
{
    double coordinate{0.0};
    double value{0.0};
};

struct PublishedValidationComparison
{
    std::vector<PublishedReferencePoint> referenceProfile;
    std::vector<PublishedReferencePoint> computedProfile;
    double maxAbsoluteError{0.0};
    double meanAbsoluteError{0.0};
    bool passed{false};
};

class PublishedValidationAnalyzer
{
public:
    [[nodiscard]] PublishedValidationComparison compareProfile(
        const std::vector<CenterlinePoint>& computedProfile,
        double reynoldsNumber = 100.0
    ) const;
};

}
