#pragma once

#include <cstddef>

namespace cfd
{

struct SIMPLESettings
{
    std::size_t maxIterations{1000};
    double momentumTolerance{1e-8};
    double pressureTolerance{1e-8};
    double continuityTolerance{1e-8};
    double velocityRelaxation{0.7};
    double pressureRelaxation{0.3};

    // SIMPLE::solve requires the inner per-iteration momentum/pressure linear solve to
    // already be within momentumTolerance/pressureTolerance on every single outer
    // iteration, hard-failing otherwise -- it is not only a final convergence target.
    // When a case's own declared tolerance is tighter than the inner CG/BiCGSTAB solver
    // (falling back to Gauss-Seidel) can reliably hit within its iteration budget on
    // every outer iteration, these let a caller loosen just that per-iteration gate
    // without loosening momentumTolerance/pressureTolerance themselves, which still
    // drive the outer convergence check. 0.0 (the default) means "unset" -- SIMPLE::solve
    // falls back to momentumTolerance/pressureTolerance directly, i.e. today's behavior.
    double innerMomentumTolerance{0.0};
    double innerPressureTolerance{0.0};
};

}
