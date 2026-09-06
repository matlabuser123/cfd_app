#pragma once

#include "gpu/GPUSettings.hpp"

#include <string>

namespace cfd
{

class CUDAContext
{
public:
    explicit CUDAContext(GPUSettings settings = {});

    [[nodiscard]] bool available() const noexcept;
    [[nodiscard]] const std::string& status() const noexcept;

private:
    bool available_{false};
    std::string status_;
};

}
