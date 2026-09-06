#pragma once

#include "core/case/Case.hpp"

#include <filesystem>

namespace cfd
{

class CaseLoader
{
public:
    [[nodiscard]] Case load(const std::filesystem::path& caseDirectory) const;
};

}
