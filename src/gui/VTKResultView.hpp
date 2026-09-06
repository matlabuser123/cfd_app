#pragma once

#include "io/vtk/VTKReader.hpp"

#include <QWidget>

enum class VTKDisplayField
{
    Pressure,
    VelocityMagnitude,
    Vorticity
};

class VTKResultView final : public QWidget
{
public:
    explicit VTKResultView(QWidget* parent = nullptr);

    void setResult(cfd::VTKResult result);
    void setField(VTKDisplayField field);
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    [[nodiscard]] const std::vector<double>& displayedValues() const;

    cfd::VTKResult result_;
    VTKDisplayField field_{VTKDisplayField::VelocityMagnitude};
    bool hasResult_{false};
};