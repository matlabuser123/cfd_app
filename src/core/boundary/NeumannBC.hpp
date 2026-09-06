#pragma once

#include "core/boundary/BoundaryCondition.hpp"

namespace cfd
{

class NeumannBC final : public BoundaryCondition
{
public:
    NeumannBC(std::string patchName, std::vector<std::size_t> cellIndices, double gradient);

    void apply(ScalarField& field) const override;

    [[nodiscard]] double gradient() const noexcept;

private:
    double gradient_{0.0};
};

}
