#include "gui/SolverWorker.hpp"

#include <exception>

SolverWorker::SolverWorker(QObject* parent)
    : QObject(parent)
{
}

void SolverWorker::resetCancel() noexcept
{
    cancelRequested_.store(false);
}

void SolverWorker::requestCancel() noexcept
{
    cancelRequested_.store(true);
}

void SolverWorker::run(cfd::ValidationCase validationCase, cfd::ComputeBackend requestedBackend)
{
    try
    {
        const cfd::SolverBackendSelection selection = controller_.configureBackend(requestedBackend);
        emit backendStatus(QString::fromStdString(selection.status));
        emit finished(controller_.runValidation(
            validationCase,
            &cancelRequested_,
            [this](const cfd::SIMPLEIteration& iteration) { emit progress(iteration); }
        ));
    }
    catch (const std::exception& error)
    {
        emit failed(QString::fromUtf8(error.what()));
    }
}
