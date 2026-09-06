#include <cassert>
#include <cmath>
#include <filesystem>

#include "gui/CFDController.hpp"
#include "io/ResultsExporter.hpp"

int main()
{
    const std::filesystem::path caseDirectory = std::filesystem::path(CFDAPP_SOURCE_DIR) / "cases" / "cavity_20x20";
    const cfd::ValidationCase validationCase = cfd::CFDController{}.loadValidationCase(caseDirectory);

    assert(validationCase.name == "lid_driven_cavity");
    assert(validationCase.nx == 20);
    assert(validationCase.ny == 20);
    assert(std::abs(validationCase.length - 1.0) < 1e-12);
    assert(std::abs(validationCase.height - 1.0) < 1e-12);
    assert(std::abs(validationCase.density - 1.0) < 1e-12);
    assert(std::abs(validationCase.viscosity - 0.01) < 1e-12);
    assert(std::abs(validationCase.lidVelocity - 1.0) < 1e-12);
    assert(std::abs(validationCase.momentumTolerance - 1e-8) < 1e-16);
    assert(std::abs(validationCase.pressureTolerance - 1e-8) < 1e-16);
    assert(validationCase.maxIterations == 1000);

    const cfd::CFDController controller;
    const cfd::SolverBackendSelection serialSelection = controller.configureBackend(cfd::ComputeBackend::Serial);
    assert(serialSelection.requested == cfd::ComputeBackend::Serial);
    assert(serialSelection.active == cfd::ComputeBackend::Serial);
    assert(!serialSelection.status.empty());

    const cfd::SolverBackendSelection openMPSelection = controller.configureBackend(cfd::ComputeBackend::OpenMP);
    assert(openMPSelection.requested == cfd::ComputeBackend::OpenMP);
#if CFDAPP_HAS_OPENMP
    assert(openMPSelection.active == cfd::ComputeBackend::OpenMP);
#else
    assert(openMPSelection.active == cfd::ComputeBackend::Serial);
#endif

    const cfd::SolverBackendSelection cudaSelection = controller.configureBackend(cfd::ComputeBackend::CUDA);
    assert(cudaSelection.requested == cfd::ComputeBackend::CUDA);
    assert(cudaSelection.active == cfd::ComputeBackend::Serial);
    assert(!cudaSelection.status.empty());

    cfd::CFDResults results;
    results.nx = 1;
    results.ny = 1;
    results.x = {0.5};
    results.y = {0.5};
    results.u = {1.0};
    results.v = {0.0};
    results.pressure = {2.0};
    results.velocityMagnitude = {1.0};
    results.vorticity = {0.0};
    const std::filesystem::path sourceDirectory = std::filesystem::temp_directory_path() / "cfd_gui_vtk_source";
    const std::filesystem::path exportDirectory = std::filesystem::temp_directory_path() / "cfd_gui_vtk_export";
    cfd::ResultsExporter::exportAll(results, sourceDirectory);
    const cfd::VTKResult vtkResult = controller.loadVTKResult(sourceDirectory / "fields.vtk");
    controller.exportVTKResult(vtkResult, exportDirectory, {false, true, false});
    assert(std::filesystem::exists(exportDirectory / "solution.json"));
    assert(!std::filesystem::exists(exportDirectory / "fields.csv"));
    assert(!std::filesystem::exists(exportDirectory / "fields.vtk"));
    std::filesystem::remove_all(sourceDirectory);
    std::filesystem::remove_all(exportDirectory);
    return 0;
}