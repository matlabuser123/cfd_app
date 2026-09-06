#include "profiling/Profiler.hpp"

#include <algorithm>

namespace cfd
{

void Profiler::record(const std::string& name, double milliseconds)
{
    ProfileEntry& entry = entries_[name];
    if (entry.calls == 0)
    {
        entry.minMilliseconds = milliseconds;
        entry.maxMilliseconds = milliseconds;
    }
    else
    {
        entry.minMilliseconds = std::min(entry.minMilliseconds, milliseconds);
        entry.maxMilliseconds = std::max(entry.maxMilliseconds, milliseconds);
    }
    ++entry.calls;
    entry.totalMilliseconds += milliseconds;
}

const std::unordered_map<std::string, ProfileEntry>& Profiler::entries() const noexcept
{
    return entries_;
}

void Profiler::reset() noexcept
{
    entries_.clear();
}

}
