#include "profiling/ProfileScope.hpp"

#include "profiling/Profiler.hpp"

#include <utility>

namespace cfd
{

ProfileScope::ProfileScope(Profiler& profiler, std::string name)
    : profiler_(profiler),
      name_(std::move(name)),
      start_(std::chrono::steady_clock::now())
{
}

ProfileScope::~ProfileScope()
{
    const auto end = std::chrono::steady_clock::now();
    const double milliseconds = std::chrono::duration<double, std::milli>(end - start_).count();
    profiler_.record(name_, milliseconds);
}

}
