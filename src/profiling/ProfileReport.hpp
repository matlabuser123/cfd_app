#pragma once

#include "profiling/Profiler.hpp"

#include <cstddef>
#include <filesystem>
#include <string>

namespace cfd
{

class ProfileReport
{
public:
    [[nodiscard]] static std::string json(
        const Profiler& profiler,
        const std::string& caseName,
        std::size_t nx,
        std::size_t ny,
        std::size_t iterations
    );

    static void writeJson(
        const Profiler& profiler,
        const std::filesystem::path& path,
        const std::string& caseName,
        std::size_t nx,
        std::size_t ny,
        std::size_t iterations
    );

    static void writeCsv(
        const Profiler& profiler,
        const std::filesystem::path& path
    );
};

}
