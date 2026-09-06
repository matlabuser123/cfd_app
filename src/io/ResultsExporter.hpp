#pragma once

#include "io/Results.hpp"

#include <filesystem>

namespace cfd
{

struct ResultExportOptions
{
    bool csv{true};
    bool json{true};
    bool vtk{true};
};

class ResultsExporter
{
public:
    static void exportAll(const CFDResults& results, const std::filesystem::path& directory);
    static void exportSelected(const CFDResults& results, const std::filesystem::path& directory, const ResultExportOptions& options);
};

}
