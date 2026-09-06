#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace cfd
{

class ScalarField;
class VectorField;

class BoundaryCondition
{
public:
    BoundaryCondition(std::string patchName, std::vector<std::size_t> cellIndices);
    virtual ~BoundaryCondition() = default;

    [[nodiscard]] const std::string& patchName() const noexcept;
    [[nodiscard]] const std::vector<std::size_t>& cellIndices() const noexcept;

    virtual void apply(ScalarField& field) const = 0;
    virtual void apply(VectorField& field) const;

protected:
    void validateScalarField(const ScalarField& field) const;
    void validateVectorField(const VectorField& field) const;

private:
    std::string patchName_;
    std::vector<std::size_t> cellIndices_;
};

}
