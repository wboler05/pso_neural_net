#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QKeyEvent>
#include <QTimer>
#include <QTime>
#include <QMessageBox>

#include <fstream>
#include <cinttypes>
#include <vector>
#include <ctime>
#include <random>

#include <andtrainer.h>
#include "backend/neuralpso.h"
#include "backend/NeuralNet/NeuralNet.h"
#include "backend/PSO/pso.h"
#include "logger.h"
#include "frontend/innernetnodesinput.h"

//#include "fnnpsogsa.h"

#include "aboutconfusionmatrixdialog.h"

#ifdef OPENCL_DEFINED
#include "CL/cl.hpp"
#endif

#define TOTAL_GENERATED_LABELS 10000

namespace Ui {
class MainWindow;
}

class NeuralPso;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:

    void keyPressEvent(QKeyEvent * e);
    uint32_t char2uint(uint8_t *input);
    uint32_t readUnsignedInt(std::ifstream &input);

    void setOutputLabel(const QString & s);

    void generateAndLabels();
    void tellParameters();

protected slots:
    void runNeuralPso();
    void stopPso();
    void updatePlot();
    void applyParameterChanges();
    void setParameterDefaults();
    void updateParameterGui();
    void setInnerNetNodesFromGui();
    void setCurrentNet();
    void setInputsForTrainedNetFromGui();
    void testTrainedNetWithInput();
    void updateConfusionMatrix();
    void showConfusionMatrixHelpBox();
    void closeEvent(QCloseEvent *event);
    void scrollToBottom_toggled();
    void on_actionEnable_Output_toggled(bool b);
    void on_actionSave_PSO_State_triggered();
    void on_actionLoad_PSO_State_triggered();
    void on_resetAndRun_btn_clicked();
    void on_clearState_btn_clicked();

private:
    Ui::MainWindow *ui;
    ANDTrainer *_neuralPsoTrainer = nullptr;
    std::unique_ptr<NeuralNet> _trainedNeuralNet;
    TrainingParameters _params;
    NeuralNet::EdgeType _gb;

    QTime _runTimer;

    // Input Data
    ANDTrainer::InputCache _inputCache;
    std::vector<int> _inputskips;

    bool _runPso = false;
//    bool _fileLoaded = false;

    vector<real> _labelsData;
    vector<vector<real>> _inputData;

#ifdef OPENCL_DEFINED
    std::vector<cl::Device> _cpuDevices;
    std::vector<cl::Device> _gpuDevices;
    std::vector<cl::Device> _allDevices;

    void initializeCL(std::vector<cl::Device> &cpuDevices,
                      std::vector<cl::Device> &gpuDevices,
                      std::vector<cl::Device> &allDevices);
#endif

    void initializeData();
    QString loadInputFileDialog();
    void enableParameterInput(bool b);
    void tryInjectGB();
    void clearPSOState();
    int getNetTypeCBIndex();
    void setNetTypeByIndex(const int & i);
};

#endif // MAINWINDOW_H
