#include "profiling/ProfileReport.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace cfd
{
namespace
{

std::vector<std::string> sortedNames(const Profiler& profiler)
{
    std::vector<std::string> names;
    names.reserve(profiler.entries().size());
    for (const auto& [name, entry] : profiler.entries()) names.push_back(name);
    std::sort(names.begin(), names.end());
    return names;
}

}

std::string ProfileReport::json(
    const Profiler& profiler,
    const std::string& caseName,
    std::size_t nx,
    std::size_t ny,
    std::size_t iterations
)
{
    std::ostringstream output;
    output << std::setprecision(17) << "{\n"
           << "  \"case\": \"" << caseName << "\",\n"
           << "  \"grid\": {\"nx\": " << nx << ", \"ny\": " << ny << ", \"cells\": " << nx * ny << "},\n"
           << "  \"simple\": {\"iterations\": " << iterations << "},\n"
           << "  \"regions\": {\n";
    const auto names = sortedNames(profiler);
    for (std::size_t index = 0; index < names.size(); ++index)
    {
        const ProfileEntry& entry = profiler.entries().at(names[index]);
        output << "    \"" << names[index] << "\": {\"calls\": " << entry.calls
               << ", \"total_ms\": " << entry.totalMilliseconds
               << ", \"min_ms\": " << entry.minMilliseconds
               << ", \"max_ms\": " << entry.maxMilliseconds << "}";
        if (index + 1 != names.size()) output << ',';
        output << '\n';
    }
    output << "  }\n}\n";
    return output.str();
}

void ProfileReport::writeJson(
    const Profiler& profiler,
    const std::filesystem::path& path,
    const std::string& caseName,
    std::size_t nx,
    std::size_t ny,
    std::size_t iterations
)
{
    std::ofstream output(path);
    if (!output) throw std::runtime_error("Unable to open profiling JSON report: " + path.string());
    output << json(profiler, caseName, nx, ny, iterations);
}

void ProfileReport::writeCsv(const Profiler& profiler, const std::filesystem::path& path)
{
    std::ofstream output(path);
    if (!output) throw std::runtime_error("Unable to open profiling CSV report: " + path.string());
    output << "region,calls,total_ms,min_ms,max_ms\n";
    for (const std::string& name : sortedNames(profiler))
    {
        const ProfileEntry& entry = profiler.entries().at(name);
        output << name << ',' << entry.calls << ',' << entry.totalMilliseconds << ','
               << entry.minMilliseconds << ',' << entry.maxMilliseconds << '\n';
    }
}

}
