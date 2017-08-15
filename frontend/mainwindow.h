#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QKeyEvent>
#include <QTimer>

#include <fstream>
#include <cinttypes>
#include <vector>
#include <ctime>

#include <boost/thread.hpp>
#include "backend/neuralpso.h"
#include "backend/util.h"
#include "CL/cl.hpp"
#include "frontend/innernetnodesinput.h"

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

    struct LabelInfo {
      uint32_t dataType;
      uint32_t dataDimensions;
      uint32_t numItems;
    };

    struct ImageInfo {
      uint32_t dataType;
      uint32_t dataDimensions;
      uint32_t numItems;
      uint32_t rows;
      uint32_t cols;
    };

protected:

    void keyPressEvent(QKeyEvent * e);
    //void onKeyInput();
    void loadTrainingData(std::string imageFile,
                          std::string labelFile,
                          std::vector<std::vector<vector<uint8_t> > > &trainingImages,
                          std::vector<uint8_t> &trainingLabels);

    bool readMagicNumber(std::ifstream &in,
                         uint32_t &dataType,
                         uint32_t &dimensions,
                         uint32_t &numItems);
    bool readLabelHeading(std::ifstream &in,
                          LabelInfo &lb);
    bool readImageHeading(std::ifstream &in,
                          ImageInfo &im);
    uint32_t char2uint(uint8_t *input);
    uint32_t readUnsignedInt(std::ifstream &input);
    bool readPEFile(std::vector<double> &labels, std::vector<std::vector<double>> &data);

    void initializeCL(std::vector<cl::Device> &cpuDevices,
                      std::vector<cl::Device> &gpuDevices,
                      std::vector<cl::Device> &allDevices);

    void setOutputLabel(const QString & s);

protected slots:
    void runNeuralPso();
    void stopPso();
    void updatePlot();
    void loadFile_btn();
    void applyParameterChanges();
    void setParameterDefaults();
    void updateParameterGui();
    void setInnerNetNodesFromGui();

private:
    Ui::MainWindow *ui;
    NeuralPso *_neuralPso = nullptr;
    PsoParams _pParams;
    NeuralNetParameters _nParams;

    bool _runPso = false;
    bool _fileLoaded = false;

    vector<double> _labelsData;
    vector<vector<double>> _inputData;

    std::vector<cl::Device> _cpuDevices;
    std::vector<cl::Device> _gpuDevices;
    std::vector<cl::Device> _allDevices;

    void initializeData();
    QString loadInputFileDialog();
    void enableParameterInput(bool b);
};

#endif // MAINWINDOW_H
