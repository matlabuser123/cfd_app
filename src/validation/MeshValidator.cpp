#include "validation/MeshValidator.hpp"

#include <cmath>

namespace cfd
{

bool MeshValidator::valid(const Mesh& mesh) noexcept
{
    return mesh.nx() > 0 && mesh.ny() > 0 && mesh.cellCount() == mesh.nx() * mesh.ny()
        && std::isfinite(mesh.dx()) && std::isfinite(mesh.dy())
        && mesh.dx() > 0.0 && mesh.dy() > 0.0;
}

}
