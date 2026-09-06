#pragma once

#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"

#include <cstddef>

namespace cfd
{

class Divergence
{
public:
    [[nodiscard]] static double calculate(
        const VectorField& velocity,
        const Mesh& mesh,
        std::size_t cell
    );
};

}
