#pragma once

#include "gui/CFDController.hpp"
#include "solver/simple/SIMPLEResult.hpp"

#include <QMetaType>
#include <QObject>

#include <atomic>

class SolverWorker final : public QObject
{
    Q_OBJECT

public:
    explicit SolverWorker(QObject* parent = nullptr);
    void resetCancel() noexcept;
    void requestCancel() noexcept;

public slots:
    void run(cfd::ValidationCase validationCase, cfd::ComputeBackend requestedBackend);

signals:
    void backendStatus(QString status);
    void progress(cfd::SIMPLEIteration iteration);
    void finished(cfd::ValidationResult result);
    void failed(QString message);

private:
    cfd::CFDController controller_;
    std::atomic_bool cancelRequested_{false};
};

Q_DECLARE_METATYPE(cfd::ValidationCase)
Q_DECLARE_METATYPE(cfd::ComputeBackend)
Q_DECLARE_METATYPE(cfd::SIMPLEIteration)
Q_DECLARE_METATYPE(cfd::ValidationResult)
