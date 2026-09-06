#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// End-to-end regression test for TODO.md item #3: `cfdapp --case <path>` must load an
// arbitrary case directory (not just the hardcoded --validate-20 path), run it through
// the SIMPLE solver to its configured iteration limit, and emit the same evidence
// artifact set --validate-20 does, under a case-specific results path. This spawns the
// real built cfdapp executable rather than calling library code directly, so it actually
// covers the CLI wiring (argument parsing, exit code, stdout) and not just the underlying
// CFDController/ValidationRunner calls already covered by CFDGUIControllerTests.

namespace
{

// std::system() on Windows runs the command through `cmd.exe /c`, which has a documented
// quirk: if the command line contains more than one pair of quotes, cmd.exe's
// quote-stripping heuristic corrupts it ("The filename, directory name, or volume label
// syntax is incorrect."), even though the string itself is perfectly well-formed. The
// fix is to wrap the whole command in one more, outer pair of quotes. Paths are also
// normalized to native (backslash) separators first, since cmd.exe can fail to resolve a
// forward-slash absolute path when launching a program, even though std::filesystem
// itself handles mixed separators fine.
int runCfdapp(const std::filesystem::path& executable, const std::string& args)
{
    std::filesystem::path normalizedExecutable = executable;
    normalizedExecutable.make_preferred();
    std::ostringstream command;
    command << '"' << normalizedExecutable.string() << "\" " << args;
    const std::string wrapped = "\"" + command.str() + "\"";
    return std::system(wrapped.c_str());
}

}

int main()
{
    const std::filesystem::path executable(CFDAPP_EXECUTABLE_PATH);
    std::filesystem::path caseDirectory = std::filesystem::path(CFDAPP_SOURCE_DIR) / "cases" / "cavity_20x20";
    caseDirectory.make_preferred();
    // main.cpp writes evidence to the relative path "results/case/<name>" (matching
    // --validate-20's own pre-existing "results/validation/..." convention), which
    // resolves against whatever the process's current working directory happens to be.
    // CTest runs test executables with their working directory set to the test binary's
    // own directory, not the source root, and cfdapp (launched via std::system() below)
    // inherits that same directory -- so check there, not under CFDAPP_SOURCE_DIR.
    const std::filesystem::path evidenceDirectory =
        std::filesystem::current_path() / "results" / "case" / "lid_driven_cavity";

    // Start from a clean slate so this test proves the CLI run actually produced the
    // files below, rather than finding stale evidence left over from a previous run.
    std::error_code ignored;
    std::filesystem::remove_all(evidenceDirectory, ignored);

    // cavity_20x20's own numerics.json declares a 1e-8 momentum/pressure tolerance that
    // SIMPLE does not reach within its declared 1000-iteration cap (see main.cpp's
    // --case handler for why: this is a genuine, pre-existing SIMPLE characteristic, not
    // a CLI bug). Exit code 1 -- "ran the full iteration limit without converging" -- is
    // therefore the correct, deterministic, expected outcome here, not a failure of this
    // test: the acceptance bar for this item is "runs to completion and writes result
    // artifacts", which does not require the run to actually converge.
    const int exitCode = runCfdapp(executable, "--case \"" + caseDirectory.string() + "\"");
    assert(exitCode == 1);

    assert(std::filesystem::exists(evidenceDirectory / "validation_report.txt"));
    assert(std::filesystem::exists(evidenceDirectory / "validation.json"));
    assert(std::filesystem::exists(evidenceDirectory / "residuals.csv"));

    std::ifstream report(evidenceDirectory / "validation_report.txt");
    std::ostringstream reportContent;
    reportContent << report.rdbuf();
    const std::string reportText = reportContent.str();
    assert(reportText.find("case: lid_driven_cavity") != std::string::npos);
    assert(reportText.find("mesh: 20x20") != std::string::npos);
    assert(reportText.find("iterations: 1000") != std::string::npos);
    assert(reportText.find("smoke_gate_passed: false") != std::string::npos);

    std::ifstream metadata(evidenceDirectory / "validation.json");
    std::ostringstream metadataContent;
    metadataContent << metadata.rdbuf();
    const std::string metadataText = metadataContent.str();
    assert(metadataText.find("\"case\": \"lid_driven_cavity\"") != std::string::npos);
    assert(metadataText.find("\"smoke_gate_passed\": false") != std::string::npos);

    // Missing/invalid case directories must fail cleanly (exit code 2), not crash.
    const int badExitCode = runCfdapp(executable, "--case \"" + (caseDirectory / "does_not_exist").string() + "\"");
    assert(badExitCode == 2);

    return 0;
}
