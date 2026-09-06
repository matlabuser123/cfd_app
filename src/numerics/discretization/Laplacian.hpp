#pragma once

#include "core/mesh/Mesh.hpp"

#include <cstddef>

namespace cfd
{

class ScalarField;

class Laplacian
{
public:
    [[nodiscard]] static double centralDifference(
        const ScalarField& field,
        const Mesh& mesh,
        std::size_t cell
    );
};

}
