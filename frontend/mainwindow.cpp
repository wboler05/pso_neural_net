#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

#define DT_UNSIGNED_TYPE 0X08
#define DT_SIGNED_BYTE 0x09
#define DT_SHORT 0X0B
#define DT_INT 0x0C
#define DT_FLOAT 0x0D
#define DT_DOUBLE 0x0E

#define CHAR_TO_INT(x) *(int *) x

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    srand(time(NULL));
    _runTimer.start();

    // Set the logger file
    Logger::setOutputFile("log/run.log");

    time_t now = time(0);
    tm *gmtm = gmtime(&now);

    std::string logFile("logs\\run_");
    logFile += stringPut(gmtm->tm_year+1900);
    logFile += stringPut(gmtm->tm_mon);
    logFile += stringPut(gmtm->tm_mday);
    logFile += "_";
    logFile += stringPut(gmtm->tm_hour);
    logFile += stringPut(gmtm->tm_min);
    logFile += stringPut(gmtm->tm_sec);
    logFile += ".log";

    cout << logFile << endl;

    // Set the logger file
    Logger::setOutputFile(logFile);

    string dateTime;
    dateTime += "\nDate / Time: ";
    dateTime += asctime(gmtm);
    dateTime += "\n";
    Logger::write(dateTime);

    std::string headerString;
    headerString += "   ****************************************\n";
    headerString += "   * PSO Neural Net (Gaussian Activation) *\n";
    headerString += "   *       by William Boler, BSCE         *\n";
    headerString += "   *     Research Assistant, IUPUI        *\n";
    headerString += "   *                                      *\n";
    headerString += "   *  Professor: Dr. Lauren Christopher   *\n";
    headerString += "   *     Created: March 13, 2017          *\n";
    headerString += "   ****************************************\n";
    Logger::write(headerString);


    //cl::Context _context;

    //initializeCL(_cpuDevices, _gpuDevices, _allDevices);

    //! TODO Need to change this functionality
    //boost::thread thread1(this->runNeuralPso);
    //boost::thread thread2(&(this->onKeyInput());
    //runNeuralPso();
    //onKeyInput();

    //thread1.join();
    connect(ui->run_btn, SIGNAL(clicked(bool)), this, SLOT(runNeuralPso()));
    connect(ui->stop_btn, SIGNAL(clicked(bool)), this, SLOT(stopPso()));
    connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(loadFile_btn()));
    connect(ui->applyParams_btn, SIGNAL(clicked(bool)), this, SLOT(applyParameterChanges()));
    connect(ui->innerNet_btn, SIGNAL(clicked(bool)), this, SLOT(setInnerNetNodesFromGui()));
    connect(ui->printGB_btn, SIGNAL(clicked(bool)), this, SLOT(printGB()));
    connect(ui->classError_btn, SIGNAL(clicked(bool)), this, SLOT(printClassError()));
    connect(ui->setNet_btn, SIGNAL(clicked(bool)), this, SLOT(setCurrentNet()));
    connect(ui->testInput_btn, SIGNAL(clicked(bool)), this, SLOT(testTrainedNetWithInput()));

    connect(ui->etco2_dsb, SIGNAL(editingFinished()), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->age_dsb, SIGNAL(editingFinished()), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->sysbloodpres_dsb, SIGNAL(editingFinished()), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->sao2_dsb, SIGNAL(editingFinished()), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->hemoptysis_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->tobacco_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->surgery_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->cardiac_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->dvtpe_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->uls_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));

    connect(ui->actionConfusion_Matrix, SIGNAL(triggered(bool)), this, SLOT(showConfusionMatrixHelpBox()));

    QTimer * updateTimer = new QTimer();
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updatePlot()));
    updateTimer->start(500);

    initializeData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::printGB() {
    if (_neuralPsoTrainer != nullptr) {
        _neuralPsoTrainer->printGB();
    }
}

void MainWindow::printClassError() {
    if (_neuralPsoTrainer != nullptr) {
        TestStatistics::ClassificationError ce;
//        _neuralPsoTrainer->classError(&ce);
    }
}

void MainWindow::setCurrentNet() {
    if (_neuralPsoTrainer != nullptr) {
        _trainedNeuralNet = _neuralPsoTrainer->buildNeuralNetFromGb();
        qDebug() << "Test me baby: " << _trainedNeuralNet->getWeights().size();
    }
}

void MainWindow::initializeData() {
    loadFile_btn();
    setParameterDefaults();
    setInputsForTrainedNetFromGui();
}

void MainWindow::keyPressEvent(QKeyEvent * e) {
    if (e->key() == Qt::Key_C) {
        cout << "Ending process.  Please wait. " << endl;
        NeuralPso::interruptProcess();
    } else if (e->key() == Qt::Key_P) {
        NeuralPso::setToPrint();
    } else if (e->key() == Qt::Key_G) {
        NeuralPso::setToPrintGBNet();
    }
    QMainWindow::keyPressEvent(e);
}

void MainWindow::applyParameterChanges() {
    _params.pp.particles = ui->totalParticles_sb->value();
    _params.pp.neighbors = ui->totalNeighbors_sb->value();
    _params.pp.iterations = ui->totalIterations_sb->value();
    _params.pp.delta = ui->delta_dsb->value();
    _params.pp.window = ui->window_sb->value();
    _params.pp.termIterationFlag = ui->enableIteration_cb->isChecked();
    _params.pp.termDeltaFlag = ui->enableDelta_cb->isChecked();

    _params.np.testIterations = ui->testIt_sb->value();

    _params.fp.mse_floor = ui->mse_floor_dsb->value();
    _params.fp.floors.accuracy = ui->acc_floor_dsb->value();
    _params.fp.floors.precision = ui->pre_floor_dsb->value();
    _params.fp.floors.sensitivity = ui->sen_floor_dsb->value();
    _params.fp.floors.specificity = ui->spe_floor_dsb->value();
    _params.fp.floors.f_score = ui->fscore_floor_dsb->value();

    _params.fp.mse_weight = ui->mse_weight_dsb->value();
    _params.fp.weights.accuracy = ui->acc_weight_dsb->value();
    _params.fp.weights.precision = ui->pre_weight_dsb->value();
    _params.fp.weights.sensitivity = ui->sen_weight_dsb->value();
    _params.fp.weights.specificity = ui->spe_weight_dsb->value();
    _params.fp.weights.f_score = ui->fscore_weight_dsb->value();
}

void MainWindow::updateParameterGui() {
    ui->totalParticles_sb->setValue(_params.pp.particles);
    ui->totalNeighbors_sb->setValue(_params.pp.neighbors);
    ui->totalIterations_sb->setValue(_params.pp.iterations);
    ui->window_sb->setValue(_params.pp.window);
    ui->delta_dsb->setValue(_params.pp.delta);
    ui->enableIteration_cb->setChecked(_params.pp.termIterationFlag);
    ui->enableDelta_cb->setChecked(_params.pp.termDeltaFlag);

    ui->testIt_sb->setValue(_params.np.testIterations);

    ui->mse_floor_dsb->setValue(_params.fp.mse_floor);
    ui->acc_floor_dsb->setValue(_params.fp.floors.accuracy);
    ui->pre_floor_dsb->setValue(_params.fp.floors.precision);
    ui->sen_floor_dsb->setValue(_params.fp.floors.sensitivity);
    ui->spe_floor_dsb->setValue(_params.fp.floors.specificity);
    ui->fscore_floor_dsb->setValue(_params.fp.floors.f_score);

    ui->mse_weight_dsb->setValue(_params.fp.mse_weight);
    ui->acc_weight_dsb->setValue(_params.fp.weights.accuracy);
    ui->pre_weight_dsb->setValue(_params.fp.weights.precision);
    ui->sen_weight_dsb->setValue(_params.fp.weights.sensitivity);
    ui->spe_weight_dsb->setValue(_params.fp.weights.specificity);
    ui->fscore_weight_dsb->setValue(_params.fp.weights.f_score);
}

void MainWindow::setInnerNetNodesFromGui() {
    InnerNetNodesInput * dialog = new InnerNetNodesInput(_params.np);
}

void MainWindow::setParameterDefaults() {
    _params.pp.particles = 50; // 50
    _params.pp.neighbors = 13; // 10
    _params.pp.iterations = 1000;
    _params.pp.delta = 5E-7;
    _params.pp.vDelta = 5E-200;
    _params.pp.termIterationFlag = true;
    _params.pp.termDeltaFlag = true;
    _params.pp.window = 500;

    /*
    NeuralNetParameters nParams;
    nParams.inputs = trainingImages[0].size() * trainingImages[0][0].size();
    nParams.innerNets = 1;
    nParams.innerNetNodes.push_back(100);
    //nParams.innerNetNodes.push_back(50);
    nParams.outputs = 10;
    */
    _params.np.inputs = _inputData[0].size();
    _params.np.innerNetNodes.clear();
    _params.np.innerNetNodes.push_back(3);
    _params.np.innerNetNodes.push_back(2);
    _params.np.innerNets = _params.np.innerNetNodes.size();
    _params.np.outputs = 1;
    _params.np.testIterations = 200; //500

    _params.fp.floors.accuracy = .05;
    _params.fp.floors.precision = 0.05;
    _params.fp.floors.sensitivity = 0.05;
    _params.fp.floors.specificity = .05;
    _params.fp.floors.f_score = 0.05;
    _params.fp.mse_floor = 0;

    _params.fp.mse_weight = 1;
    _params.fp.weights.accuracy = 0.01;
    _params.fp.weights.precision = .4;
    _params.fp.weights.sensitivity = 100.0;
    _params.fp.weights.specificity = 0.001;
    _params.fp.weights.f_score = .01;

    updateParameterGui();
}

void MainWindow::setInputsForTrainedNetFromGui() {
    _inputCache.etco2 = ui->etco2_dsb->value();
    _inputCache.age = ui->age_dsb->value();
    _inputCache.sysBloodPres = ui->sysbloodpres_dsb->value();
    _inputCache.sao2 = ui->sao2_dsb->value();
    _inputCache.hemoptysis = ui->hemoptysis_cb->isChecked();
    _inputCache.tobacco = ui->tobacco_cb->isChecked();
    _inputCache.surgery = ui->surgery_cb->isChecked();
    _inputCache.cardiac = ui->cardiac_cb->isChecked();
    _inputCache.dvtpe = ui->dvtpe_cb->isChecked();
    _inputCache.uls = ui->uls_cb->isChecked();
}

void MainWindow::updateConfusionMatrix() {
    TestStatistics & ts = _neuralPsoTrainer->testStats();
    TestStatistics::ClassificationError ce;
    ts.getClassError(&ce);

    double actPos = ts.tp() + ts.fn();
    double actNeg = ts.fp() + ts.tn();
    double prePos = ts.tp() + ts.fp();
    double preNeg = ts.tn() + ts.fn();
    double pop = ts.population();

    ui->acc_lbl->setText(QString::number(ce.accuracy));
    ui->prec_lbl->setText(QString::number(ce.precision));
    ui->sens_lbl->setText(QString::number(ce.sensitivity));
    ui->spec_lbl->setText(QString::number(ce.specificity));
    ui->fscore_lbl->setText(QString::number(ce.f_score));

    ui->actPosNum_lbl->setText(QString::number(actPos));
    ui->actPosPerc_lbl->setText(QString::number(actPos / pop));
    ui->actNegNum_lbl->setText(QString::number(actNeg));
    ui->actNegPerc_lbl->setText(QString::number(actNeg / pop));
    ui->predPosNum_lbl->setText(QString::number(prePos));
    ui->predPosPerc_lbl->setText(QString::number(prePos / pop));
    ui->predNegNum_lbl->setText(QString::number(preNeg));
    ui->predNegPerc_lbl->setText(QString::number(preNeg / pop));

    ui->truePosNum_lbl->setText(QString::number(ts.tp()));
    ui->truePosPerc_lbl->setText(QString::number(ts.tp_norm()));
    ui->trueNegNum_lbl->setText(QString::number(ts.tn()));
    ui->trueNegPerc_lbl->setText(QString::number(ts.tn_norm()));
    ui->falsePosNum_lbl->setText(QString::number(ts.fp()));
    ui->falsePosPerc_lbl->setText(QString::number(ts.fp_norm()));
    ui->falseNegNum_lbl->setText(QString::number(ts.fn()));
    ui->falseNegPerc_lbl->setText(QString::number(ts.fn_norm()));
}

void MainWindow::testTrainedNetWithInput() {
    if (_trainedNeuralNet != nullptr) {
        std::vector<double> newInput;
        std::vector<double> curInput = _inputCache.inputize();
        for (int i = 0; i < curInput.size(); i++) {
            // Remember, skips include PE as the first index, so subtract 1
            bool skipPos=false;
            for (int j = 0; j < _inputskips.size(); j++) {
                if (_inputskips[j] == i+1) {
                    skipPos = true;
                    break;
                }
            }
            if (skipPos) {
                continue;
            } else {
                newInput.push_back(curInput[i]);
            }
        }

        if (_trainedNeuralNet->totalInputs() != newInput.size()) {
            qDebug() << "Something broke";
            ui->testInput_output->setText("Failed to process.");
            return;
        }

        _trainedNeuralNet->resetInputs();
        for (int i = 0; i < newInput.size(); i++) {
            _trainedNeuralNet->loadInput(newInput[i], i);
        }
        std::vector<double> output = _trainedNeuralNet->process();

        if (output.size() != 1) {
            qDebug() << "What is this output?";
            ui->testInput_output->setText("UNK Output");
            return;
        } else {
            if (output[0] < 0.5) {
                ui->testInput_output->setText("No PE");
            } else if (output[0] >= 0.5) {
                ui->testInput_output->setText("Det PE");
            } else {
                ui->testInput_output->setText("Confused");
            }
        }
    }
}

void MainWindow::stopPso() {
    _runPso = false;
    cout << "Ending process.  Please wait. " << endl;
    enableParameterInput(true);

    QString completionMsg;
    completionMsg.append("Complete. ");
    completionMsg.append(QString::number(_neuralPsoTrainer->iterations(), 10));
    completionMsg.append(" iterations\t");
    completionMsg.append(QString::number((double)_runTimer.elapsed() / 1000.0));
    completionMsg.append(" seconds");
    setOutputLabel(completionMsg);

    NeuralPso::interruptProcess();
}

void MainWindow::setOutputLabel(const QString & s) {
    ui->output_lbl->setText(s);
}

void MainWindow::runNeuralPso() {
  /// image.rows.cols
  //vector<vector<vector<uint8_t> > > trainingImages;
  //vector<uint8_t> trainingLabels;

  if (_runPso) {
      return;
  }

  _runTimer.restart();

  _runPso = true;
  setOutputLabel("Training running.");
  enableParameterInput(false);

  if (!_fileLoaded) {
    return;
  }

    // Make sure that parameters are ready
  applyParameterChanges();

  std::string outputString;

  outputString += "\n\nInputs: ";
  outputString += stringPut(_params.np.inputs);
  outputString += "\nInner Nets: ";
  outputString += stringPut(_params.np.innerNets);
  outputString += "\n";
  for (uint i = 0; i < _params.np.innerNetNodes.size(); i++) {
    outputString += " - ";
    outputString += stringPut(_params.np.innerNetNodes[i]);
    outputString += "\n";
    //cout << " - " << nParams.innerNetNodes[i] << endl;
  }
  outputString += "Tests per train(min): ";
    outputString += stringPut(_params.np.testIterations);
    outputString += "\n";
  outputString += "Particles: ";
    outputString += stringPut(_params.pp.particles);
    outputString += "\nNeighbors: ";
    outputString += stringPut(_params.pp.neighbors);
    outputString += "\n";
  outputString += "Minimum Particle Iterations: ";
    outputString += stringPut(_params.pp.iterations);
    outputString += "\n";

  //cout << "Tests per train(min): " << nParams.testIterations << endl;
  //cout << "Particles: " << pParams.particles << "\nNeighbors: " << pParams.neighbors << endl;
  //cout << "Minimum Particle Iterations: " << pParams.iterations << endl;
  Logger::write(outputString);

  for (uint i = 0; i < _labelsData.size(); i++) {
    if (_labelsData[i] != 1) {
      _labelsData[i] = 0;
    }
  }

  if (_neuralPsoTrainer != nullptr) {
      delete _neuralPsoTrainer;
  }

  PETrainer *np = new PETrainer(_params);
  _neuralPsoTrainer = np;
  //np->build(trainingImages, trainingLabels);
  np->build(_inputData, _labelsData);
  np->setFunctionMsg("PE");

  NeuralNet *net = np->neuralNet();

  //ui->neuralNetPlot->setEdges(&(net->getWeights()));

  // Train the net
  np->runTrainer();

  /*
  for (int j = 0; j < 15; j++) { // Test point
    np->testGB();
  }
  */

  TestStatistics::ClassificationError ce;
  np->classError(&ce);

  stopPso();

}

void MainWindow::loadFile_btn() {
    _fileLoaded = readPEFile(_labelsData, _inputData);
}

void MainWindow::updatePlot() {
    if (_neuralPsoTrainer != nullptr) {
        NeuralNet::EdgeType * edge = &(_neuralPsoTrainer->gb()->_x);
        ui->neuralNetPlot->setEdges(edge);
        updateConfusionMatrix();

        if (_runPso) {
            if (_neuralPsoTrainer->checkTermProcess()) {
                stopPso();
            } else {

                QString completionMsg;
                completionMsg.append("Training running. ");
                completionMsg.append(QString::number(_neuralPsoTrainer->iterations(), 10));
                completionMsg.append(" iterations");
                setOutputLabel(completionMsg);
            }
        }
    }
}

void MainWindow::enableParameterInput(bool b) {
    ui->param_gb->setEnabled(b);
}

void MainWindow::loadTrainingData(string imageFile, string labelFile, vector<vector<vector<uint8_t> > > &trainingImages, vector<uint8_t> &trainingLabels) {
  ifstream trainLabelFile(labelFile, ios::binary);

  uint8_t temp;
  LabelInfo trainingLabelInfo;

  if (readLabelHeading(trainLabelFile, trainingLabelInfo)) {
    cout << "Data type: " << trainingLabelInfo.dataType
      << "\tDimensions: " << trainingLabelInfo.dataDimensions << "\tNum items: " << trainingLabelInfo.numItems << endl;
  }

  trainingLabels.resize(trainingLabelInfo.numItems);

  for (uint32_t i = 0; i < trainingLabelInfo.numItems; i++) {
    if (trainLabelFile.eof()) {
      cout << "You can't count, son! " << trainingLabelInfo.numItems << " != " << i << endl;
      break;
    } else {
      temp = trainLabelFile.get();
      trainingLabels[i] = temp;
      //cout << "(" << i << "): " << (uint32_t) trainingLabels.at(i) << endl;
    }
  }
  trainLabelFile.close();


  ifstream trainImageFile(imageFile, ios::binary);
  // Load training images
  ImageInfo trainingImageInfo;
  if (readImageHeading(trainImageFile, trainingImageInfo)) {
    cout << "Image file read. " << endl;
  } else {
    cout << "Image file broke, yo." << endl;
  }

  trainingImages.resize(trainingImageInfo.numItems);
  for (uint i = 0; i < trainingImageInfo.numItems; i++) {
    trainingImages[i].resize(trainingImageInfo.rows);
    for (uint j = 0; j < trainingImageInfo.rows; j++) {
      trainingImages[i][j].resize(trainingImageInfo.cols);
    }
  }

  cout << "Allocated image space." << endl;

  for (uint i = 0; i < trainingImageInfo.numItems; i++) {
    for (uint j = 0; j < trainingImageInfo.rows; j++) {
      for (uint k = 0; k < trainingImageInfo.cols; k++) {
        temp = trainImageFile.get();
        trainingImages[i][j][k] = temp;
      }
    }
  }
  trainImageFile.close();

  cout << "Loaded training images." << endl;

}

QString MainWindow::loadInputFileDialog() {
    qApp->processEvents();
    QDir curDir(qApp->applicationDirPath());
    curDir.cdUp();

    QFileDialog * fileDialog = new QFileDialog();
    QString fileString = fileDialog->getOpenFileName(
                this,
                tr("Open FNNPSOGSA data"),
                curDir.absolutePath(),
                tr("Data Files (*.csv)"));
    return fileString;
}

void MainWindow::showConfusionMatrixHelpBox() {
    AboutConfusionMatrixDialog *db = new AboutConfusionMatrixDialog();
    connect(this, SIGNAL(destroyed(QObject*)), db, SLOT(close()));
    db->show();
}

bool MainWindow::readPEFile(vector<double> &labels, vector<vector<double>> &data) {

    std::string file = loadInputFileDialog().toStdString();

    if (file.empty()) {
        cout << "File could not be read." << endl;
        return false;
    }

  //ifstream inputFile("FNNPSOGSAclot/clean.csv");
    ifstream inputFile(file);

  // Select data columns to skip (never select '0'th column!)
  vector<int> skips = {3, 5, 6, 8};
  //vector<int> skips;
  _inputskips = skips;

  if (!inputFile.is_open()) {
    cout << "File could not be read." << endl;
    return false;
  }

  labels.clear();
  data.clear();

  string temp;
  getline(inputFile, temp, '\n');

  while (getline(inputFile, temp)) {

    vector<double> inData;
    string::size_type prevP = 0, pos = 0;
    int index = 0;
    while ((pos = temp.find(',', pos)) != std::string::npos) {
      bool skipLine = false;
      for (uint i = 0; i < skips.size(); i++) {
        if (skips[i] == 0) continue;
        if (index == skips[i]) {
          skipLine = true;
          break;
        }
      }
      index++;
      if (!skipLine) {
        std::string substring(temp.substr(prevP, pos-prevP));

        double dig;
        {
          stringstream ss;
          ss << substring;
          ss >> dig;
        }
        inData.push_back(dig);
      }
      prevP = ++pos;
    }
    labels.push_back(inData[0]);
    vector<double> data_;
    data_.resize(inData.size()-1);
    for(size_t i = 1; i < inData.size(); i++) {
      data_[i-1] = inData[i];
    }
    data.push_back(data_);
  }
  inputFile.close();
  return true;
}




//bool MainWindow::readMagicNumber(ifstream &in, uint32_t &dataType, uint32_t &dimensions, uint32_t &numItems) {}
bool MainWindow::readLabelHeading(ifstream &in, LabelInfo &lb) {

  in.seekg(0, in.beg);

  unsigned char temp;

  for (int i = 0; i < 2; i++) {
      temp = in.get();
      if (temp != 0)
        return false;
  }

  lb.dataType = (uint32_t) in.get();
  lb.dataDimensions = (uint32_t) in.get();

  lb.numItems = readUnsignedInt(in);

  return true;
}

bool MainWindow::readImageHeading(ifstream &in, ImageInfo &im) {
  in.seekg(0, in.beg);

  unsigned char temp;

  for (int i = 0; i < 2; i++) {
    temp = in.get();
    if (temp != 0)
      return false;
  }

  im.dataType = (uint32_t) in.get();
  im.dataDimensions = (uint32_t) in.get();

  im.numItems = readUnsignedInt(in);
  im.rows = readUnsignedInt(in);
  im.cols = readUnsignedInt(in);

  return true;
}

uint32_t MainWindow::readUnsignedInt(ifstream &input) {
  uint8_t temp[4];
  for (int i = 3; i >= 0; i--) {
    temp[i] = input.get();
  }
  return char2uint(temp);
}

uint32_t MainWindow::char2uint(uint8_t *input) {
  uint32_t num = 0x00000000;
  num +=  ((uint32_t) input[0])         & 0x000000FF;
  num +=  (((uint32_t) input[1]) << 8)  & 0x0000FF00;
  num +=  (((uint32_t) input[2]) << 16) & 0x00FF0000;
  num +=  (((uint32_t) input[3]) << 24) & 0xFF000000;
  return num;
}

void MainWindow::initializeCL(std::vector<cl::Device> &cpuDevices,
                  std::vector<cl::Device> &gpuDevices,
                  std::vector<cl::Device> &allDevices)
{

    std::vector<cl::Platform> platforms;

    cl::Platform::get(&platforms);

    for (uint i = 0; i < platforms.size(); i++) {
        std::vector<cl::Device> devices;
        platforms[i].getDevices(CL_DEVICE_TYPE_CPU, &devices);
        for (uint j = 0; j < devices.size(); j++) {
            cpuDevices.push_back(devices.at(j));
            allDevices.push_back(devices.at(j));
        }
    }

    for (uint i = 0; i < platforms.size(); i++) {
        std::vector<cl::Device> devices;
        platforms[i].getDevices(CL_DEVICE_TYPE_GPU, &devices);
        for (uint j = 0; j < devices.size(); j++) {
            gpuDevices.push_back(devices.at(j));
            allDevices.push_back(devices.at(j));
        }
    }

    cout << "C++ OpenCL.  Devices found: " << allDevices.size() << endl;
    cout << " - Total CPU Devices: " << cpuDevices.size() << endl;
    for (uint i = 0; i < cpuDevices.size(); i++) {
        cl::Device &dev = cpuDevices[i];
        std::string deviceName;
        std::string deviceVendor;
        std::string deviceVersion;
        dev.getInfo(CL_DEVICE_NAME, &deviceName);
        dev.getInfo(CL_DEVICE_VENDOR, &deviceVendor);
        dev.getInfo(CL_DEVICE_VERSION, &deviceVersion);
        cout << " -- " << deviceName.c_str() << "\t " << deviceVendor.c_str() << "\t" << deviceVersion.c_str() << endl;
    }
    cout << " - Total GPU Devices: " << gpuDevices.size() << endl;
    for (uint i = 0; i < gpuDevices.size(); i++) {
        cl::Device &dev = gpuDevices[i];
        std::string deviceName;
        std::string deviceVendor;
        std::string deviceVersion;
        dev.getInfo(CL_DEVICE_NAME, &deviceName);
        dev.getInfo(CL_DEVICE_VENDOR, &deviceVendor);
        dev.getInfo(CL_DEVICE_VERSION, &deviceVersion);
        cout << " -- " << deviceName.c_str() << "\t " << deviceVendor.c_str() << "\t" << deviceVersion.c_str() << endl;
    }
}


/*

FILE FORMATS FOR THE MNIST DATABASE

The data is stored in a very simple file format designed for storing vectors and multidimensional matrices. General info on this format is given at the end of this page, but you don't need to read that to use the data files.
All the integers in the files are stored in the MSB first (high endian) format used by most non-Intel processors. Users of Intel processors and other low-endian machines must flip the bytes of the header.

There are 4 files:

train-images-idx3-ubyte: training set images
train-labels-idx1-ubyte: training set labels
t10k-images-idx3-ubyte:  test set images
t10k-labels-idx1-ubyte:  test set labels

The training set contains 60000 examples, and the test set 10000 examples.

The first 5000 examples of the test set are taken from the original NIST training set. The last 5000 are taken from the original NIST test set. The first 5000 are cleaner and easier than the last 5000.

TRAINING SET LABEL FILE (train-labels-idx1-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000801(2049) magic number (MSB first)
0004     32 bit integer  60000            number of items
0008     unsigned byte   ??               label
0009     unsigned byte   ??               label
........
xxxx     unsigned byte   ??               label
The labels values are 0 to 9.

TRAINING SET IMAGE FILE (train-images-idx3-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000803(2051) magic number
0004     32 bit integer  60000            number of images
0008     32 bit integer  28               number of rows
0012     32 bit integer  28               number of columns
0016     unsigned byte   ??               pixel
0017     unsigned byte   ??               pixel
........
xxxx     unsigned byte   ??               pixel
Pixels are organized row-wise. Pixel values are 0 to 255. 0 means background (white), 255 means foreground (black).

TEST SET LABEL FILE (t10k-labels-idx1-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000801(2049) magic number (MSB first)
0004     32 bit integer  10000            number of items
0008     unsigned byte   ??               label
0009     unsigned byte   ??               label
........
xxxx     unsigned byte   ??               label
The labels values are 0 to 9.

TEST SET IMAGE FILE (t10k-images-idx3-ubyte):

[offset] [type]          [value]          [description]
0000     32 bit integer  0x00000803(2051) magic number
0004     32 bit integer  10000            number of images
0008     32 bit integer  28               number of rows
0012     32 bit integer  28               number of columns
0016     unsigned byte   ??               pixel
0017     unsigned byte   ??               pixel
........
xxxx     unsigned byte   ??               pixel
Pixels are organized row-wise. Pixel values are 0 to 255. 0 means background (white), 255 means foreground (black).






the IDX file format is a simple format for vectors and multidimensional matrices of various numerical types.
The basic format is

magic number
size in dimension 0
size in dimension 1
size in dimension 2
.....
size in dimension N
data

The magic number is an integer (MSB first). The first 2 bytes are always 0.

The third byte codes the type of the data:
0x08: unsigned byte
0x09: signed byte
0x0B: short (2 bytes)
0x0C: int (4 bytes)
0x0D: float (4 bytes)
0x0E: double (8 bytes)

The 4-th byte codes the number of dimensions of the vector/matrix: 1 for vectors, 2 for matrices....

The sizes in each dimension are 4-byte integers (MSB first, high endian, like in most non-Intel processors).

The data is stored like in a C array, i.e. the index in the last dimension changes the fastest.

*/

