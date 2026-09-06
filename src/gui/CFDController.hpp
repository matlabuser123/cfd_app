#pragma once

#include "validation/ValidationCase.hpp"
#include "validation/ValidationResult.hpp"
#include "solver/simple/SIMPLEResult.hpp"
#include "gpu/GPUSettings.hpp"
#include "io/ResultsExporter.hpp"
#include "io/vtk/VTKReader.hpp"

#include <atomic>
#include <filesystem>
#include <functional>
#include <string>

namespace cfd
{

struct SolverBackendSelection
{
    ComputeBackend requested{ComputeBackend::Serial};
    ComputeBackend active{ComputeBackend::Serial};
    std::string status;
};

class CFDController
{
public:
    [[nodiscard]] ValidationCase loadValidationCase(const std::filesystem::path& caseDirectory) const;
    [[nodiscard]] VTKResult loadVTKResult(const std::filesystem::path& file) const;
    void exportVTKResult(const VTKResult& result, const std::filesystem::path& directory, const ResultExportOptions& options) const;
    [[nodiscard]] SolverBackendSelection configureBackend(ComputeBackend requestedBackend) const;
    [[nodiscard]] ValidationResult runValidation(
        const ValidationCase& validationCase,
        const std::atomic_bool* cancelRequested = nullptr,
        std::function<void(const SIMPLEIteration&)> iterationCallback = {}
    ) const;
};

}
