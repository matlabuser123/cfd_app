#include "io/csv/CSVWriter.hpp"

#include <fstream>
#include <iomanip>
#include <locale>
#include <stdexcept>

namespace cfd
{

void CSVWriter::writeFields(const CFDResults& results, const std::filesystem::path& file)
{
    const std::size_t cells = results.nx * results.ny;
    if (results.x.size() != cells || results.y.size() != cells || results.u.size() != cells || results.v.size() != cells || results.pressure.size() != cells || results.velocityMagnitude.size() != cells || results.vorticity.size() != cells) throw std::invalid_argument("CFD result field sizes do not match mesh");
    std::ofstream output(file, std::ios::binary);
    if (!output) throw std::runtime_error("Unable to open CSV field file: " + file.string());
    output.imbue(std::locale::classic());
    output << std::setprecision(17) << "i,j,x,y,u,v,pressure,velocity_magnitude,vorticity\n";
    for (std::size_t j = 0; j < results.ny; ++j)
    {
        for (std::size_t i = 0; i < results.nx; ++i)
        {
            const std::size_t index = j * results.nx + i;
            output << i << ',' << j << ',' << results.x[index] << ',' << results.y[index] << ',' << results.u[index] << ',' << results.v[index] << ',' << results.pressure[index] << ',' << results.velocityMagnitude[index] << ',' << results.vorticity[index] << '\n';
        }
    }
}

void CSVWriter::writeResiduals(const std::vector<ResidualRecord>& residuals, const std::filesystem::path& file)
{
    std::ofstream output(file, std::ios::binary);
    if (!output) throw std::runtime_error("Unable to open CSV residual file: " + file.string());
    output.imbue(std::locale::classic());
    output << std::setprecision(17) << "iteration,continuity,u_momentum,v_momentum\n";
    for (const ResidualRecord& record : residuals) output << record.iteration << ',' << record.continuity << ',' << record.uMomentum << ',' << record.vMomentum << '\n';
}

}
