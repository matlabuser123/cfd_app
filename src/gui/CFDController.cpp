#include "gui/CFDController.hpp"

#include "core/case/CaseLoader.hpp"
#include "gpu/GPUBackend.hpp"
#include "parallel/ParallelRuntime.hpp"
#include "validation/ValidationRunner.hpp"

namespace cfd
{

ValidationCase CFDController::loadValidationCase(const std::filesystem::path& caseDirectory) const
{
    const Case loadedCase = CaseLoader{}.load(caseDirectory);
    const CaseConfig& config = loadedCase.config();
    ValidationCase validationCase;
    validationCase.name = config.name;
    validationCase.nx = config.mesh.nx;
    validationCase.ny = config.mesh.ny;
    validationCase.length = config.mesh.width;
    validationCase.height = config.mesh.height;
    validationCase.density = config.fluid.density;
    validationCase.viscosity = config.fluid.dynamicViscosity;
    validationCase.momentumTolerance = config.numerics.momentumTolerance;
    validationCase.pressureTolerance = config.numerics.pressureTolerance;
    validationCase.maxIterations = config.numerics.maxIterations;
    if (const auto top = config.boundaries.find("top"); top != config.boundaries.end() && top->second.vectorValue)
    {
        validationCase.lidVelocity = top->second.vectorValue->x;
    }
    return validationCase;
}

VTKResult CFDController::loadVTKResult(const std::filesystem::path& file) const
{
    return VTKReader::read(file);
}

void CFDController::exportVTKResult(
    const VTKResult& result,
    const std::filesystem::path& directory,
    const ResultExportOptions& options
) const
{
    ResultsExporter::exportSelected(VTKReader::toResults(result), directory, options);
}

SolverBackendSelection CFDController::configureBackend(ComputeBackend requestedBackend) const
{
    if (requestedBackend == ComputeBackend::Serial)
    {
        ParallelRuntime::initialize({false, 1});
        return {requestedBackend, ComputeBackend::Serial, "Serial CPU backend selected"};
    }
    if (requestedBackend == ComputeBackend::OpenMP)
    {
        ParallelRuntime::initialize({true, 0});
#if CFDAPP_HAS_OPENMP
        return {requestedBackend, ComputeBackend::OpenMP, "OpenMP CPU backend selected"};
#else
        return {requestedBackend, ComputeBackend::Serial, "OpenMP is unavailable; serial CPU fallback selected"};
#endif
    }

    GPUSettings settings;
    settings.requestedBackend = ComputeBackend::CUDA;
    const GPUBackend cudaBackend(settings);
    ParallelRuntime::initialize({false, 1});
    if (cudaBackend.available())
    {
        return {requestedBackend, ComputeBackend::Serial, "CUDA device available; serial CPU selected because CUDA SIMPLE coupling is unavailable"};
    }
    return {requestedBackend, ComputeBackend::Serial, "CUDA unavailable; serial CPU fallback selected"};
}

ValidationResult CFDController::runValidation(
    const ValidationCase& validationCase,
    const std::atomic_bool* cancelRequested,
    std::function<void(const SIMPLEIteration&)> iterationCallback
) const
{
    return ValidationRunner{}.run(validationCase, cancelRequested, std::move(iterationCallback));
}

}
