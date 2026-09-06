#pragma once

#include "validation/ValidationResult.hpp"

#include <initializer_list>
#include <vector>

namespace cfd
{

struct GridRefinementSummary
{
    std::vector<ValidationResult> samples;
};

class GridRefinementAnalyzer
{
public:
    [[nodiscard]] GridRefinementSummary analyze(std::initializer_list<std::size_t> gridSizes) const;
};

}
