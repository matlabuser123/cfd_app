#pragma once

#include "core/fields/VectorField.hpp"

#include <map>
#include <optional>
#include <string>

namespace cfd
{

struct MeshConfig
{
    std::size_t nx{0};
    std::size_t ny{0};
    double width{1.0};
    double height{1.0};
};

struct FluidConfig
{
    std::string name;
    double density{0.0};
    double dynamicViscosity{0.0};
    Vector2 gravity{};
    bool compressible{false};
};

struct BoundarySpec
{
    std::string type;
    std::optional<double> value;
    std::optional<Vector2> vectorValue;
    std::optional<double> gradient;
};

struct InitialConditions
{
    Vector2 velocity{};
    double pressure{0.0};
};

struct NumericsConfig
{
    std::string algorithm;
    std::string momentumSolver;
    std::string pressureSolver;
    double momentumTolerance{0.0};
    double pressureTolerance{0.0};
    std::size_t maxIterations{0};
    double velocityUnderRelaxation{0.0};
    double pressureUnderRelaxation{0.0};
};

struct OutputConfig
{
    std::string directory;
    bool writeCsv{false};
    bool writeJson{false};
    bool writeVtk{false};
};

struct CaseConfig
{
    std::string name;
    int dimensions{0};
    MeshConfig mesh;
    FluidConfig fluid;
    std::map<std::string, BoundarySpec> boundaries;
    InitialConditions initialConditions;
    NumericsConfig numerics;
    OutputConfig output;
};

}
