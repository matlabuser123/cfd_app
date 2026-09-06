#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

namespace cfd
{

struct ProfileEntry
{
    std::size_t calls{0};
    double totalMilliseconds{0.0};
    double minMilliseconds{0.0};
    double maxMilliseconds{0.0};
};

class Profiler
{
public:
    void record(const std::string& name, double milliseconds);
    [[nodiscard]] const std::unordered_map<std::string, ProfileEntry>& entries() const noexcept;
    void reset() noexcept;

private:
    std::unordered_map<std::string, ProfileEntry> entries_;
};

}
