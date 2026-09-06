#include "io/vtk/VTKReader.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

namespace cfd
{
namespace
{

void expect(std::istream& input, const char* expected)
{
    std::string value;
    input >> value;
    if (value != expected) throw std::runtime_error(std::string("Invalid VTK file: expected ") + expected);
}

std::vector<double> readScalar(std::istream& input, const char* expectedName, std::size_t cells)
{
    std::string keyword;
    std::string name;
    std::string type;
    std::size_t components = 0;
    input >> keyword >> name >> type >> components;
    if (keyword != "SCALARS" || name != expectedName || components != 1) throw std::runtime_error("Invalid VTK scalar field");
    expect(input, "LOOKUP_TABLE");
    std::string lookupTable;
    input >> lookupTable;

    std::vector<double> values(cells);
    for (double& value : values)
    {
        if (!(input >> value)) throw std::runtime_error("Invalid VTK scalar value");
    }
    return values;
}

}

VTKResult VTKReader::read(const std::filesystem::path& file)
{
    std::ifstream input(file, std::ios::binary);
    if (!input) throw std::runtime_error("Unable to open VTK result file: " + file.string());

    std::string header;
    std::string title;
    std::string format;
    std::string dataset;
    std::getline(input, header);
    std::getline(input, title);
    std::getline(input, format);
    std::getline(input, dataset);
    if (header != "# vtk DataFile Version 3.0" || format != "ASCII" || dataset != "DATASET STRUCTURED_POINTS")
    {
        throw std::runtime_error("Unsupported VTK result format");
    }

    expect(input, "DIMENSIONS");
    std::size_t pointsX = 0;
    std::size_t pointsY = 0;
    std::size_t pointsZ = 0;
    input >> pointsX >> pointsY >> pointsZ;
    if (pointsX < 2 || pointsY < 2 || pointsZ != 1) throw std::runtime_error("Invalid VTK dimensions");
    expect(input, "ORIGIN");
    double originX = 0.0;
    double originY = 0.0;
    double originZ = 0.0;
    input >> originX >> originY >> originZ;
    expect(input, "SPACING");
    VTKResult result;
    input >> result.dx >> result.dy >> originZ;
    if (result.dx <= 0.0 || result.dy <= 0.0) throw std::runtime_error("Invalid VTK spacing");
    result.nx = pointsX - 1;
    result.ny = pointsY - 1;
    const std::size_t cells = result.nx * result.ny;
    expect(input, "CELL_DATA");
    std::size_t cellCount = 0;
    input >> cellCount;
    if (cellCount != cells) throw std::runtime_error("VTK cell count does not match dimensions");

    result.pressure = readScalar(input, "pressure", cells);
    result.velocityMagnitude = readScalar(input, "velocity_magnitude", cells);
    result.vorticity = readScalar(input, "vorticity", cells);
    expect(input, "VECTORS");
    std::string vectorName;
    std::string vectorType;
    input >> vectorName >> vectorType;
    if (vectorName != "velocity") throw std::runtime_error("Invalid VTK velocity field");
    result.velocity.resize(cells);
    for (Vector2& value : result.velocity)
    {
        double z = 0.0;
        if (!(input >> value.x >> value.y >> z)) throw std::runtime_error("Invalid VTK velocity value");
    }
    return result;
}

CFDResults VTKReader::toResults(const VTKResult& result, std::string caseName)
{
    const std::size_t cells = result.nx * result.ny;
    if (result.nx == 0 || result.ny == 0 || result.dx <= 0.0 || result.dy <= 0.0
        || result.pressure.size() != cells || result.velocityMagnitude.size() != cells
        || result.vorticity.size() != cells || result.velocity.size() != cells)
    {
        throw std::invalid_argument("VTK result fields are inconsistent");
    }

    CFDResults converted;
    converted.caseName = std::move(caseName);
    converted.nx = result.nx;
    converted.ny = result.ny;
    converted.length = result.dx * static_cast<double>(result.nx);
    converted.height = result.dy * static_cast<double>(result.ny);
    converted.x.resize(cells);
    converted.y.resize(cells);
    converted.u.resize(cells);
    converted.v.resize(cells);
    for (std::size_t row = 0; row < result.ny; ++row)
    {
        for (std::size_t column = 0; column < result.nx; ++column)
        {
            const std::size_t index = row * result.nx + column;
            converted.x[index] = (static_cast<double>(column) + 0.5) * result.dx;
            converted.y[index] = (static_cast<double>(row) + 0.5) * result.dy;
            converted.u[index] = result.velocity[index].x;
            converted.v[index] = result.velocity[index].y;
        }
    }
    converted.pressure = result.pressure;
    converted.velocityMagnitude = result.velocityMagnitude;
    converted.vorticity = result.vorticity;
    return converted;
}

}