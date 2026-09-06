#pragma once

#include "solver/simple/SIMPLESettings.hpp"
#include "solver/simple/SIMPLEResult.hpp"

#include "core/boundary/BoundaryCondition.hpp"
#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"

#include <atomic>
#include <functional>
#include <vector>

namespace cfd
{

class Profiler;

class SIMPLE
{
public:
    SIMPLE(const Mesh& mesh, double density, double viscosity, SIMPLESettings settings = {});

    void setBoundaryConditions(std::vector<const BoundaryCondition*> conditions);
    void setProfiler(Profiler* profiler) noexcept;
    [[nodiscard]] SIMPLEResult solve(
        VectorField& velocity,
        ScalarField& pressure,
        const std::atomic_bool* cancelRequested = nullptr,
        std::function<void(const SIMPLEIteration&)> iterationCallback = {}
    );

private:
    void applyBoundaryConditions(VectorField& velocity, ScalarField& pressure) const;
    void validateFields(const VectorField& velocity, const ScalarField& pressure) const;

    const Mesh& mesh_;
    double density_;
    double viscosity_;
    SIMPLESettings settings_;
    std::vector<const BoundaryCondition*> boundaryConditions_;
    Profiler* profiler_{nullptr};
};

}
