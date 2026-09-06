#include "gui/MainWindow.hpp"

#include "gui/CaseEditor.hpp"
#include "gui/SolverWorker.hpp"
#include "gui/VTKResultView.hpp"

#include <QFileDialog>
#include <QCheckBox>
#include <QComboBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <exception>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
            caseEditor_(new CaseEditor(this)),
      statusLabel_(new QLabel("Ready", this)),
            resultLabel_(new QLabel("No validation run yet", this)),
            statusLog_(new QPlainTextEdit(this)),
    backendComboBox_(new QComboBox(this)),
            vtkFieldComboBox_(new QComboBox(this)),
    progressBar_(new QProgressBar(this)),
    residualTable_(new QTableWidget(this)),
            loadButton_(new QPushButton("Load case", this)),
            loadVTKButton_(new QPushButton("Load VTK", this)),
    exportButton_(new QPushButton("Export", this)),
      runButton_(new QPushButton("Run validation", this)),
            cancelButton_(new QPushButton("Cancel", this)),
    csvExportCheckBox_(new QCheckBox("CSV", this)),
    jsonExportCheckBox_(new QCheckBox("JSON", this)),
    vtkExportCheckBox_(new QCheckBox("VTK", this)),
      workerThread_(new QThread(this)),
            worker_(new SolverWorker()),
            vtkResultView_(new VTKResultView(this))
{
    qRegisterMetaType<cfd::ValidationCase>();
    qRegisterMetaType<cfd::ComputeBackend>();
    qRegisterMetaType<cfd::SIMPLEIteration>();
    qRegisterMetaType<cfd::ValidationResult>();
    cfd::ValidationCase defaultCase;
    defaultCase.continuityTolerance = 100.0;
    defaultCase.momentumTolerance = 10.0;
    defaultCase.pressureTolerance = 20.0;
    defaultCase.maxIterations = 200;
    caseEditor_->setValidationCase(defaultCase);

    auto* central = new QWidget(this);
    auto* layout = new QVBoxLayout(central);
    layout->addWidget(new QLabel("CFDApp", central));
    layout->addWidget(caseEditor_);
    backendComboBox_->addItem("Serial CPU", static_cast<int>(cfd::ComputeBackend::Serial));
    backendComboBox_->addItem("OpenMP CPU", static_cast<int>(cfd::ComputeBackend::OpenMP));
    backendComboBox_->addItem("CUDA", static_cast<int>(cfd::ComputeBackend::CUDA));
    backendComboBox_->setCurrentIndex(1);
    auto* backendLayout = new QHBoxLayout();
    backendLayout->addWidget(new QLabel("Solver backend", central));
    backendLayout->addWidget(backendComboBox_, 1);
    layout->addLayout(backendLayout);
    layout->addWidget(loadButton_);
    layout->addWidget(runButton_);
    layout->addWidget(cancelButton_);
    layout->addWidget(statusLabel_);
    progressBar_->setRange(0, 100);
    layout->addWidget(progressBar_);
    resultLabel_->setWordWrap(true);
    layout->addWidget(resultLabel_);
    statusLog_->setReadOnly(true);
    statusLog_->document()->setMaximumBlockCount(100);
    statusLog_->setMaximumHeight(120);
    layout->addWidget(statusLog_);
    residualTable_->setColumnCount(5);
    residualTable_->setHorizontalHeaderLabels({"Iteration", "Continuity", "U", "V", "Pressure"});
    residualTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    residualTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(residualTable_, 1);
    vtkFieldComboBox_->addItem("Pressure", static_cast<int>(VTKDisplayField::Pressure));
    vtkFieldComboBox_->addItem("Velocity magnitude", static_cast<int>(VTKDisplayField::VelocityMagnitude));
    vtkFieldComboBox_->addItem("Vorticity", static_cast<int>(VTKDisplayField::Vorticity));
    vtkFieldComboBox_->setCurrentIndex(1);
    auto* vtkLayout = new QHBoxLayout();
    vtkLayout->addWidget(loadVTKButton_);
    vtkLayout->addWidget(vtkFieldComboBox_, 1);
    layout->addLayout(vtkLayout);
    layout->addWidget(vtkResultView_, 2);
    csvExportCheckBox_->setChecked(true);
    jsonExportCheckBox_->setChecked(true);
    vtkExportCheckBox_->setChecked(true);
    exportButton_->setEnabled(false);
    auto* exportLayout = new QHBoxLayout();
    exportLayout->addWidget(csvExportCheckBox_);
    exportLayout->addWidget(jsonExportCheckBox_);
    exportLayout->addWidget(vtkExportCheckBox_);
    exportLayout->addWidget(exportButton_);
    layout->addLayout(exportLayout);
    setCentralWidget(central);
    setWindowTitle("CFDApp");
    resize(560, 680);
    setRunInProgress(false);
    reportStatus("Ready");

    worker_->moveToThread(workerThread_);
    connect(loadButton_, &QPushButton::clicked, this, &MainWindow::loadCase);
    connect(loadVTKButton_, &QPushButton::clicked, this, &MainWindow::loadVTKResult);
    connect(exportButton_, &QPushButton::clicked, this, &MainWindow::exportResults);
    connect(vtkFieldComboBox_, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index)
    {
        vtkResultView_->setField(static_cast<VTKDisplayField>(vtkFieldComboBox_->itemData(index).toInt()));
    });
    connect(runButton_, &QPushButton::clicked, this, &MainWindow::startRun);
    connect(cancelButton_, &QPushButton::clicked, this, &MainWindow::cancelRun);
    connect(workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(worker_, &SolverWorker::backendStatus, this, &MainWindow::handleBackendStatus);
    connect(worker_, &SolverWorker::progress, this, &MainWindow::handleProgress);
    connect(worker_, &SolverWorker::finished, this, &MainWindow::handleFinished);
    connect(worker_, &SolverWorker::failed, this, &MainWindow::handleFailure);
    workerThread_->start();
}

MainWindow::~MainWindow()
{
    worker_->requestCancel();
    workerThread_->quit();
    workerThread_->wait();
}

void MainWindow::setRunInProgress(bool inProgress)
{
    caseEditor_->setEnabled(!inProgress);
    backendComboBox_->setEnabled(!inProgress);
    loadButton_->setEnabled(!inProgress);
    runButton_->setEnabled(!inProgress);
    cancelButton_->setEnabled(inProgress);
    if (!inProgress) cancellationRequested_ = false;
}

void MainWindow::reportStatus(const QString& message)
{
    statusLabel_->setText(message);
    statusBar()->showMessage(message);
    statusLog_->appendPlainText(message);
}

void MainWindow::loadCase()
{
    const QString directory = QFileDialog::getExistingDirectory(this, "Load CFD case");
    if (directory.isEmpty()) return;

    try
    {
        const cfd::ValidationCase validationCase = caseController_.loadValidationCase(directory.toStdString());
        caseEditor_->setValidationCase(validationCase);
        reportStatus("Case loaded");
        resultLabel_->setText(QString("Loaded %1 (%2 x %3)").arg(QString::fromStdString(validationCase.name)).arg(validationCase.nx).arg(validationCase.ny));
    }
    catch (const std::exception& error)
    {
        reportStatus(QString("Case loading failed: %1").arg(QString::fromUtf8(error.what())));
        QMessageBox::critical(this, "Case loading error", QString::fromUtf8(error.what()));
    }
}

void MainWindow::loadVTKResult()
{
    const QString file = QFileDialog::getOpenFileName(this, "Load VTK result", {}, "VTK files (*.vtk)");
    if (file.isEmpty()) return;

    try
    {
        const cfd::VTKResult result = caseController_.loadVTKResult(file.toStdString());
        loadedVTKResult_ = result;
        vtkResultView_->setResult(result);
        exportButton_->setEnabled(true);
        reportStatus("VTK result loaded");
        resultLabel_->setText(QString("VTK grid: %1 x %2").arg(result.nx).arg(result.ny));
    }
    catch (const std::exception& error)
    {
        reportStatus(QString("VTK loading failed: %1").arg(QString::fromUtf8(error.what())));
        QMessageBox::critical(this, "VTK loading error", QString::fromUtf8(error.what()));
    }
}

void MainWindow::exportResults()
{
    if (!loadedVTKResult_) return;
    const cfd::ResultExportOptions options{
        csvExportCheckBox_->isChecked(),
        jsonExportCheckBox_->isChecked(),
        vtkExportCheckBox_->isChecked()
    };
    if (!options.csv && !options.json && !options.vtk)
    {
        reportStatus("Export failed: no output format selected");
        QMessageBox::warning(this, "Export", "Select at least one output format");
        return;
    }

    const QString directory = QFileDialog::getExistingDirectory(this, "Export results");
    if (directory.isEmpty()) return;
    try
    {
        caseController_.exportVTKResult(*loadedVTKResult_, directory.toStdString(), options);
        reportStatus("Results exported");
    }
    catch (const std::exception& error)
    {
        reportStatus(QString("Export failed: %1").arg(QString::fromUtf8(error.what())));
        QMessageBox::critical(this, "Export error", QString::fromUtf8(error.what()));
    }
}

void MainWindow::startRun()
{
    const cfd::ValidationCase validationCase = caseEditor_->validationCase();
    const auto requestedBackend = static_cast<cfd::ComputeBackend>(backendComboBox_->currentData().toInt());
    worker_->resetCancel();
    cancellationRequested_ = false;
    setRunInProgress(true);
    progressBar_->setRange(0, static_cast<int>(validationCase.maxIterations));
    progressBar_->setValue(0);
    runTimer_.start();
    reportStatus("Configuring backend...");
    resultLabel_->setText("Solving selected cavity validation case");
    residualTable_->setRowCount(0);
    QMetaObject::invokeMethod(
        worker_,
        "run",
        Qt::QueuedConnection,
        Q_ARG(cfd::ValidationCase, validationCase),
        Q_ARG(cfd::ComputeBackend, requestedBackend)
    );
}

void MainWindow::cancelRun()
{
    worker_->requestCancel();
    cancellationRequested_ = true;
    cancelButton_->setEnabled(false);
    reportStatus("Cancelling...");
}

void MainWindow::handleBackendStatus(QString status)
{
    reportStatus(status);
}

void MainWindow::handleProgress(cfd::SIMPLEIteration iteration)
{
    progressBar_->setValue(static_cast<int>(iteration.iteration));
    if (cancellationRequested_)
    {
        const QString status = QString("Cancelling after iteration %1").arg(iteration.iteration);
        statusLabel_->setText(status);
        statusBar()->showMessage(status);
    }
    else
    {
        const QString status = QString("Running: iteration %1 / %2 | continuity %3 | %4 ms")
            .arg(iteration.iteration)
            .arg(progressBar_->maximum())
            .arg(iteration.continuityResidual, 0, 'g', 6)
            .arg(runTimer_.elapsed());
        statusLabel_->setText(status);
        statusBar()->showMessage(status);
    }
    const int row = residualTable_->rowCount();
    residualTable_->insertRow(row);
    const auto setValue = [&](int column, const QString& value)
    {
        residualTable_->setItem(row, column, new QTableWidgetItem(value));
    };
    setValue(0, QString::number(iteration.iteration));
    setValue(1, QString::number(iteration.continuityResidual, 'g', 8));
    setValue(2, QString::number(iteration.uResidual, 'g', 8));
    setValue(3, QString::number(iteration.vResidual, 'g', 8));
    setValue(4, QString::number(iteration.pressureResidual, 'g', 8));
    residualTable_->scrollToBottom();
}

void MainWindow::handleFinished(cfd::ValidationResult result)
{
    if (result.cancelled)
    {
        reportStatus(QString("Run cancelled after %1 ms").arg(runTimer_.elapsed()));
        resultLabel_->setText(QString("Cancelled after %1 iterations").arg(result.iterations));
    }
    else
    {
        reportStatus(
            result.passed
                ? QString("Validation passed in %1 ms").arg(runTimer_.elapsed())
                : QString("Validation completed with failures in %1 ms").arg(runTimer_.elapsed())
        );
        resultLabel_->setText(
            QString("Grid: %1 x %2\nIterations: %3\nContinuity residual: %4\nRuntime: %5 s\nDeterministic: %6")
                .arg(result.nx)
                .arg(result.ny)
                .arg(result.iterations)
                .arg(result.continuityResidual, 0, 'g', 8)
                .arg(result.runtimeSeconds, 0, 'f', 3)
                .arg(result.deterministic ? "yes" : "no")
        );
    }
    setRunInProgress(false);
}

void MainWindow::handleFailure(QString message)
{
    reportStatus(QString("Solver failed: %1").arg(message));
    resultLabel_->setText("No result available");
    setRunInProgress(false);
    QMessageBox::critical(this, "CFD error", message);
}
