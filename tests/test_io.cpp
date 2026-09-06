#include <cassert>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "io/ResultsExporter.hpp"
#include "io/vtk/VTKReader.hpp"

namespace
{

std::string readFile(const std::filesystem::path& path)
{
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

}

int main()
{
    cfd::CFDResults results;
    results.caseName = "io_test";
    results.nx = 2;
    results.ny = 2;
    results.x = {0.25, 0.75, 0.25, 0.75};
    results.y = {0.25, 0.25, 0.75, 0.75};
    results.u = {1.0, 2.0, 3.0, 4.0};
    results.v = {0.0, 0.0, 1.0, 1.0};
    results.pressure = {5.0, 6.0, 7.0, 8.0};
    results.velocityMagnitude = {1.0, 2.0, 3.0, 4.0};
    results.vorticity = {0.0, 1.0, 2.0, 3.0};
    results.residualHistory = {{1, 2.0, 3.0, 4.0}};

    const std::filesystem::path directory = std::filesystem::temp_directory_path() / "cfd_io_test";
    const std::filesystem::path repeatedDirectory = std::filesystem::temp_directory_path() / "cfd_io_test_repeat";
    cfd::ResultsExporter::exportAll(results, directory);
    cfd::ResultsExporter::exportAll(results, repeatedDirectory);
    assert(std::filesystem::exists(directory / "fields.csv"));
    assert(std::filesystem::exists(directory / "residuals.csv"));
    assert(std::filesystem::exists(directory / "solution.json"));
    assert(std::filesystem::exists(directory / "fields.vtk"));

    assert(readFile(directory / "fields.csv") ==
        "i,j,x,y,u,v,pressure,velocity_magnitude,vorticity\n"
        "0,0,0.25,0.25,1,0,5,1,0\n"
        "1,0,0.75,0.25,2,0,6,2,1\n"
        "0,1,0.25,0.75,3,1,7,3,2\n"
        "1,1,0.75,0.75,4,1,8,4,3\n");
    assert(readFile(directory / "residuals.csv") ==
        "iteration,continuity,u_momentum,v_momentum\n"
        "1,2,3,4\n");
    assert(readFile(directory / "solution.json") ==
        "{\n"
        "  \"case\": \"io_test\",\n"
        "  \"mesh\": {\"nx\": 2, \"ny\": 2},\n"
        "  \"geometry\": {\"length\": 1, \"height\": 1},\n"
        "  \"physics\": {\"rho\": 1, \"mu\": 0.01, \"reynolds\": 100},\n"
        "  \"solver\": {\"iterations\": 0, \"continuity_residual\": 0, \"u_residual\": 0, \"v_residual\": 0}\n"
        "}\n");
    assert(readFile(directory / "fields.vtk") ==
        "# vtk DataFile Version 3.0\n"
        "CFD solution\n"
        "ASCII\n"
        "DATASET STRUCTURED_POINTS\n"
        "DIMENSIONS 3 3 1\n"
        "ORIGIN 0 0 0\n"
        "SPACING 0.5 0.5 1\n"
        "CELL_DATA 4\n"
        "SCALARS pressure double 1\n"
        "LOOKUP_TABLE default\n"
        "5\n6\n7\n8\n"
        "SCALARS velocity_magnitude double 1\n"
        "LOOKUP_TABLE default\n"
        "1\n2\n3\n4\n"
        "SCALARS vorticity double 1\n"
        "LOOKUP_TABLE default\n"
        "0\n1\n2\n3\n"
        "VECTORS velocity double\n"
        "1 0 0\n2 0 0\n3 1 0\n4 1 0\n");

    const cfd::VTKResult vtkResult = cfd::VTKReader::read(directory / "fields.vtk");
    assert(vtkResult.nx == 2);
    assert(vtkResult.ny == 2);
    assert(vtkResult.pressure == results.pressure);
    assert(vtkResult.velocityMagnitude == results.velocityMagnitude);
    assert(vtkResult.vorticity == results.vorticity);
    assert(vtkResult.velocity[2].x == results.u[2]);
    assert(vtkResult.velocity[2].y == results.v[2]);
    const cfd::CFDResults importedResults = cfd::VTKReader::toResults(vtkResult, "imported");
    assert(importedResults.caseName == "imported");
    assert(importedResults.x == results.x);
    assert(importedResults.y == results.y);
    assert(importedResults.u == results.u);
    assert(importedResults.v == results.v);

    const std::filesystem::path csvDirectory = std::filesystem::temp_directory_path() / "cfd_io_csv_only";
    cfd::ResultsExporter::exportSelected(results, csvDirectory, {true, false, false});
    assert(std::filesystem::exists(csvDirectory / "fields.csv"));
    assert(std::filesystem::exists(csvDirectory / "residuals.csv"));
    assert(!std::filesystem::exists(csvDirectory / "solution.json"));
    assert(!std::filesystem::exists(csvDirectory / "fields.vtk"));

    for (const char* fileName : {"fields.csv", "residuals.csv", "solution.json", "fields.vtk"})
    {
        assert(readFile(directory / fileName) == readFile(repeatedDirectory / fileName));
    }

    std::filesystem::remove_all(directory);
    std::filesystem::remove_all(repeatedDirectory);
    std::filesystem::remove_all(csvDirectory);
    return 0;
}
