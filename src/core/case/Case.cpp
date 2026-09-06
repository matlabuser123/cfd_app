#include "core/case/Case.hpp"

#include <utility>

namespace cfd
{

Case::Case(CaseConfig config)
    : config_(std::move(config))
{
}

const CaseConfig& Case::config() const noexcept
{
    return config_;
}

const std::string& Case::name() const noexcept
{
    return config_.name;
}

int Case::dimensions() const noexcept
{
    return config_.dimensions;
}

}
