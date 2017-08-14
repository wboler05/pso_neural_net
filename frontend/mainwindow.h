#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>

#include <fstream>
#include <cinttypes>
#include <vector>
#include <ctime>

#include <boost/thread.hpp>
#include "backend/neuralpso.h"
#include "backend/util.h"
#include "CL/cl.hpp"

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

    void onKeyInput();
    void runNeuralPso();
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

private:
    Ui::MainWindow *ui;

    std::vector<cl::Device> _cpuDevices;
    std::vector<cl::Device> _gpuDevices;
    std::vector<cl::Device> _allDevices;
};

#endif // MAINWINDOW_H
