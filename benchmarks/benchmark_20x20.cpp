#include <iostream>

#include "core/mesh/Mesh.hpp"

int main()
{
    cfd::Mesh mesh;
    mesh.generateUniform(20, 20);
    std::cout << "20x20 cells: " << mesh.cellCount() << '\n';
}
