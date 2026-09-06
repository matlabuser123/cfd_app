#include "gui/CaseEditor.hpp"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>

namespace
{

QDoubleSpinBox* createDoubleSpinBox(double minimum, double maximum, int decimals, double step, QWidget* parent)
{
    auto* spinBox = new QDoubleSpinBox(parent);
    spinBox->setRange(minimum, maximum);
    spinBox->setDecimals(decimals);
    spinBox->setSingleStep(step);
    return spinBox;
}

}

CaseEditor::CaseEditor(QWidget* parent)
    : QWidget(parent),
      nameEdit_(new QLineEdit(this)),
      nxSpinBox_(new QSpinBox(this)),
      nySpinBox_(new QSpinBox(this)),
      lengthSpinBox_(createDoubleSpinBox(1e-6, 1e6, 6, 0.1, this)),
      heightSpinBox_(createDoubleSpinBox(1e-6, 1e6, 6, 0.1, this)),
      densitySpinBox_(createDoubleSpinBox(1e-12, 1e12, 8, 0.1, this)),
      viscositySpinBox_(createDoubleSpinBox(1e-12, 1e12, 12, 0.001, this)),
      lidVelocitySpinBox_(createDoubleSpinBox(-1e6, 1e6, 6, 0.1, this)),
      continuityToleranceSpinBox_(createDoubleSpinBox(1e-12, 1e12, 12, 0.1, this)),
      momentumToleranceSpinBox_(createDoubleSpinBox(1e-12, 1e12, 12, 0.1, this)),
      pressureToleranceSpinBox_(createDoubleSpinBox(1e-12, 1e12, 12, 0.1, this)),
      maxIterationsSpinBox_(new QSpinBox(this))
{
    nxSpinBox_->setRange(1, 1000);
    nySpinBox_->setRange(1, 1000);
    maxIterationsSpinBox_->setRange(1, 1'000'000);

    auto* layout = new QFormLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addRow("Case", nameEdit_);
    layout->addRow("Cells in X", nxSpinBox_);
    layout->addRow("Cells in Y", nySpinBox_);
    layout->addRow("Length", lengthSpinBox_);
    layout->addRow("Height", heightSpinBox_);
    layout->addRow("Density", densitySpinBox_);
    layout->addRow("Viscosity", viscositySpinBox_);
    layout->addRow("Lid velocity", lidVelocitySpinBox_);
    layout->addRow("Continuity tolerance", continuityToleranceSpinBox_);
    layout->addRow("Momentum tolerance", momentumToleranceSpinBox_);
    layout->addRow("Pressure tolerance", pressureToleranceSpinBox_);
    layout->addRow("Maximum iterations", maxIterationsSpinBox_);

    setValidationCase({});
}

cfd::ValidationCase CaseEditor::validationCase() const
{
    cfd::ValidationCase validationCase;
    validationCase.name = nameEdit_->text().trimmed().toStdString();
    if (validationCase.name.empty()) validationCase.name = "lid_driven_cavity";
    validationCase.nx = static_cast<std::size_t>(nxSpinBox_->value());
    validationCase.ny = static_cast<std::size_t>(nySpinBox_->value());
    validationCase.length = lengthSpinBox_->value();
    validationCase.height = heightSpinBox_->value();
    validationCase.density = densitySpinBox_->value();
    validationCase.viscosity = viscositySpinBox_->value();
    validationCase.lidVelocity = lidVelocitySpinBox_->value();
    validationCase.continuityTolerance = continuityToleranceSpinBox_->value();
    validationCase.momentumTolerance = momentumToleranceSpinBox_->value();
    validationCase.pressureTolerance = pressureToleranceSpinBox_->value();
    validationCase.maxIterations = static_cast<std::size_t>(maxIterationsSpinBox_->value());
    return validationCase;
}

void CaseEditor::setValidationCase(const cfd::ValidationCase& validationCase)
{
    nameEdit_->setText(QString::fromStdString(validationCase.name));
    nxSpinBox_->setValue(static_cast<int>(validationCase.nx));
    nySpinBox_->setValue(static_cast<int>(validationCase.ny));
    lengthSpinBox_->setValue(validationCase.length);
    heightSpinBox_->setValue(validationCase.height);
    densitySpinBox_->setValue(validationCase.density);
    viscositySpinBox_->setValue(validationCase.viscosity);
    lidVelocitySpinBox_->setValue(validationCase.lidVelocity);
    continuityToleranceSpinBox_->setValue(validationCase.continuityTolerance);
    momentumToleranceSpinBox_->setValue(validationCase.momentumTolerance);
    pressureToleranceSpinBox_->setValue(validationCase.pressureTolerance);
    maxIterationsSpinBox_->setValue(static_cast<int>(validationCase.maxIterations));
}