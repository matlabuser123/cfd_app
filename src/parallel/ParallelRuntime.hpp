#pragma once

#include "parallel/ParallelSettings.hpp"

namespace cfd
{

class ParallelRuntime
{
public:
    static void initialize(const ParallelSettings& settings);
    [[nodiscard]] static bool enabled() noexcept;
    [[nodiscard]] static int threadCount() noexcept;
};

}
