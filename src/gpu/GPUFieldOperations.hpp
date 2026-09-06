#pragma once

#include <vector>

namespace cfd
{

class GPUBackend;

class GPUFieldOperations
{
public:
    static void scale(std::vector<double>& values, double factor, const GPUBackend& backend);
};

}