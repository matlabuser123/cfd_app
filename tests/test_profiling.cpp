#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

#include "core/boundary/DirichletBC.hpp"
#include "core/boundary/WallBC.hpp"
#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "profiling/ProfileReport.hpp"
#include "profiling/ProfileScope.hpp"
#include "solver/simple/SIMPLE.hpp"

int main()
{
    cfd::Profiler profiler;
    {
        cfd::ProfileScope scope(profiler, "manual_region");
        volatile double value = 0.0;
        for (int index = 0; index < 10000; ++index) value += index;
        assert(value >= 0.0);
    }
    assert(profiler.entries().at("manual_region").calls == 1);
    assert(profiler.entries().at("manual_region").totalMilliseconds >= 0.0);

    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);
    cfd::VectorField velocity(mesh.cellCount(), {0.0, 0.0});
    cfd::ScalarField pressure(mesh.cellCount(), 0.0);
    const std::vector<std::size_t> top{380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399};
    const std::vector<std::size_t> bottom{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    std::vector<std::size_t> left;
    std::vector<std::size_t> right;
    for (std::size_t row = 0; row < 20; ++row) { left.push_back(row * 20); right.push_back(row * 20 + 19); }
    cfd::DirichletBC lid("top", top, cfd::Vector2{1.0, 0.0});
    cfd::WallBC bottomWall("bottom", bottom);
    cfd::WallBC leftWall("left", left);
    cfd::WallBC rightWall("right", right);
    cfd::SIMPLE simple(mesh, 1.0, 0.01, {1, 10.0, 10.0, 100.0, 0.7, 0.3});
    simple.setBoundaryConditions({&lid, &bottomWall, &leftWall, &rightWall});
    simple.setProfiler(&profiler);
    const cfd::SIMPLEResult result = simple.solve(velocity, pressure);
    assert(result.iterations == 1);
    for (const std::string name : {"simple_iteration", "u_momentum_assembly", "u_linear_solve", "v_momentum_assembly", "v_linear_solve", "pressure_assembly", "pressure_linear_solve", "velocity_correction", "flux_correction", "residual_calculation"})
    {
        assert(profiler.entries().contains(name));
        assert(profiler.entries().at(name).calls == 1);
    }

    const std::filesystem::path jsonPath = std::filesystem::temp_directory_path() / "cfd_profile_test.json";
    const std::filesystem::path csvPath = std::filesystem::temp_directory_path() / "cfd_profile_test.csv";
    cfd::ProfileReport::writeJson(profiler, jsonPath, "cavity", 20, 20, result.iterations);
    cfd::ProfileReport::writeCsv(profiler, csvPath);
    std::ifstream json(jsonPath);
    std::string content((std::istreambuf_iterator<char>(json)), std::istreambuf_iterator<char>());
    assert(content.find("u_momentum_assembly") != std::string::npos);
    assert(std::filesystem::file_size(csvPath) > 0);
    json.close();
    std::filesystem::remove(jsonPath);
    std::filesystem::remove(csvPath);
    return 0;
}
