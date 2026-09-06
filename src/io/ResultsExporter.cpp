#include "io/ResultsExporter.hpp"

#include "io/csv/CSVWriter.hpp"
#include "io/json/JSONWriter.hpp"
#include "io/vtk/VTKWriter.hpp"

#include <filesystem>

namespace cfd
{

void ResultsExporter::exportAll(const CFDResults& results, const std::filesystem::path& directory)
{
    exportSelected(results, directory, {});
}

void ResultsExporter::exportSelected(const CFDResults& results, const std::filesystem::path& directory, const ResultExportOptions& options)
{
    std::filesystem::create_directories(directory);
    if (options.csv)
    {
        CSVWriter::writeFields(results, directory / "fields.csv");
        CSVWriter::writeResiduals(results.residualHistory, directory / "residuals.csv");
    }
    if (options.json) JSONWriter::write(results, directory / "solution.json");
    if (options.vtk) VTKWriter::write(results, directory / "fields.vtk");
}

}
