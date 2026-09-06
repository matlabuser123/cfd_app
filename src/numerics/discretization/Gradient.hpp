#pragma once

#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"

#include <cstddef>

namespace cfd
{

class ScalarField;

class Gradient
{
public:
    [[nodiscard]] static Vector2 centralDifference(
        const ScalarField& field,
        const Mesh& mesh,
        std::size_t cell
    );
};

}
