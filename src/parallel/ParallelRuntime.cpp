#include "parallel/ParallelRuntime.hpp"

#if CFDAPP_HAS_OPENMP
#include <omp.h>
#endif

namespace cfd
{
namespace
{
ParallelSettings currentSettings{};
}

void ParallelRuntime::initialize(const ParallelSettings& settings)
{
    currentSettings = settings;
#if CFDAPP_HAS_OPENMP
    if (settings.threadCount > 0) omp_set_num_threads(settings.threadCount);
#endif
}

bool ParallelRuntime::enabled() noexcept
{
#if CFDAPP_HAS_OPENMP
    return currentSettings.enabled;
#else
    return false;
#endif
}

int ParallelRuntime::threadCount() noexcept
{
#if CFDAPP_HAS_OPENMP
    return currentSettings.threadCount > 0 ? currentSettings.threadCount : omp_get_max_threads();
#else
    return 1;
#endif
}

}
