#pragma once

#include "io/Results.hpp"

#include <filesystem>

namespace cfd
{

class VTKWriter
{
public:
    static void write(const CFDResults& results, const std::filesystem::path& file);
};

}
