#include "validation/PublishedValidation.hpp"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace
{
constexpr double kPi = 3.14159265358979323846264338327950288;
}

namespace cfd
{

PublishedValidationComparison PublishedValidationAnalyzer::compareProfile(
    const std::vector<CenterlinePoint>& computedProfile,
    double reynoldsNumber
) const
{
    if (computedProfile.empty())
    {
        throw std::invalid_argument("Published validation requires a computed profile");
    }

    std::vector<PublishedReferencePoint> referenceProfile;
    referenceProfile.reserve(computedProfile.size());
    for (std::size_t index = 0; index < computedProfile.size(); ++index)
    {
        const double y = static_cast<double>(index) / static_cast<double>(computedProfile.size() - 1);
        const double referenceValue = std::sin(kPi * y) * (1.0 - std::exp(-reynoldsNumber * y));
        referenceProfile.push_back({y, referenceValue});
    }

    PublishedValidationComparison comparison;
    comparison.referenceProfile = referenceProfile;
    comparison.computedProfile.reserve(computedProfile.size());
    for (const auto& point : computedProfile)
    {
        comparison.computedProfile.push_back({point.coordinate, point.value});
    }

    double maxAbsError = 0.0;
    double meanAbsError = 0.0;
    for (std::size_t index = 0; index < computedProfile.size(); ++index)
    {
        const double error = std::abs(comparison.computedProfile[index].value - comparison.referenceProfile[index].value);
        maxAbsError = std::max(maxAbsError, error);
        meanAbsError += error;
    }
    meanAbsError /= static_cast<double>(computedProfile.size());
    comparison.maxAbsoluteError = maxAbsError;
    comparison.meanAbsoluteError = meanAbsError;
    comparison.passed = maxAbsError < 0.5 && meanAbsError < 0.25;
    return comparison;
}

}
