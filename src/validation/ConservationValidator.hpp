#pragma once

#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"

namespace cfd
{

class ConservationValidator
{
public:
    [[nodiscard]] static double continuityResidual(
        const Mesh& mesh,
        const VectorField& velocity,
        double density
    );
};

}
