#pragma once

#include "core/mesh/Mesh.hpp"

namespace cfd
{

class MeshValidator
{
public:
    [[nodiscard]] static bool valid(const Mesh& mesh) noexcept;
};

}
