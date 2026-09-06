#include "io/json/JSONWriter.hpp"

#include <fstream>
#include <iomanip>
#include <locale>
#include <stdexcept>

namespace cfd
{

void JSONWriter::write(const CFDResults& results, const std::filesystem::path& file)
{
    std::ofstream output(file, std::ios::binary);
    if (!output) throw std::runtime_error("Unable to open JSON result file: " + file.string());
    output.imbue(std::locale::classic());
    const double reynolds = results.viscosity == 0.0 ? 0.0 : results.density * results.length / results.viscosity;
    output << std::setprecision(17)
        << "{\n  \"case\": \"" << results.caseName << "\",\n"
        << "  \"mesh\": {\"nx\": " << results.nx << ", \"ny\": " << results.ny << "},\n"
        << "  \"geometry\": {\"length\": " << results.length << ", \"height\": " << results.height << "},\n"
        << "  \"physics\": {\"rho\": " << results.density << ", \"mu\": " << results.viscosity << ", \"reynolds\": " << reynolds << "},\n"
        << "  \"solver\": {\"iterations\": " << results.iterations << ", \"continuity_residual\": " << results.continuityResidual << ", \"u_residual\": " << results.uResidual << ", \"v_residual\": " << results.vResidual << "}\n}\n";
}

}
