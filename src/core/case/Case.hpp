#pragma once

#include "core/case/CaseConfig.hpp"

namespace cfd
{

class Case
{
public:
    explicit Case(CaseConfig config);

    [[nodiscard]] const CaseConfig& config() const noexcept;
    [[nodiscard]] const std::string& name() const noexcept;
    [[nodiscard]] int dimensions() const noexcept;

private:
    CaseConfig config_;
};

}
