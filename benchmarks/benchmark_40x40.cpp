#include <iostream>

#include "core/mesh/Mesh.hpp"

int main()
{
    cfd::Mesh mesh;
    mesh.generateUniform(40, 40);
    std::cout << "40x40 cells: " << mesh.cellCount() << '\n';
}
