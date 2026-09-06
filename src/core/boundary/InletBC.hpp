#pragma once

#include "core/boundary/BoundaryCondition.hpp"
#include "core/fields/VectorField.hpp"

namespace cfd
{

class InletBC final : public BoundaryCondition
{
public:
    InletBC(std::string patchName, std::vector<std::size_t> cellIndices, double value);
    InletBC(std::string patchName, std::vector<std::size_t> cellIndices, Vector2 value);

    void apply(ScalarField& field) const override;
    void apply(VectorField& field) const override;

private:
    double scalarValue_{0.0};
    Vector2 vectorValue_{};
    bool isVector_{false};
};

}
