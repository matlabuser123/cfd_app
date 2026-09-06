#pragma once

#include "core/boundary/BoundaryCondition.hpp"

#include <optional>

namespace cfd
{

class OutletBC final : public BoundaryCondition
{
public:
    OutletBC(std::string patchName, std::vector<std::size_t> cellIndices);
    OutletBC(std::string patchName, std::vector<std::size_t> cellIndices, double pressure);

    void apply(ScalarField& field) const override;

    [[nodiscard]] bool hasPressure() const noexcept;
    [[nodiscard]] std::optional<double> pressure() const noexcept;

private:
    std::optional<double> pressure_;
};

}
