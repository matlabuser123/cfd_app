#pragma once

#include "validation/ValidationCase.hpp"

#include <QWidget>

class QDoubleSpinBox;
class QLineEdit;
class QSpinBox;

class CaseEditor final : public QWidget
{
public:
    explicit CaseEditor(QWidget* parent = nullptr);

    [[nodiscard]] cfd::ValidationCase validationCase() const;
    void setValidationCase(const cfd::ValidationCase& validationCase);

private:
    QLineEdit* nameEdit_;
    QSpinBox* nxSpinBox_;
    QSpinBox* nySpinBox_;
    QDoubleSpinBox* lengthSpinBox_;
    QDoubleSpinBox* heightSpinBox_;
    QDoubleSpinBox* densitySpinBox_;
    QDoubleSpinBox* viscositySpinBox_;
    QDoubleSpinBox* lidVelocitySpinBox_;
    QDoubleSpinBox* continuityToleranceSpinBox_;
    QDoubleSpinBox* momentumToleranceSpinBox_;
    QDoubleSpinBox* pressureToleranceSpinBox_;
    QSpinBox* maxIterationsSpinBox_;
};