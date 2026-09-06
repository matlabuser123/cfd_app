#include "io/vtk/VTKWriter.hpp"

#include <fstream>
#include <iomanip>
#include <locale>
#include <stdexcept>

namespace cfd
{

void VTKWriter::write(const CFDResults& results, const std::filesystem::path& file)
{
    const std::size_t cells = results.nx * results.ny;
    if (results.u.size() != cells || results.v.size() != cells || results.pressure.size() != cells || results.velocityMagnitude.size() != cells || results.vorticity.size() != cells) throw std::invalid_argument("CFD result field sizes do not match mesh");
    std::ofstream output(file, std::ios::binary);
    if (!output) throw std::runtime_error("Unable to open VTK result file: " + file.string());
    output.imbue(std::locale::classic());
    output << std::setprecision(17) << "# vtk DataFile Version 3.0\nCFD solution\nASCII\nDATASET STRUCTURED_POINTS\n"
        << "DIMENSIONS " << results.nx + 1 << ' ' << results.ny + 1 << " 1\n"
        << "ORIGIN 0 0 0\nSPACING " << results.length / static_cast<double>(results.nx) << ' ' << results.height / static_cast<double>(results.ny) << " 1\n"
        << "CELL_DATA " << cells << "\n";
    output << "SCALARS pressure double 1\nLOOKUP_TABLE default\n";
    for (double value : results.pressure) output << value << '\n';
    output << "SCALARS velocity_magnitude double 1\nLOOKUP_TABLE default\n";
    for (double value : results.velocityMagnitude) output << value << '\n';
    output << "SCALARS vorticity double 1\nLOOKUP_TABLE default\n";
    for (double value : results.vorticity) output << value << '\n';
    output << "VECTORS velocity double\n";
    for (std::size_t index = 0; index < cells; ++index) output << results.u[index] << ' ' << results.v[index] << " 0\n";
}

}
