#pragma once

#include "core/boundary/BoundaryCondition.hpp"

namespace cfd
{

class WallBC final : public BoundaryCondition
{
public:
    WallBC(std::string patchName, std::vector<std::size_t> cellIndices);

    void apply(ScalarField& field) const override;
    void apply(VectorField& field) const override;
};

}
