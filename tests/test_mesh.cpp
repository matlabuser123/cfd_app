#include <cassert>

#include "core/mesh/Mesh.hpp"

int main()
{
    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);

    assert(mesh.cellCount() == 400);
    assert(mesh.cells().front().id == 0);
    assert(mesh.cells().back().id == 399);

    return 0;
}
