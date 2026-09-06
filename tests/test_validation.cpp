#include <cassert>
#include <atomic>
#include <cmath>
#include <vector>

#include "validation/ConservationValidator.hpp"
#include "validation/GridRefinementAnalyzer.hpp"
#include "validation/PublishedValidation.hpp"
#include "validation/ValidationRunner.hpp"

int main()
{
    cfd::ValidationCase validationCase;
    validationCase.name = "cavity_20x20_smoke";
    validationCase.continuityTolerance = 100.0;
    validationCase.momentumTolerance = 10.0;
    validationCase.pressureTolerance = 20.0;
    validationCase.maxIterations = 200;

    std::vector<cfd::SIMPLEIteration> reportedIterations;
    const cfd::ValidationResult result = cfd::ValidationRunner{}.run(
        validationCase,
        nullptr,
        [&](const cfd::SIMPLEIteration& iteration) { reportedIterations.push_back(iteration); }
    );
    assert(result.caseName == "cavity_20x20_smoke");
    assert(result.nx == 20);
    assert(result.ny == 20);
    assert(result.finiteSolution);
    assert(result.deterministic);
    assert(std::isfinite(result.massImbalance));
    assert(result.passed);
    assert(reportedIterations.size() == result.iterations);

    std::atomic_bool cancelRequested{true};
    const cfd::ValidationResult cancelledResult = cfd::ValidationRunner{}.run(validationCase, &cancelRequested);
    assert(cancelledResult.cancelled);
    assert(!cancelledResult.passed);
    assert(!cancelledResult.deterministic);
    assert(cancelledResult.iterations == 0);

    cfd::Mesh mesh;
    mesh.generateUniform(2, 2);
    cfd::VectorField uniformVelocity(mesh.cellCount(), {0.0, 0.0});
    assert(std::abs(cfd::ConservationValidator::continuityResidual(mesh, uniformVelocity, 1.0)) < 1e-12);

    cfd::ScalarField profileField(6, 0.0);
    profileField[3] = 2.0;
    const auto profile = cfd::extractVerticalUProfile(profileField, 2, 3, 0.5);
    assert(profile.size() == 3);
    assert(std::abs(profile[1].coordinate - 0.5) < 1e-12);
    assert(std::abs(profile[1].value - 2.0) < 1e-12);

    cfd::ValidationCase refinementCase;
    refinementCase.name = "cavity_40x40_refinement";
    refinementCase.nx = 40;
    refinementCase.ny = 40;
    refinementCase.continuityTolerance = 100.0;
    refinementCase.momentumTolerance = 10.0;
    refinementCase.pressureTolerance = 20.0;
    refinementCase.maxIterations = 5000;

    const cfd::ValidationResult refinementResult = cfd::ValidationRunner{}.run(refinementCase);
    assert(refinementResult.caseName == "cavity_40x40_refinement");
    assert(refinementResult.nx == 40);
    assert(refinementResult.ny == 40);
    assert(refinementResult.finiteSolution);
    assert(refinementResult.deterministic);
    assert(std::isfinite(refinementResult.massImbalance));
    assert(refinementResult.passed);

    const auto refinementSummary = cfd::GridRefinementAnalyzer{}.analyze({20, 40, 80});
    assert(refinementSummary.samples.size() == 3);
    assert(refinementSummary.samples[0].nx == 20);
    assert(refinementSummary.samples[0].ny == 20);
    assert(refinementSummary.samples[1].nx == 40);
    assert(refinementSummary.samples[1].ny == 40);
    assert(refinementSummary.samples[2].nx == 80);
    assert(refinementSummary.samples[2].ny == 80);
    for (const auto& sample : refinementSummary.samples)
    {
        assert(sample.finiteSolution);
        assert(std::isfinite(sample.massImbalance));
        assert(std::isfinite(sample.runtimeSeconds));
    }

    const std::vector<cfd::CenterlinePoint> referenceProfile = {
        {0.0, 0.0},
        {0.5, 0.5},
        {1.0, 1.0}
    };
    const auto comparison = cfd::PublishedValidationAnalyzer{}.compareProfile(referenceProfile, 100.0);
    assert(comparison.computedProfile.size() == 3);
    assert(std::isfinite(comparison.maxAbsoluteError));
    assert(std::isfinite(comparison.meanAbsoluteError));

    return 0;
}
