#pragma once

#include "core/fields/VectorField.hpp"
#include "io/Results.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>

namespace cfd
{

struct VTKResult
{
    std::size_t nx{0};
    std::size_t ny{0};
    double dx{0.0};
    double dy{0.0};
    std::vector<double> pressure;
    std::vector<double> velocityMagnitude;
    std::vector<double> vorticity;
    std::vector<Vector2> velocity;
};

class VTKReader
{
public:
    [[nodiscard]] static VTKResult read(const std::filesystem::path& file);
    [[nodiscard]] static CFDResults toResults(const VTKResult& result, std::string caseName = "imported_vtk");
};

}