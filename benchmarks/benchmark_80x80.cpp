#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string_view>
#include <vector>

#include "core/boundary/DirichletBC.hpp"
#include "core/boundary/WallBC.hpp"
#include "core/fields/ScalarField.hpp"
#include "core/fields/VectorField.hpp"
#include "core/mesh/Mesh.hpp"
#include "parallel/ParallelRuntime.hpp"
#include "profiling/Profiler.hpp"
#include "solver/simple/SIMPLE.hpp"

namespace
{

constexpr std::size_t samples = 5;
constexpr auto maximumMedianMilliseconds = 200;

struct BenchmarkResult
{
    cfd::SIMPLEResult solverResult;
    double medianMilliseconds{0.0};
    bool finite{true};
    std::size_t sampleCount{0};
};

bool isFinite(const cfd::VectorField& velocity, const cfd::ScalarField& pressure)
{
    for (std::size_t cell = 0; cell < velocity.size(); ++cell)
    {
        if (!std::isfinite(velocity[cell].x) || !std::isfinite(velocity[cell].y)) return false;
    }
    for (std::size_t cell = 0; cell < pressure.size(); ++cell)
    {
        if (!std::isfinite(pressure[cell])) return false;
    }
    return true;
}

BenchmarkResult runBenchmark(
    const cfd::Mesh& mesh,
    const std::vector<const cfd::BoundaryCondition*>& boundaryConditions,
    int threadCount,
    cfd::Profiler* profiler = nullptr
)
{
    cfd::ParallelRuntime::initialize({true, threadCount});
    std::vector<double> elapsedMilliseconds;
    elapsedMilliseconds.reserve(samples);
    BenchmarkResult benchmark;
    for (std::size_t sample = 0; sample < samples; ++sample)
    {
        cfd::VectorField velocity(mesh.cellCount(), {0.0, 0.0});
        cfd::ScalarField pressure(mesh.cellCount(), 0.0);
        if (profiler != nullptr) profiler->reset();
        cfd::SIMPLE simple(mesh, 1.0, 0.01, {200, 10.0, 50.0, 100.0, 0.7, 0.3});
        simple.setBoundaryConditions(boundaryConditions);
        simple.setProfiler(profiler);
        const auto start = std::chrono::steady_clock::now();
        benchmark.solverResult = simple.solve(velocity, pressure);
        const auto end = std::chrono::steady_clock::now();
        elapsedMilliseconds.push_back(std::chrono::duration<double, std::milli>(end - start).count());
        benchmark.finite = isFinite(velocity, pressure);
        if (!benchmark.solverResult.converged || !benchmark.finite) break;
    }

    std::sort(elapsedMilliseconds.begin(), elapsedMilliseconds.end());
    benchmark.sampleCount = elapsedMilliseconds.size();
    if (!elapsedMilliseconds.empty()) benchmark.medianMilliseconds = elapsedMilliseconds[elapsedMilliseconds.size() / 2];
    return benchmark;
}

void printSummary(const BenchmarkResult& benchmark)
{
    std::cout << "iterations=" << benchmark.solverResult.iterations
              << " continuityResidual=" << benchmark.solverResult.continuityResidual
              << " uResidual=" << benchmark.solverResult.uResidual
              << " vResidual=" << benchmark.solverResult.vResidual
              << " median_elapsed_ms=" << benchmark.medianMilliseconds
              << " samples=" << benchmark.sampleCount
              << " converged=" << (benchmark.solverResult.converged ? "true" : "false")
              << " finite=" << (benchmark.finite ? "true" : "false");
}

bool passed(const BenchmarkResult& benchmark)
{
    return benchmark.solverResult.converged && benchmark.finite && benchmark.sampleCount == samples;
}

}

int main(int argc, char* argv[])
{
    const bool scalingMode = argc == 2 && std::string_view(argv[1]) == "--thread-scaling";
    if (argc > 1 && !scalingMode)
    {
        std::cerr << "Usage: benchmark_80x80 [--thread-scaling]\n";
        return 2;
    }

    cfd::Mesh mesh;
    mesh.generateUniform(80, 80);
    std::vector<std::size_t> top;
    std::vector<std::size_t> bottom;
    std::vector<std::size_t> left;
    std::vector<std::size_t> right;
    top.reserve(80);
    bottom.reserve(80);
    left.reserve(80);
    right.reserve(80);
    for (std::size_t column = 0; column < 80; ++column)
    {
        top.push_back((79 * 80) + column);
        bottom.push_back(column);
    }
    for (std::size_t row = 0; row < 80; ++row)
    {
        left.push_back(row * 80);
        right.push_back(row * 80 + 79);
    }

    cfd::DirichletBC lid("top", top, cfd::Vector2{1.0, 0.0});
    cfd::WallBC bottomWall("bottom", bottom);
    cfd::WallBC leftWall("left", left);
    cfd::WallBC rightWall("right", right);
    const std::vector<const cfd::BoundaryCondition*> boundaryConditions{&lid, &bottomWall, &leftWall, &rightWall};

    if (scalingMode)
    {
        const std::array<int, 4> threadCounts{1, 2, 4, 8};
        std::vector<BenchmarkResult> measurements;
        measurements.reserve(threadCounts.size());
        for (const int threadCount : threadCounts)
        {
            measurements.push_back(runBenchmark(mesh, boundaryConditions, threadCount));
            if (!passed(measurements.back())) return 1;
        }

        const double oneThreadMilliseconds = measurements.front().medianMilliseconds;
        std::cout << "80x80 thread scaling: cells=" << mesh.cellCount() << " samples=" << samples << '\n';
        for (std::size_t index = 0; index < threadCounts.size(); ++index)
        {
            const BenchmarkResult& measurement = measurements[index];
            std::cout << "  threads=" << threadCounts[index]
                      << " median_elapsed_ms=" << measurement.medianMilliseconds
                      << " speedup=" << oneThreadMilliseconds / measurement.medianMilliseconds << '\n';
        }
        return 0;
    }

    cfd::Profiler profiler;
    const BenchmarkResult benchmark = runBenchmark(mesh, boundaryConditions, 0, &profiler);
    std::cout << "80x80 benchmark: cells=" << mesh.cellCount() << ' ';
    printSummary(benchmark);
    std::cout << " maximum_median_ms=" << maximumMedianMilliseconds << '\n';

    std::vector<std::pair<std::string, double>> ranked;
    ranked.reserve(profiler.entries().size());
    for (const auto& [name, entry] : profiler.entries()) ranked.emplace_back(name, entry.totalMilliseconds);
    std::sort(ranked.begin(), ranked.end(), [](const auto& left, const auto& right) { return left.second > right.second; });
    std::cout << "Top hotspots:\n";
    for (std::size_t index = 0; index < std::min<std::size_t>(ranked.size(), 5U); ++index)
    {
        std::cout << "  " << index + 1 << ". " << ranked[index].first << " = " << ranked[index].second << " ms\n";
    }

    return passed(benchmark) && benchmark.medianMilliseconds <= maximumMedianMilliseconds ? 0 : 1;
}