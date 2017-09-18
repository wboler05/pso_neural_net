#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "PSO/pso.cpp"

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
    Logger::setOutputBrowser(QPointer<QTextBrowser>(ui->output_tb));
    Logger::setEnableScrollToBottom(ui->scrollToBottom_cb->isChecked());
    Logger::setEnableTextBrowser(ui->actionEnable_Output->isChecked());

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

    connect(ui->run_btn, SIGNAL(clicked(bool)), this, SLOT(runNeuralPso()));
    connect(ui->stop_btn, SIGNAL(clicked(bool)), this, SLOT(stopPso()));
    //connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(loadFile_btn()));
    connect(ui->applyParams_btn, SIGNAL(clicked(bool)), this, SLOT(applyParameterChanges()));
    connect(ui->innerNet_btn, SIGNAL(clicked(bool)), this, SLOT(setInnerNetNodesFromGui()));
    connect(ui->setNet_btn, SIGNAL(clicked(bool)), this, SLOT(setCurrentNet()));
    connect(ui->testInput_btn, SIGNAL(clicked(bool)), this, SLOT(testTrainedNetWithInput()));
    connect(ui->scrollToBottom_cb, SIGNAL(toggled(bool)), this, SLOT(scrollToBottom_toggled()));

    connect(ui->a_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));
    connect(ui->b_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));

    connect(ui->actionConfusion_Matrix, SIGNAL(triggered(bool)), this, SLOT(showConfusionMatrixHelpBox()));

    QTimer * updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updatePlot()));
    updateTimer->start(500);

    initializeData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    qDebug() << "MainWindow: Terminating program.";
    Logger::terminate();
    stopPso();
    qDebug() << "MainWindow: Terminated successfully.";
    QMainWindow::closeEvent(event);
}

void MainWindow::scrollToBottom_toggled() {
    Logger::setEnableScrollToBottom(ui->scrollToBottom_cb->isChecked());
}

void MainWindow::on_actionEnable_Output_toggled(bool b) {
    qDebug() << "Auto stuff works.";

    ui->txtBrowser_gb->setEnabled(b);
    ui->txtBrowser_gb->setVisible(b);
    Logger::setEnableTextBrowser(b);
}

void MainWindow::on_actionSave_PSO_State_triggered() {

    if (_neuralPsoTrainer == nullptr) {
        QMessageBox * msgBox = new QMessageBox();
        msgBox->setText("Error, you have not ran the simulation!\nNo data to save.");
        msgBox->exec();
        return;
    }

    QDir curDir(qApp->applicationDirPath());
    QString fileName = QFileDialog::getSaveFileName(this, "Save the PSO State", curDir.absolutePath(), "PSO (*.pso)");
    QString psoState(_neuralPsoTrainer->stringifyState().c_str());

    QFile outputFile(fileName);
    if (outputFile.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream outputStream(&outputFile);
        outputStream << psoState;
        outputFile.close();

        QMessageBox * msgBox = new QMessageBox();
        QString msgBoxTxt;
        msgBoxTxt.append("PSO State Saved!\n");
        msgBoxTxt.append(fileName);
        msgBox->setText(msgBoxTxt);
        msgBox->exec();
        return;
    } else {
        QMessageBox * msgBox = new QMessageBox();
        QString msgBoxTxt;
        msgBoxTxt.append("Error, could not open file:\n");
        msgBoxTxt.append(fileName);
        msgBox->setText(msgBoxTxt);
        msgBox->exec();
        return;
    }
}

void MainWindow::on_actionLoad_PSO_State_triggered() {
    //!TEST!//
    qDebug() << "auto load state triggered";
    QMessageBox *msgBox = new QMessageBox();
    QString msgTxt;

    QDir curDir(qApp->applicationDirPath());
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open PSO State"), curDir.absolutePath(), ("PSO (*.pso)"));

    QFile inputFile(fileName);
    if (inputFile.open(QFile::ReadOnly)) {

        QTextStream inputStream(&inputFile);
        QString psoState = inputStream.readAll();

        clearPSOState();

        if (_neuralPsoTrainer->loadStatefromString(psoState.toStdString())) {
            _params.pp = _neuralPsoTrainer->psoParams();
            _params.np = *_neuralPsoTrainer->neuralNet()->nParams();
            updateParameterGui();

            msgTxt.append("Loaded PSO State: \n" );
            msgTxt.append(fileName);
            updatePlot();
        } else {
            msgTxt.append("PSO State is unreadable!\n" );
            msgTxt.append(fileName);
        }

    } else {
        msgTxt.append("Failed to open file: \n" );
        msgTxt.append(fileName);
    }

    msgBox->setText(msgTxt);
    msgBox->exec();
}

void MainWindow::setCurrentNet() {
    if (_neuralPsoTrainer != nullptr) {
        _trainedNeuralNet = _neuralPsoTrainer->buildNeuralNetFromGb();
        //qDebug() << "Test me baby: " << _trainedNeuralNet->getWeights().size();
        qDebug() << "Updated new neural net.";
    }
}

void MainWindow::initializeData() {
    //loadFile_btn();
    generateAndLabels();
    setParameterDefaults();
    setInputsForTrainedNetFromGui();
}

void MainWindow::generateAndLabels() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0, 1);

    _inputData.resize(TOTAL_GENERATED_LABELS);
    _labelsData.resize(TOTAL_GENERATED_LABELS);

    for (size_t i = 0; i < TOTAL_GENERATED_LABELS; i++) {
        _inputData[i].resize(2);

        double A = dist(gen);
        double B = dist(gen);
        double result =
                ANDTrainer::convertInput(
                    ANDTrainer::convertOutput(A) &&
                    ANDTrainer::convertOutput(B)
                );

        _inputData[i][0] = A;
        _inputData[i][1] = B;
        _labelsData[i] = result;
    }
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

    setNetTypeByIndex(ui->netType_cb->currentIndex());
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

    ui->netType_cb->setCurrentIndex(getNetTypeCBIndex());
}

int MainWindow::getNetTypeCBIndex() {
    switch (_params.np.type) {
    case NeuralNet::Feedforward:
        return 0;
        break;
    case NeuralNet::Recurrent:
        return 1;
        break;
    default:
        return 0;
        break;
    }
}

void MainWindow::setNetTypeByIndex(const int & i) {
    switch (i) {
    case 0:
        _params.np.type = NeuralNet::Feedforward;
        break;
    case 1:
        _params.np.type = NeuralNet::Recurrent;
        break;
    default:
        _params.np.type = NeuralNet::Feedforward;
        break;
    }
}

void MainWindow::setInnerNetNodesFromGui() {
    InnerNetNodesInput * dialog = new InnerNetNodesInput(_params.np);
}

void MainWindow::setParameterDefaults() {
    _params.pp.particles = 50; // 50
    _params.pp.neighbors = 13; // 10
    _params.pp.iterations = 1000;
    _params.pp.delta = 5E-8;
    _params.pp.vDelta = 5E-200;
    _params.pp.termIterationFlag = false;
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
    _params.np.innerNetNodes.push_back(4);
    _params.np.innerNetNodes.push_back(4);
    _params.np.innerNetNodes.push_back(4);
    _params.np.innerNets = _params.np.innerNetNodes.size();
    _params.np.outputs = 1;
    _params.np.testIterations = 500; //500

    _params.fp.floors.accuracy = .05;
    _params.fp.floors.precision = 0.05;
    _params.fp.floors.sensitivity = 0.05;
    _params.fp.floors.specificity = .05;
    _params.fp.floors.f_score = 0.05;
    _params.fp.mse_floor = 0;

    _params.fp.mse_weight = 1;
    _params.fp.weights.accuracy = 0.0;
    _params.fp.weights.precision = 0.0;
    _params.fp.weights.sensitivity = 0.0;
    _params.fp.weights.specificity = 0.0;
    _params.fp.weights.f_score = 0.0;

    updateParameterGui();
}

void MainWindow::setInputsForTrainedNetFromGui() {
    _inputCache.a = ui->a_cb->isChecked();
    _inputCache.b = ui->b_cb->isChecked();
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

    ui->acc_lbl->setText(QString::number((double)ce.accuracy));
    ui->prec_lbl->setText(QString::number((double)ce.precision));
    ui->sens_lbl->setText(QString::number((double)ce.sensitivity));
    ui->spec_lbl->setText(QString::number((double)ce.specificity));
    ui->fscore_lbl->setText(QString::number((double)ce.f_score));

    ui->actPosNum_lbl->setText(QString::number(actPos));
    ui->actPosPerc_lbl->setText(QString::number(actPos / pop));
    ui->actNegNum_lbl->setText(QString::number(actNeg));
    ui->actNegPerc_lbl->setText(QString::number(actNeg / pop));
    ui->predPosNum_lbl->setText(QString::number(prePos));
    ui->predPosPerc_lbl->setText(QString::number(prePos / pop));
    ui->predNegNum_lbl->setText(QString::number(preNeg));
    ui->predNegPerc_lbl->setText(QString::number(preNeg / pop));

    ui->truePosNum_lbl->setText(QString::number((double)ts.tp()));
    ui->truePosPerc_lbl->setText(QString::number((double)ts.tp_norm()));
    ui->trueNegNum_lbl->setText(QString::number((double)ts.tn()));
    ui->trueNegPerc_lbl->setText(QString::number((double)ts.tn_norm()));
    ui->falsePosNum_lbl->setText(QString::number((double)ts.fp()));
    ui->falsePosPerc_lbl->setText(QString::number((double)ts.fp_norm()));
    ui->falseNegNum_lbl->setText(QString::number((double)ts.fn()));
    ui->falseNegPerc_lbl->setText(QString::number((double)ts.fn_norm()));
}

void MainWindow::testTrainedNetWithInput() {
    if (_trainedNeuralNet != nullptr) {
        std::vector<real> newInput;
        //std::vector<real> curInput = _inputCache.inputize();
        std::vector<real> curInput;
        curInput.push_back(ANDTrainer::convertInput(_inputCache.a));
        curInput.push_back(ANDTrainer::convertInput(_inputCache.b));
        for (size_t i = 0; i < curInput.size(); i++) {
            // Remember, skips include PE as the first index, so subtract 1
            bool skipPos=false;
            for (size_t j = 0; j < _inputskips.size(); j++) {
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
        for (size_t i = 0; i < newInput.size(); i++) {
            _trainedNeuralNet->loadInput(newInput[i], i);
        }
        std::vector<real> output = _trainedNeuralNet->process();

        if (output.size() != 1) {
            qDebug() << "What is this output?";
            ui->testInput_output->setText("UNK Output");
            return;
        } else {
            bool result = ANDTrainer::convertOutput(output[0]);
            if (result) {
                ui->testInput_output->setText("Statement is True");
            } else {
                ui->testInput_output->setText("Statement is False");
            }
        }
    }
}

void MainWindow::stopPso() {
    _runPso = false;
    cout << "Ending process.  Please wait. " << endl;

    NeuralPso::interruptProcess();
}

void MainWindow::setOutputLabel(const QString & s) {
    ui->output_lbl->setText(s);
}

void MainWindow::updateFitnessPlot() {
    if (_neuralPsoTrainer) {
        ui->fitnessPlot->plotHistory(_neuralPsoTrainer->historyFromLastRun());
    }
}

void MainWindow::runNeuralPso() {
  if (_runPso) {
      return;
  }

  _runTimer.restart();
  _runPso = true;

  QTimer * fitnessPlotTimer = new QTimer(this);
  fitnessPlotTimer->setInterval(200);
  connect(fitnessPlotTimer, SIGNAL(timeout()), this, SLOT(updateFitnessPlot()));
  fitnessPlotTimer->start();

  setOutputLabel("Training running.");
  enableParameterInput(false);

  // Make sure that parameters are ready
  //!FIXME Not actually updaing parameters.
  applyParameterChanges();
  tellParameters();

  if (_neuralPsoTrainer == nullptr) {
      clearPSOState();
  }

  //NeuralNet *net = _neuralPsoTrainer->neuralNet();
  //ui->neuralNetPlot->setEdges(&(net->getWeights()));

  // Train the net
  tryInjectGB();
  _neuralPsoTrainer->runTrainer();

  TestStatistics::ClassificationError ce;
  _neuralPsoTrainer->classError(&ce);

  stopPso();

  enableParameterInput(true);

  QString completionMsg;
  completionMsg.append("Complete. ");
  completionMsg.append(QString::number(_neuralPsoTrainer->iterations(), 10));
  completionMsg.append(" iterations\t");
  completionMsg.append(QString::number((double)_runTimer.elapsed() / 1000.0));
  completionMsg.append(" seconds");
  setOutputLabel(completionMsg);

  _gb = _neuralPsoTrainer->getGbEdges();

  disconnect(fitnessPlotTimer, SIGNAL(timeout()), this, SLOT(updateFitnessPlot()));
  fitnessPlotTimer->deleteLater();

  qApp->alert(this);
}

void MainWindow::on_resetAndRun_btn_clicked() {
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setText("Will run after clearing state.\nClear state and run?");
    msgBox->addButton(QMessageBox::Yes);
    msgBox->addButton(QMessageBox::Cancel);
    int choice = msgBox->exec();

    switch(choice) {
    case QMessageBox::Yes:
        applyParameterChanges();
        clearPSOState();
        runNeuralPso();
        break;
    default:
        return;
    }
}

void MainWindow::on_clearState_btn_clicked() {
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setText("Will clear PSO State.\nProceed?");
    msgBox->addButton(QMessageBox::Yes);
    msgBox->addButton(QMessageBox::Cancel);
    int choice = msgBox->exec();

    switch(choice) {
    case QMessageBox::Yes:
        clearPSOState();
        break;
    default:
        return;
    }
}

void MainWindow::clearPSOState() {
    if (_neuralPsoTrainer != nullptr) {
        delete _neuralPsoTrainer;
    }

    _neuralPsoTrainer = new ANDTrainer(_params);
    _neuralPsoTrainer->build(_inputData, _labelsData);
    _neuralPsoTrainer->setFunctionMsg("AND");
}

void MainWindow::tryInjectGB() {

    if (ui->actionInject_Global_Best->isChecked()) {
        if (_neuralPsoTrainer != nullptr) {
            if (_gb.size() != 0){
                _neuralPsoTrainer->injectGb(_gb);
            }
        }
    }
}

void MainWindow::updatePlot() {
    if (_neuralPsoTrainer != nullptr) {
        NeuralNet::CombEdgeType * edge = &(_neuralPsoTrainer->gb()->_x);
        ui->neuralNetPlot->setEdges(edge, _params.np.type);
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

void MainWindow::showConfusionMatrixHelpBox() {
    AboutConfusionMatrixDialog *db = new AboutConfusionMatrixDialog();
    connect(this, SIGNAL(destroyed(QObject*)), db, SLOT(close()));
    db->show();
}

void MainWindow::tellParameters() {
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

#ifdef OPENCL_DEFINED

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

#endif

