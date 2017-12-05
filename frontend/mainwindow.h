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

#include "Trainer/OutageTrainer.h"
#include "backend/neuralpso.h"
#include "backend/NeuralNet/NeuralNet.h"
#include "backend/PSO/pso.h"
#include "logger.h"
#include "frontend/innernetnodesinput.h"
#include "maxmemorydialog.h"
#include "slicenumberdialog.h"
#include "particleplotwindow.h"
#include "frontend/Trainer/experimentfileparser.h"

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

//    void generateAndLabels();
    void tellParameters();

protected slots:
    void runNeuralPso();
    void stopPso();
    void updatePlot();
    void applyParameterChanges();
    void setParameterDefaults();
    void updateParameterGui();
    void updateActivationCB();
    void setActivationByCB();
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
    void updateFitnessPlot();
    void on_actionLoad_Input_File_triggered();
    void on_actionMax_Memory_triggered();
    void on_actionSlices_Per_Cache_triggered();
    void updateCacheMaxBytes(const int & bytes);
    void updateSlicesPerCache(const int & slices);
    void on_actionParticle_Plotter_triggered();
    void updateFitnessPlotWindowSize();
    void resizeEvent(QResizeEvent * event);
    void on_testBaseCase_btn_clicked();
    void on_actionSaveSelected_ANN_triggered();
    void on_testFullSet_btn_clicked();
    void on_testProcedure_btn_clicked();
    void updateFileLabel();
    void cleanOutputRangeLineEdit();
    void on_actionLoad_GB_Neural_Net_triggered();
    void updateFromGlobalBestComboBox();

private:
    Ui::MainWindow *ui;
    std::unique_ptr<OutageTrainer> _neuralPsoTrainer;
    std::unique_ptr<NeuralNet> _trainedNeuralNet;
    ConfusionMatrix _currentNetCm;
    std::shared_ptr<TrainingParameters> _params;
    NeuralNet::State _gb;
    QVector<ParticlePlotWindow *> _particlePlotWindow;

    QTime _runTimer;

    // Input Data
    std::shared_ptr<InputCache> _inputCache;
    std::vector<int> _inputskips;

    bool _runPso = false;
    bool _runOnce = false;

    volatile bool _runningAutomatedTestProcedure = true;

#ifdef OPENCL_DEFINED
    std::vector<cl::Device> _cpuDevices;
    std::vector<cl::Device> _gpuDevices;
    std::vector<cl::Device> _allDevices;

    void initializeCL(std::vector<cl::Device> &cpuDevices,
                      std::vector<cl::Device> &gpuDevices,
                      std::vector<cl::Device> &allDevices);
#endif

    void initializeData();
    void initializeCache();
    bool loadIniFile();
    void saveIniFile();
    QString loadInputFileDialog();
    void enableParameterInput(bool b);
    void tryInjectGB();
    void clearPSOState();
    int getNetTypeCBIndex();
    void setNetTypeByIndex(const int & i);
    void applyElementSkips();
    void updateElementSkips();
    void setGlobalBestSelectionBox();
    void getGlobalBestSelectionFromBox();
    void updateOutputRanges();
};

#endif // MAINWINDOW_H
