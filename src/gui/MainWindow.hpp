#pragma once

#include "gui/CFDController.hpp"

#include <QElapsedTimer>
#include <QMainWindow>

#include <optional>

class QLabel;
class QPushButton;
class QThread;
class QTableWidget;
class QProgressBar;
class QComboBox;
class QCheckBox;
class QPlainTextEdit;
class CaseEditor;
class SolverWorker;
class VTKResultView;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void loadCase();
    void loadVTKResult();
    void exportResults();
    void startRun();
    void cancelRun();
    void handleBackendStatus(QString status);
    void handleProgress(cfd::SIMPLEIteration iteration);
    void handleFinished(cfd::ValidationResult result);
    void handleFailure(QString message);

private:
    void setRunInProgress(bool inProgress);
    void reportStatus(const QString& message);

    CaseEditor* caseEditor_;
    QLabel* statusLabel_;
    QLabel* resultLabel_;
    QPlainTextEdit* statusLog_;
    QComboBox* backendComboBox_;
    QComboBox* vtkFieldComboBox_;
    QProgressBar* progressBar_;
    QTableWidget* residualTable_;
    QPushButton* loadButton_;
    QPushButton* loadVTKButton_;
    QPushButton* exportButton_;
    QPushButton* runButton_;
    QPushButton* cancelButton_;
    QCheckBox* csvExportCheckBox_;
    QCheckBox* jsonExportCheckBox_;
    QCheckBox* vtkExportCheckBox_;
    cfd::CFDController caseController_;
    QThread* workerThread_;
    SolverWorker* worker_;
    VTKResultView* vtkResultView_;
    QElapsedTimer runTimer_;
    bool cancellationRequested_{false};
    std::optional<cfd::VTKResult> loadedVTKResult_;
};
