#pragma once

#include "io/Results.hpp"

#include <filesystem>

namespace cfd
{

class CSVWriter
{
public:
    static void writeFields(const CFDResults& results, const std::filesystem::path& file);
    static void writeResiduals(const std::vector<ResidualRecord>& residuals, const std::filesystem::path& file);
};

}
