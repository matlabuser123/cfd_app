#pragma once

#include <chrono>
#include <string>

namespace cfd
{

class Profiler;

class ProfileScope
{
public:
    ProfileScope(Profiler& profiler, std::string name);
    ~ProfileScope();

private:
    Profiler& profiler_;
    std::string name_;
    std::chrono::steady_clock::time_point start_;
};

}
