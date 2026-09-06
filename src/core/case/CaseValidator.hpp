#pragma once

#include "core/case/CaseConfig.hpp"

namespace cfd
{

class CaseValidator
{
public:
    static void validate(const CaseConfig& config);
};

}
