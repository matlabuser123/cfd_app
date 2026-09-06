#include <cassert>
#include <cmath>
#include <filesystem>
#include <stdexcept>

#include "core/case/BoundaryFactory.hpp"
#include "core/case/CaseLoader.hpp"
#include "core/case/CaseValidator.hpp"

int main()
{
    const std::filesystem::path caseDirectory = std::filesystem::path(CFDAPP_SOURCE_DIR) / "cases" / "cavity_20x20";
    const cfd::Case loaded = cfd::CaseLoader{}.load(caseDirectory);
    const cfd::CaseConfig& config = loaded.config();

    assert(loaded.name() == "lid_driven_cavity");
    assert(loaded.dimensions() == 2);
    assert(config.mesh.nx == 20);
    assert(config.mesh.ny == 20);
    assert(std::abs(config.fluid.dynamicViscosity - 0.01) < 1e-12);
    assert(config.boundaries.size() == 4);
    assert(config.numerics.maxIterations == 1000);

    cfd::BoundaryFactory::PatchCells patches{
        {"top", {380, 381}}, {"bottom", {0, 1}}, {"left", {0, 20}}, {"right", {19, 39}}
    };
    const auto boundaryConditions = cfd::BoundaryFactory::create(config, patches);
    assert(boundaryConditions.size() == 4);

    bool rejected = false;
    try
    {
        cfd::CaseConfig invalidConfig = config;
        invalidConfig.fluid.dynamicViscosity = -0.01;
        cfd::CaseValidator::validate(invalidConfig);
    }
    catch (const std::invalid_argument&)
    {
        rejected = true;
    }
    assert(rejected);

    return 0;
}
