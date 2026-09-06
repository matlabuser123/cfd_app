#pragma once

#include "core/case/CaseConfig.hpp"
#include "core/boundary/BoundaryCondition.hpp"

#include <map>
#include <memory>
#include <vector>

namespace cfd
{

class BoundaryFactory
{
public:
    using PatchCells = std::map<std::string, std::vector<std::size_t>>;

    [[nodiscard]] static std::vector<std::unique_ptr<BoundaryCondition>> create(
        const CaseConfig& config,
        const PatchCells& patchCells
    );
};

}
