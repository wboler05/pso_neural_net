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

// Where everything begins
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _runTimer.start();

    ///TEST

    NeuralNet::NeuralNetParameters np;
    np.inputs = 2;
    np.innerNetNodes.push_back(1);
    np.outputs = 1;
    np.type = NeuralNet::Feedforward;

    NeuralNet net(np);
    NeuralNet::State netState = net.state();
    qDebug() << "Test Net: " << "\nState Size: " << netState.size();
    qDebug() << "Left Nodes: " << netState[1].size();
    netState[1][0][0] = 1;
    netState[1][1][0] = 1;
    netState[1][2][0] = 0;
    netState[2][0][0] = 1;
    netState[2][1][0] = 0;
    if (!net.setState(netState)) {
        qDebug() << "MainWindow: Failed to set NeuralNet. Test case failure.";
    }

    NeuralNet newNet;
    if (!newNet.setState(netState)) {
        qDebug() << "MainWindow: Failed to create a net from state. Test Case.";
    }

    std::vector<real> netInput = {1, 1};
    qDebug() << "Inputs: ";
    for (auto val : netInput) {
        qDebug() << " - " << val;
    }


    net.loadInputs(netInput);
    std::vector<real> output = net.process();

    qDebug() << "Output: " << output[0];

    newNet.loadInputs(netInput);
    output = net.process();

    qDebug() << "New Output: " << output[0];

    /// end test

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
    headerString += "   ***************************************\n";
    headerString += "   * PSO Neural Net - Outage Prediction  *\n";
    headerString += "   *       by William Boler, BSCE        *\n";
    headerString += "   *     Research Assistant, IUPUI       *\n";
    headerString += "   *         Kevin Galler, MSCE          *\n";
    headerString += "   *               IUPUI                 *\n";
    headerString += "   *                                     *\n";
    headerString += "   *         Course: ECE 57000           *\n";
    headerString += "   *      Created: March 13, 2017        *\n";
    headerString += "   ***************************************\n";
    Logger::write(headerString);


    //cl::Context _context;

    //initializeCL(_cpuDevices, _gpuDevices, _allDevices);

    // Connect signals to slots
    connect(ui->run_btn, SIGNAL(clicked(bool)), this, SLOT(runNeuralPso()));
    //connect(ui->stop_btn, SIGNAL(clicked(bool)), this, SLOT(stopPso()));
    //connect(ui->actionLoad_File, SIGNAL(triggered(bool)), this, SLOT(loadFile_btn()));
    connect(ui->applyParams_btn, SIGNAL(clicked(bool)), this, SLOT(applyParameterChanges()));
    connect(ui->innerNet_btn, SIGNAL(clicked(bool)), this, SLOT(setInnerNetNodesFromGui()));
    connect(ui->setNet_btn, SIGNAL(clicked(bool)), this, SLOT(setCurrentNet()));
//Disable until we can fix    connect(ui->testInput_btn, SIGNAL(clicked(bool)), this, SLOT(testTrainedNetWithInput()));
    connect(ui->scrollToBottom_cb, SIGNAL(toggled(bool)), this, SLOT(scrollToBottom_toggled()));

//    connect(ui->a_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));
//    connect(ui->b_cb, SIGNAL(clicked(bool)), this, SLOT(setInputsForTrainedNetFromGui()));

    connect(ui->actionConfusion_Matrix, SIGNAL(triggered(bool)), this, SLOT(showConfusionMatrixHelpBox()));
    connect(ui->fitnessPlotWindow_sb, SIGNAL(valueChanged(int)), this, SLOT(updateFitnessPlotWindowSize()));

    connect(ui->outputRange_le, SIGNAL(editingFinished()), this, SLOT(cleanOutputRangeLineEdit()));
    connect(ui->globalBestSelection_cb, SIGNAL(currentIndexChanged(int)), this, SLOT(updateFromGlobalBestComboBox()));

    QTimer * updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updatePlot()));
    updateTimer->start(500);

    ui->OutageInputBox->setVisible(false);
    ui->fitnessLimitsWidget->setVisible(false);

    // Init before running things
    initializeData();

    on_actionEnable_Output_toggled(false);
    updateOutputRanges();

    QSize windowSize(1500, 850);
//    QPoint dockOffset(100, 75);
//    QPoint dockPos = pos() + dockOffset;
//    dockPos.rx() += size().rwidth();
//    ui->dockWidget->move(dockPos);

    this->resize(windowSize);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {

    saveIniFile();

    if (_particlePlotWindow.size() > 0) {
        qDebug() << "Closing your plot window.";
        foreach(auto p, _particlePlotWindow) {
            p->close();
        }

        _particlePlotWindow.clear();
    }

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

void MainWindow::on_actionLoad_Input_File_triggered() {
    /**TEST**/
    qDebug() << "Don't forget to debug the new input_file function in MainWindow.";
    QDir curDir(_params->cp.inputFileName);
    if (!curDir.exists()) {
        curDir = QDir(qApp->applicationDirPath());
    }
    //QDir curDir(qApp->applicationDirPath());
    QString filename = QFileDialog::getOpenFileName(this, "Open input file:", curDir.absolutePath(), "CSV (*.csv)");
    if (filename != "") {

        _params->cp.inputFileName = filename;
        initializeCache();
        clearPSOState();

        /*
        if (!_inputCache->reloadCache(filename)) {
            QMessageBox fileLoadFailed;
            fileLoadFailed.setText("Error: could not load cache from file!");
            fileLoadFailed.exec();
        } else {
            if (_neuralPsoTrainer) {
                _neuralPsoTrainer->setInputCache(_inputCache);
            }
            _params->cp = _inputCache->cacheParams();
        }
        */
    }
    updateFileLabel();
}

void MainWindow::updateFileLabel() {
    QString fileLabelString;
    fileLabelString.append("File: " );
    fileLabelString.append(_params->cp.inputFileName);
    ui->file_lbl->setText(fileLabelString);
}

void MainWindow::on_actionMax_Memory_triggered() {
    /**TEST**/
    MaxMemoryDialog * mDialog = new MaxMemoryDialog(static_cast<int>(_params->cp.maxBytes));
    connect(mDialog, SIGNAL(maxBytesUpdated(int)), this, SLOT(updateCacheMaxBytes(int)));
}

void MainWindow::updateCacheMaxBytes(const int & bytes) {
    /**TEST**/
    _params->cp.maxBytes = static_cast<size_t>(bytes);
    if (_inputCache) {
        _inputCache->setMaxBytes(static_cast<size_t>(bytes));
    }
}

void MainWindow::on_actionSlices_Per_Cache_triggered() {
    /**TEST**/
    SliceNumberDialog * sDialog = new SliceNumberDialog(
                static_cast<int>(_params->cp.totalSlicesPerCache));
    connect(sDialog, SIGNAL(slicesUpdated(int)), this, SLOT(updateSlicesPerCache(int)));
}

void MainWindow::updateSlicesPerCache(const int & slices) {
    /**TEST**/
    _params->cp.totalSlicesPerCache = static_cast<size_t>(slices);
    if (_inputCache) {
        _inputCache->setTotalSlicesPerCache(static_cast<size_t>(slices));
    }
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
            _params->pp = _neuralPsoTrainer->psoParams();
            _params->np = *_neuralPsoTrainer->neuralNet()->nParams();
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

void MainWindow::cleanOutputRangeLineEdit() {
    QString lineEdit = ui->outputRange_le->text();
    QStringList commaSeparated = lineEdit.split(',');
    for (int i = 0; i < commaSeparated.length(); i++) {
        QString stripped = commaSeparated[i];
        stripped.remove(QRegExp("[^\\d]"));
        commaSeparated[i] = stripped;
    }

    for (int i = 0; i < commaSeparated.length(); i++) {
        if (commaSeparated[i].length() == 0) {
            commaSeparated.removeAt(i);
            i--;
        }
    }

    lineEdit.clear();
    for(int i = 0; i < commaSeparated.length(); i++) {
        lineEdit.append(commaSeparated[i]);
        if (i != commaSeparated.length()-1) {
            lineEdit.append(',');
        }
    }
    ui->outputRange_le->setText(lineEdit);
}

void MainWindow::setCurrentNet() {
    if (_neuralPsoTrainer != nullptr) {
        _trainedNeuralNet = _neuralPsoTrainer->buildNeuralNetFromGb();
        //qDebug() << "Test me baby: " << _trainedNeuralNet->getWeights().size();
        qDebug() << "Updated new neural net.";
    }
}

void MainWindow::on_actionSaveSelected_ANN_triggered() {
    using namespace NeuralPsoStream;

    if (_neuralPsoTrainer == nullptr) {
        QMessageBox * msgBox = new QMessageBox();
        msgBox->setText("Error, you have not ran the simulation!\nNo data to save.");
        msgBox->exec();
        return;
    }

    QDir curDir(qApp->applicationDirPath());
    QString fileName = QFileDialog::getSaveFileName(this, "Save the Selected Best", curDir.absolutePath(), "ANN State (*.state)");

    //QString psoState(_neuralPsoTrainer->stringifyState().c_str());
    std::string psoState;
    psoState.append(stringifyParamsVector<size_t>("inputSkips", OutageDataWrapper::inputSkips()));
    psoState.append(stringifyParamsNugget("history", _params->inputHistorySize));
    psoState.append(stringifyParamsNugget("activation_function", static_cast<size_t>(_params->np.act)));
    psoState.append(openToken("_best_overall_gb"));
    psoState.append("\n");
    psoState.append(stringifyState(_neuralPsoTrainer->getOverallBest().state));
    psoState.append(closeToken("_best_overall_gb"));


    QFile outputFile(fileName);
    if (outputFile.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream outputStream(&outputFile);
        outputStream << QString(psoState.c_str());
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

void MainWindow::on_actionLoad_GB_Neural_Net_triggered() {
    using namespace NeuralPsoStream;

    QDir curDir(qApp->applicationDirPath());
    QString fileName = QFileDialog::getOpenFileName(this, "Load the ANN", curDir.absolutePath(), "ANN State (*.state)");

    QFile inputFile(fileName);
    QString rawANNState;
    if (inputFile.open(QFile::ReadOnly)) {
        QTextStream inputStream(&inputFile);
        rawANNState = inputStream.readAll();

        std::string cleanANNState = rawANNState.toStdString();
        cleanInputString(cleanANNState);

        std::vector<size_t> skips ;
        if (!vectorFromString<size_t>(cleanANNState, "inputSkips", skips)) {
            qDebug( )<< "Failed to read skips from ANN state. on_actionLoad_GB_Neural_Net_triggered()";
            return;
        }

        std::string historyString = subStringByToken(cleanANNState, "history");
        size_t historyVal = numberFromString<size_t>(historyString);
        qDebug() << "Loaded History Size String: " << historyString.c_str();
        qDebug() << "\tParsed Value: " << historyVal;

        std::string activationString = subStringByToken(cleanANNState, "activation_function");
        NeuralNet::Activation act = static_cast<NeuralNet::Activation>(numberFromString<size_t>(activationString));
        qDebug() << "Loaded Activation String: " << activationString.c_str();
        qDebug() << "\tParsed value: " << act;

        std::string stateString = subStringByToken(cleanANNState, "_best_overall_gb");
        NeuralNet::State newState = stateFromString(stateString);

        OutageDataWrapper::setInputSkips(skips);
        for (size_t i = 0;i < skips.size(); i++) {
            qDebug() << "Skip " << skips[i];
        }

        NeuralNet::NeuralNetParameters newParams = NeuralNet::paramsFromState(newState);

        _params->inputHistorySize = historyVal;
        _params->np.act = act;
        _params->np.inputs = newParams.inputs;
        _params->np.innerNetNodes = newParams.innerNetNodes;
        _params->np.outputs = newParams.outputs;
        _params->ep.setSkipsFromVector(skips);

        updateElementSkips();

        clearPSOState();

        _trainedNeuralNet = std::make_unique<NeuralNet>(_params->np, newState);

        updateParameterGui();
    }
}

void MainWindow::initializeData() {
    if (!_params) {
        _params = std::make_shared<TrainingParameters>();
    }

    ui->globalBestSelection_cb->setCurrentIndex(1);

    bool iniFileLoaded = loadIniFile();

    setParameterDefaults();
    setInputsForTrainedNetFromGui();

    updateFitnessPlotWindowSize();

    if (!iniFileLoaded) {
        saveIniFile();
    }
}

bool MainWindow::loadIniFile() {
    QString iniFile("default.ini");

    QString filePath(qApp->applicationDirPath());
    filePath.append(QDir::separator());
    filePath.append(iniFile);

    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "File not found: " << filePath;
        return false;
    }

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "Unable to open file: " << iniFile;
        return false;
    }

    QTextStream input(&file);
    QString lastDataInputPath = input.readLine();
    _params->cp.inputFileName = lastDataInputPath;
    file.close();

    return true;
}

void MainWindow::saveIniFile() {
    QString iniFile("default.ini");
    QString filePath(qApp->applicationDirPath());
    filePath.append(QDir::separator());
    filePath.append(iniFile);

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Unable to save to file: " << filePath;
        return;
    } else {
        qDebug() << "Saving to ini: " << filePath;
    }

    QTextStream output(&file);
    output << _params->cp.inputFileName;
    file.close();
}

void MainWindow::initializeCache() {

    _inputCache = std::make_shared<InputCache>(_params->cp);

    while (!_inputCache->validFile()) {
        _params->cp.inputFileName = QFileDialog::getOpenFileName(
                    this,
                    "Get input data",
                    qApp->applicationDirPath(),
                    "CSV (*.csv)");
        if (_params->cp.inputFileName.isNull()) {
            qDebug() << "Error, no input data loaded.";
        }
        _inputCache = std::make_shared<InputCache>(_params->cp);
        if (!_inputCache->validFile()) {
            qWarning() << "Error, unable to load data file.";
            exit(1);
        } else {
            break;
        }
    }

    if (_inputCache->validFile()) {

        for (size_t i = 0; i < _inputCache->totalInputItemsInFile(); i++) {
            OutageDataWrapper index = (*_inputCache)[i];
//            qDebug() <<
//                "(" << i << "): date(" <<
//                index._date.day() << "/" <<
//                index._date.month() << "/" <<
//                index._date.year() << ") Temp(" <<
//                static_cast<double>(index._temp.hi()) << "," <<
//                static_cast<double>(index._temp.avg()) << "," <<
//                static_cast<double>(index._temp.lo()) << ")\tAffected Customers: " <<
//                index._affectedCustomers << "\tOutage: " << OutageDataWrapper::bool2Double(index._outage) <<
//                "\tLOA: " << index._loa <<
//                "\tLat: " << index._latlong.latitude() << "\tLong: " << index._latlong.longitude();
        }
        if (_inputCache->length() > 0) {
            ui->confusionMatrix->setNumberOfClassifiers((*_inputCache)[0].outputize().size());
        }
        qDebug() << "Cache creation complete.";
    } else {
        qDebug() << "Failed to open input file.";
        exit(1);
    }
}

void MainWindow::setParameterDefaults() {
    if (!_params) {
        _params = std::make_shared<TrainingParameters>();
    }

    //_params->cp.inputFileName = QString ("C:\\Users\\wboler\\Desktop\\TestCodeHere\\pso_neural_net\\Outage Data\\Final Sets\\ECE570_Final_Dataset.csv");
    _params->cp.maxBytes = 512*1024*1024;
    _params->cp.totalSlicesPerCache = 8;
    _params->cp.headerSize = 0;

    initializeCache();

    _params->pp.population = 50; // 90 | 50 | 150
    _params->pp.neighbors = 5; // 5 | 10 | 30
    _params->pp.minEpochs = 50;
    _params->pp.maxEpochs = 100; // 500 | 1000
    _params->pp.delta = 5E-8L;
    _params->pp.vDelta = 5E-200L;
    _params->pp.termMinEpochsFlag = false;
    _params->pp.termMaxEpochsFlag = true;
    _params->pp.termDeltaFlag = false;
    _params->pp.windowSize = 1500;
    _params->pp.dt = 0.5; // .025
    _params->pp.tempDtFlag = false;

    /*
    NeuralNetParameters nParams;
    nParams.inputs = trainingImages[0].size() * trainingImages[0][0].size();
    nParams.innerNets = 1;
    nParams.innerNetNodes.push_back(100);
    //nParams.innerNetNodes.push_back(50);
    nParams.outputs = 10;
    */

    OutageDataWrapper::setInputSkips(_params->ep.inputSkips());

    OutageDataWrapper dataWrapper = (*_inputCache)[0];
qDebug() << "Made it here.";
    _params->np.inputs = static_cast<int>( dataWrapper.inputSize(ui->inputHistorySize_sb->value()) );
    _params->np.innerNetNodes.resize(1, 8);
    //_params->np.innerNetNodes.push_back(8);
    _params->np.outputs = static_cast<int>(dataWrapper.outputSize());
    _params->np.trainingIterations = 20; // 20
    _params->np.validationIterations = 200;
    _params->np.testIterations = 500; //500
    _params->np.act = NeuralNet::ReLU;

    _params->fp.enableTopologyTraining = true;

    _params->fp.floors.accuracy = .05L;
    _params->fp.floors.precision = 0.05L;
    _params->fp.floors.sensitivity = 0.05L;
    _params->fp.floors.specificity = .05L;
    _params->fp.floors.f_score = 0.05L;
    _params->fp.mse_floor = 0;

    _params->fp.mse_weight = 1;
    _params->fp.weights.accuracy = 0.0L;
    _params->fp.weights.precision = 0.0L;
    _params->fp.weights.sensitivity = 0.0L;
    _params->fp.weights.specificity = 0.0L;
    _params->fp.weights.f_score = 0.0L;

    _params->alpha = 1.0;
    _params->beta = 1.0;
    _params->gamma = 1.0;

    _params->kFolds = 10;
    _params->inputHistorySize = 1;

    updateParameterGui();
    setGlobalBestSelectionBox();
}

void MainWindow::updateOutputRanges() {
    QStringList outputRanges = ui->outputRange_le->text().split(',');
    bool failFlag = false;

    if (outputRanges.length() > 0) {
        for (size_t i = 0; i < outputRanges.size(); i++) {
            if (outputRanges[i].length() == 0) {
                failFlag = true;
                break;
            }
        }
        if (!failFlag) {
            std::vector<real> ranges;
            for (int i = 0; i < outputRanges.size(); i++) {
                bool ok = false;
                real rangeVal = outputRanges[i].toDouble(&ok);
                if (ok) {
                    ranges.push_back(rangeVal);
                }
            }
            if (ranges.size() > 0) {
                OutageDataWrapper::setOutputRanges(ranges);

                OutageDataWrapper dataWrapper = (*_inputCache)[0];
                _params->np.outputs = static_cast<int>(dataWrapper.outputSize());
            } else {
                failFlag = true;
            }
        }
    } else {
        failFlag = true;
    }

    if (failFlag) {
        std::vector<real> ranges = OutageDataWrapper::outputRanges();
        QString newOutputDueToFailure;
        for (int i = 0; i < outputRanges.size(); i++) {
            newOutputDueToFailure.append(QString::number(static_cast<size_t>(ranges[i]), 10));
            if (i != outputRanges.size() -1) {
                newOutputDueToFailure.append(',');
            }
        }
        ui->outputRange_le->setText(newOutputDueToFailure);
    }
}

void MainWindow::applyParameterChanges() {
    qApp->processEvents();

    applyElementSkips();
    updateOutputRanges();


    _params->pp.population = static_cast<size_t>(ui->totalParticles_sb->value());
    _params->pp.neighbors = static_cast<size_t>(ui->totalNeighbors_sb->value());
    _params->pp.minEpochs = static_cast<size_t>(ui->minEpochs_sb->value());
    _params->pp.maxEpochs = static_cast<size_t>(ui->maxEpochs_sb->value());
    _params->pp.delta = static_cast<real>(ui->delta_dsb->value());
    _params->pp.windowSize = static_cast<size_t>(ui->window_sb->value());
    _params->pp.termMinEpochsFlag = ui->enableMinEpochs_cb->isChecked();
    _params->pp.termMaxEpochsFlag = ui->enableMaxEpochs_cb->isChecked();
    _params->pp.termDeltaFlag = static_cast<size_t>(ui->enableDelta_cb->isChecked());
    _params->pp.dt = static_cast<double>(ui->dt_dsb->value());
    _params->pp.tempDtFlag = ui->tempTrainingFlag_cb->isChecked();

    _params->np.trainingIterations = ui->trainingIterations_sb->value();
    _params->np.validationIterations = ui->validationIterations_sb->value();
    _params->np.testIterations = ui->testIt_sb->value();

    _params->fp.enableTopologyTraining = ui->topoTraining_cb->isChecked();

    _params->fp.mse_floor = static_cast<real>(ui->mse_floor_dsb->value());
    _params->fp.floors.accuracy = static_cast<real>(ui->acc_floor_dsb->value());
    _params->fp.floors.precision = static_cast<real>(ui->pre_floor_dsb->value());
    _params->fp.floors.sensitivity = static_cast<real>(ui->sen_floor_dsb->value());
    _params->fp.floors.specificity = static_cast<real>(ui->spe_floor_dsb->value());
    _params->fp.floors.f_score = static_cast<real>(ui->fscore_floor_dsb->value());

    _params->fp.mse_weight = static_cast<real>(ui->mse_weight_dsb->value());
    _params->fp.weights.accuracy = static_cast<real>(ui->acc_weight_dsb->value());
    _params->fp.weights.precision = static_cast<real>(ui->pre_weight_dsb->value());
    _params->fp.weights.sensitivity = static_cast<real>(ui->sen_weight_dsb->value());
    _params->fp.weights.specificity = static_cast<real>(ui->spe_weight_dsb->value());
    _params->fp.weights.f_score = static_cast<real>(ui->fscore_weight_dsb->value());

    _params->alpha = static_cast<real>(ui->alpha_dsb->value());
    _params->beta = static_cast<real>(ui->beta_dsb->value());
    _params->gamma = static_cast<real>(ui->gamma_dsb->value());

    _params->kFolds = static_cast<size_t> (ui->validationFolds_sb->value());
    _params->inputHistorySize = static_cast<size_t> (ui->inputHistorySize_sb->value());

    setNetTypeByIndex(ui->netType_cb->currentIndex());
    setActivationByCB();
}

void MainWindow::updateParameterGui() {
    ui->totalParticles_sb->setValue(static_cast<int>(_params->pp.population));
    ui->totalNeighbors_sb->setValue(static_cast<int>(_params->pp.neighbors));
    ui->minEpochs_sb->setValue(static_cast<int>(_params->pp.minEpochs));
    ui->maxEpochs_sb->setValue(static_cast<int>(_params->pp.maxEpochs));
    ui->window_sb->setValue(static_cast<int>(_params->pp.windowSize));
    ui->delta_dsb->setValue(static_cast<double>(_params->pp.delta));
    ui->enableMinEpochs_cb->setChecked(_params->pp.termMinEpochsFlag);
    ui->enableMaxEpochs_cb->setChecked(_params->pp.termMaxEpochsFlag);
    ui->enableDelta_cb->setChecked(_params->pp.termDeltaFlag);
    ui->dt_dsb->setValue(static_cast<double>(_params->pp.dt));
    ui->tempTrainingFlag_cb->setChecked(_params->pp.tempDtFlag);

    ui->trainingIterations_sb->setValue(_params->np.trainingIterations);
    ui->validationIterations_sb->setValue(_params->np.validationIterations);
    ui->testIt_sb->setValue(_params->np.testIterations);

    ui->topoTraining_cb->setChecked(_params->fp.enableTopologyTraining);

    ui->mse_floor_dsb->setValue(static_cast<double>(_params->fp.mse_floor));
    ui->acc_floor_dsb->setValue(static_cast<double>(_params->fp.floors.accuracy));
    ui->pre_floor_dsb->setValue(static_cast<double>(_params->fp.floors.precision));
    ui->sen_floor_dsb->setValue(static_cast<double>(_params->fp.floors.sensitivity));
    ui->spe_floor_dsb->setValue(static_cast<double>(_params->fp.floors.specificity));
    ui->fscore_floor_dsb->setValue(static_cast<double>(_params->fp.floors.f_score));

    ui->mse_weight_dsb->setValue(static_cast<double>(_params->fp.mse_weight));
    ui->acc_weight_dsb->setValue(static_cast<double>(_params->fp.weights.accuracy));
    ui->pre_weight_dsb->setValue(static_cast<double>(_params->fp.weights.precision));
    ui->sen_weight_dsb->setValue(static_cast<double>(_params->fp.weights.sensitivity));
    ui->spe_weight_dsb->setValue(static_cast<double>(_params->fp.weights.specificity));
    ui->fscore_weight_dsb->setValue(static_cast<double>(_params->fp.weights.f_score));

    ui->alpha_dsb->setValue(static_cast<double>(_params->alpha));
    ui->beta_dsb->setValue(static_cast<double>(_params->beta));
    ui->gamma_dsb->setValue(static_cast<double>(_params->gamma));

    ui->netType_cb->setCurrentIndex(getNetTypeCBIndex());

    ui->validationFolds_sb->setValue(_params->kFolds);
    ui->inputHistorySize_sb->setValue(_params->inputHistorySize);

    updateElementSkips();
    getGlobalBestSelectionFromBox();
    updateActivationCB();
}

void MainWindow::setGlobalBestSelectionBox() {
    switch(_params->showBestSelected) {
    case TrainingParameters::Recent_Global_Best:
        ui->globalBestSelection_cb->setCurrentIndex(0);
        break;
    case TrainingParameters::Selected_Global_Best:
        ui->globalBestSelection_cb->setCurrentIndex(1);
        break;
    case TrainingParameters::Sanity_Check_Best:
        ui->globalBestSelection_cb->setCurrentIndex(2);
        break;
    case TrainingParameters::Current_Net:
        ui->globalBestSelection_cb->setCurrentIndex(3);
        break;
    }
}

void MainWindow::getGlobalBestSelectionFromBox() {
    if (ui->globalBestSelection_cb->currentIndex() == 0) {
        _params->showBestSelected = TrainingParameters::Recent_Global_Best;
    } else if (ui->globalBestSelection_cb->currentIndex() == 1){
        _params->showBestSelected = TrainingParameters::Selected_Global_Best;
    } else if (ui->globalBestSelection_cb->currentIndex() == 2) {
        _params->showBestSelected = TrainingParameters::Sanity_Check_Best;
    } else {
        _params->showBestSelected = TrainingParameters::Current_Net;
    }
}

void MainWindow::updateFromGlobalBestComboBox() {
    getGlobalBestSelectionFromBox();
    if (_neuralPsoTrainer) {
        updateConfusionMatrix();
        updatePlot();
    }
}

void MainWindow::setActivationByCB() {
    _params->np.act = static_cast<NeuralNet::Activation>(ui->activation_cb->currentIndex());
}

void MainWindow::updateActivationCB() {
    qDebug( )<< "Update Activation << " << _params->np.act;
    ui->activation_cb->setCurrentIndex(_params->np.act);
}

void MainWindow::applyElementSkips() {
    _params->ep.year = ui->enDateYear_cb->isChecked();
    _params->ep.month = ui->enDateMonth_cb->isChecked();
    _params->ep.day = ui->enDateDay_cb->isChecked();
    _params->ep.temp_high = ui->enTempHigh_cb->isChecked();
    _params->ep.temp_avg = ui->enTempAvg_cb->isChecked();
    _params->ep.temp_low = ui->enTempLow_cb->isChecked();
    _params->ep.dew_high = ui->enDewHigh_cb->isChecked();
    _params->ep.dew_avg = ui->enDewAvg_cb->isChecked();
    _params->ep.dew_low = ui->enDewLow_cb->isChecked();
    _params->ep.humidity_high = ui->enHumidityHigh_cb->isChecked();
    _params->ep.humidity_avg = ui->enHumidityAvg_cb->isChecked();
    _params->ep.humidity_low = ui->enHumidityLow_cb->isChecked();
    _params->ep.press_high = ui->enPressHigh_cb->isChecked();
    _params->ep.press_avg = ui->enPressAvg_cb->isChecked();
    _params->ep.press_low = ui->enPressLow_cb->isChecked();
    _params->ep.visibility_high = ui->enVisibilityHigh_cb->isChecked();
    _params->ep.visibility_avg = ui->enVisibilityAvg_cb->isChecked();
    _params->ep.visibility_low = ui->enVisibilityLow_cb->isChecked();
    _params->ep.wind_high = ui->enWindHigh_cb->isChecked();
    _params->ep.wind_avg = ui->enWindAvg_cb->isChecked();
    _params->ep.wind_gust = ui->enWindGust_cb->isChecked();
    _params->ep.precipitation = ui->enPrec_cb->isChecked();
    _params->ep.fog = ui->enStormFog_cb->isChecked();
    _params->ep.rain = ui->enStormRain_cb->isChecked();
    _params->ep.snow = ui->enStormSnow_cb->isChecked();
    _params->ep.thunderstorm = ui->enStormTS_cb->isChecked();
    _params->ep.loa = ui->enLocLOA_cb->isChecked();
    _params->ep.latitude = ui->enLocLat_cb->isChecked();
    _params->ep.longitude = ui->enLocLong_cb->isChecked();
    _params->ep.population = ui->enPopulation_cb->isChecked();

    OutageDataWrapper dataWrapper = (*_inputCache)[0];
    _params->np.inputs = static_cast<int>( dataWrapper.inputSize(ui->inputHistorySize_sb->value()) );
    _params->np.outputs = static_cast<int>(dataWrapper.outputSize());
}

void MainWindow::updateElementSkips() {
    ui->enDateYear_cb->setChecked(_params->ep.year);
    ui->enDateMonth_cb->setChecked(_params->ep.month);
    ui->enDateDay_cb->setChecked(_params->ep.day);
    ui->enTempHigh_cb->setChecked(_params->ep.temp_high);
    ui->enTempAvg_cb->setChecked(_params->ep.temp_avg);
    ui->enTempLow_cb->setChecked(_params->ep.temp_low);
    ui->enDewHigh_cb->setChecked(_params->ep.dew_high);
    ui->enDewAvg_cb->setChecked(_params->ep.dew_avg);
    ui->enDewLow_cb->setChecked(_params->ep.dew_low);
    ui->enHumidityHigh_cb->setChecked(_params->ep.humidity_high);
    ui->enHumidityAvg_cb->setChecked(_params->ep.humidity_avg);
    ui->enHumidityLow_cb->setChecked(_params->ep.humidity_low);
    ui->enPressHigh_cb->setChecked(_params->ep.press_high);
    ui->enPressAvg_cb->setChecked(_params->ep.press_avg);
    ui->enPressLow_cb->setChecked(_params->ep.press_low);
    ui->enVisibilityHigh_cb->setChecked(_params->ep.visibility_high);
    ui->enVisibilityAvg_cb->setChecked(_params->ep.visibility_avg);
    ui->enVisibilityLow_cb->setChecked(_params->ep.visibility_low);
    ui->enWindHigh_cb->setChecked(_params->ep.wind_high);
    ui->enWindAvg_cb->setChecked(_params->ep.wind_avg);
    ui->enWindGust_cb->setChecked(_params->ep.wind_gust);
    ui->enPrec_cb->setChecked(_params->ep.precipitation);
    ui->enStormFog_cb->setChecked(_params->ep.fog);
    ui->enStormRain_cb->setChecked(_params->ep.rain);
    ui->enStormSnow_cb->setChecked(_params->ep.snow);
    ui->enStormTS_cb->setChecked(_params->ep.thunderstorm);
    ui->enLocLOA_cb->setChecked(_params->ep.loa);
    ui->enLocLat_cb->setChecked(_params->ep.latitude);
    ui->enLocLong_cb->setChecked(_params->ep.longitude);
    ui->enPopulation_cb->setChecked(_params->ep.population);
    //ui->dockWidget->update();
}

int MainWindow::getNetTypeCBIndex() {
    switch (_params->np.type) {
    case NeuralNet::Feedforward:
        return 0;
    case NeuralNet::Recurrent:
        return 1;
    }
    return 0;
}

void MainWindow::setNetTypeByIndex(const int & i) {
    switch (i) {
    case 0:
        _params->np.type = NeuralNet::Feedforward;
        break;
    case 1:
        _params->np.type = NeuralNet::Recurrent;
        break;
    default:
        _params->np.type = NeuralNet::Feedforward;
        break;
    }
}

void MainWindow::setInnerNetNodesFromGui() {
    InnerNetNodesInput * dialog = new InnerNetNodesInput(_params->np);
}

void MainWindow::setInputsForTrainedNetFromGui() {
    //_inputCache.a = ui->a_cb->isChecked();
    //_inputCache.b = ui->b_cb->isChecked();
}

void MainWindow::updateConfusionMatrix() {
    if (!_neuralPsoTrainer) return;

    ConfusionMatrix cm;

    switch(_params->showBestSelected) {
    case TrainingParameters::Recent_Global_Best:
        cm = _neuralPsoTrainer->getRecentGlobalBest().cm;
        break;
    case TrainingParameters::Selected_Global_Best:
        cm = _neuralPsoTrainer->getOverallBest().cm;
        //cm = _neuralPsoTrainer->getSelectedGlobalBest().cm;
        break;
    case TrainingParameters::Sanity_Check_Best:
        cm = _neuralPsoTrainer->sanityCheckGb().cm;
        break;
    case TrainingParameters::Current_Net:
        cm = _currentNetCm;
        break;
    default:
        return;
    }

    TestStatistics ts = cm.overallStats();
    TestStatistics::ClassificationError ce = cm.overallError();

    ui->confusionMatrix->updateConfusionMatrix(cm);

    double actPos = static_cast<double>(ts.tp() + ts.fn());
    double actNeg = static_cast<double>(ts.fp() + ts.tn());
    double prePos = static_cast<double>(ts.tp() + ts.fp());
    double preNeg = static_cast<double>(ts.tn() + ts.fn());
    double pop = ts.population();

    ui->acc_lbl->setText(QString::number(static_cast<double>(ce.accuracy)));
    ui->prec_lbl->setText(QString::number(static_cast<double>(ce.precision)));
    ui->sens_lbl->setText(QString::number(static_cast<double>(ce.sensitivity)));
    ui->spec_lbl->setText(QString::number(static_cast<double>(ce.specificity)));
    ui->fscore_lbl->setText(QString::number(static_cast<double>(ce.f_score)));
    ui->mse_lbl->setText(QString::number(static_cast<double>(ce.mse)));
/*
    ui->actPosNum_lbl->setText(QString::number(actPos));
    ui->actPosPerc_lbl->setText(QString::number(actPos / pop));
    ui->actNegNum_lbl->setText(QString::number(actNeg));
    ui->actNegPerc_lbl->setText(QString::number(actNeg / pop));
    ui->predPosNum_lbl->setText(QString::number(prePos));
    ui->predPosPerc_lbl->setText(QString::number(prePos / pop));
    ui->predNegNum_lbl->setText(QString::number(preNeg));
    ui->predNegPerc_lbl->setText(QString::number(preNeg / pop));

    ui->truePosNum_lbl->setText(QString::number(static_cast<double>(ts.tp())));
    ui->truePosPerc_lbl->setText(QString::number(static_cast<double>(ts.tp_norm())));
    ui->trueNegNum_lbl->setText(QString::number(static_cast<double>(ts.tn())));
    ui->trueNegPerc_lbl->setText(QString::number(static_cast<double>(ts.tn_norm())));
    ui->falsePosNum_lbl->setText(QString::number(static_cast<double>(ts.fp())));
    ui->falsePosPerc_lbl->setText(QString::number(static_cast<double>(ts.fp_norm())));
    ui->falseNegNum_lbl->setText(QString::number(static_cast<double>(ts.fn())));
    ui->falseNegPerc_lbl->setText(QString::number(static_cast<double>(ts.fn_norm())));
    */

//    ui->confusionMatrix_gb->adjustSize();
//    ui->plotLayout->update();
}

void MainWindow::on_actionParticle_Plotter_triggered() {
    if (_neuralPsoTrainer == nullptr) {
        qDebug() << "Trainer does not exist.";
        return;
    }

    if (_particlePlotWindow.length() > 0) {
        _particlePlotWindow.clear();
    }
    if (_neuralPsoTrainer->particles() != nullptr) {
        ParticlePlotWindow * newWindow = new ParticlePlotWindow(_neuralPsoTrainer->particles());
        newWindow->show();
        _particlePlotWindow.append(newWindow);
    } else {
        qDebug() << "Error, empty particle list.";
    }
}

void MainWindow::testTrainedNetWithInput() {
    if (_trainedNeuralNet != nullptr) {
        std::vector<real> newInput;
        //std::vector<real> curInput = _inputCache.inputize();
        std::vector<real> curInput;
//        curInput.push_back(OutageTrainer::convertInput(_inputCache.a));
//        curInput.push_back(OutageTrainer::convertInput(_inputCache.b));
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
            qDebug() << "Something broke: MainWindow::testTrainedNetWithInputs()";
            ui->testInput_output->setText("Failed to process.");
            return;
        }

        _trainedNeuralNet->resetAllNodes();
        for (size_t i = 0; i < newInput.size(); i++) {
            _trainedNeuralNet->loadInput(newInput[i], i);
        }
        std::vector<real> output = _trainedNeuralNet->process();

        if (output.size() != 1) {
            qDebug() << "What is this output?";
            ui->testInput_output->setText("UNK Output");
            return;
        } else {
//            bool result = ANDTrainer::convertOutput(output[0]);
//            if (result) {
//                ui->testInput_output->setText("Statement is True");
//            } else {
//                ui->testInput_output->setText("Statement is False");
//            }
        }
    }
}

void MainWindow::on_stop_btn_clicked() {
    _runningAutomatedTestProcedure = false;
    stopPso();
}

void MainWindow::stopPso() {
        _runPso = false;
        qDebug() << "Ending process.  Please wait. ";
    if (_neuralPsoTrainer) {
        _neuralPsoTrainer->stopValidation();
    }

    NeuralPso::interruptProcess();
}

void MainWindow::on_testFullSet_btn_clicked() {
    if (_neuralPsoTrainer && _trainedNeuralNet) {

        QTimer * fitnessPlotTimer = new QTimer(this);
        fitnessPlotTimer->setInterval(200);
        connect(fitnessPlotTimer, SIGNAL(timeout()), this, SLOT(updateFitnessPlot()));
        fitnessPlotTimer->start();

        QString weAreRunning("Please wait while we test everything.");
        setOutputLabel(weAreRunning);

        GlobalBestObject customNet;
        customNet.state = _trainedNeuralNet->state();
        _neuralPsoTrainer->setCustomNet(customNet);
        _neuralPsoTrainer->runFullTestOnCustomNet(_params->np);
        _currentNetCm = _neuralPsoTrainer->customNet().cm;
        updateConfusionMatrix();

        QString completionMsg("Complete!");
        setOutputLabel(completionMsg);

        disconnect(fitnessPlotTimer, SIGNAL(timeout()), this, SLOT(updateFitnessPlot()));
        fitnessPlotTimer->deleteLater();
    }
    if (!_trainedNeuralNet) {
        qDebug( )<< "Unable to run an empty net!  Load or set the current net.";
    }
}

void MainWindow::setOutputLabel(const QString & s) {
    ui->output_lbl->setText(s);
}

void MainWindow::updateFitnessPlot() {
    if (_neuralPsoTrainer) {
        ui->fitnessPlot->plotHistory(_neuralPsoTrainer->historyFromLastRun());
    }
}

/**
 * @brief MainWindow::runNeuralPso
 * @details Runs PSO trainer.
 */
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
  //!FIXME Not actually updating parameters.
  //applyParameterChanges();
  //tellParameters();

  if (_neuralPsoTrainer == nullptr || !_runOnce) {
      _runOnce = true;
      clearPSOState();  // Initializer for PSO training
  }

  //NeuralNet *net = _neuralPsoTrainer->neuralNet();
  //ui->neuralNetPlot->setEdges(&(net->getWeights()));

  // Train the net
  tryInjectGB();
  _neuralPsoTrainer->runTrainer();

  //_neuralPsoTrainer->testGb();
  //TestStatistics::ClassificationError ce;
  //_neuralPsoTrainer->classError(ce);

  //stopPso();
  _runPso = false;

  //_neuralPsoTrainer->fullTestState();updatePlot();

  enableParameterInput(true);

  QString completionMsg;
  completionMsg.append("Complete. ");
  completionMsg.append(QString::number(_neuralPsoTrainer->epochs(), 10));
  completionMsg.append(" epochs\t");
  completionMsg.append(QString::number(static_cast<double>(_runTimer.elapsed()) / 1000.0));
  completionMsg.append(" seconds");
  setOutputLabel(completionMsg);

  _gb = _neuralPsoTrainer->getGbState();

  disconnect(fitnessPlotTimer, SIGNAL(timeout()), this, SLOT(updateFitnessPlot()));
  fitnessPlotTimer->deleteLater();

  qApp->alert(this);
}

void MainWindow::on_testProcedure_btn_clicked() {
    // At this point, we're going to test all three hidden layer settings
    // for each activation function with topo-training.  We're going to run
    // this 10 times for each expriment to gauge a good agregation of performance.
    // (We'll get to adding which files to load later.)

    if (_runningAutomatedTestProcedure) {
        _runningAutomatedTestProcedure = false;
        return;
    } else {
        _runningAutomatedTestProcedure = true;
    }

    struct BestTopoData {
        std::vector<int> proposedTopology;
        NeuralNet::Activation activationFunction;
        GlobalBestObject result;
    };
    struct TrialData {
        BestTopoData singleHiddenLayer;
        BestTopoData doubleHiddenLayer;
        BestTopoData tripleHiddenLayer;
    };
    struct AvgExperimentData {
        std::vector<int> topo;
        bool topoTrainingEnabled;
        NeuralNet::Activation activationFunction;
        size_t history;
        size_t maxEpochs;
        real delta;
        size_t deltaWindow;
        ConfusionMatrix stats;
    };

    // Alright, you just ran 210 tests, right?  That might be a lie.  Depends
    // on how many activation functions we're still using.  So, now you
    // should have three good network topologies to run without topo-training.
    // We want to run all three of these networks on the same data


    TrainingParameters defaultParams = *_params;
    ExperimentFileParser expParser(this);
    if (!expParser.readFile(_params)) {
        qWarning( )<< "Unable to read experiment file. on_testProcedure_btn_clicked()";
        return;
    }
    if (expParser.getParamsList().size() == 0){
        qWarning( )<< "Experiment list is empty.";
        return;
    }

    std::vector<std::vector<BestTopoData>> resultingNets; // Final Nets after training
    std::vector<std::vector<BestTopoData>> resultingTopos;
    size_t trialsPerExp = expParser.experimentParams().trials_per_experiment;
    std::vector<AvgExperimentData> avgResults;

    time_t now = time(0);
    tm *gmtm = gmtime(&now);
    std::string fileName("testProcedureResults_");
    fileName += stringPut(gmtm->tm_year+1900);
    fileName += stringPut(gmtm->tm_mon);
    fileName += stringPut(gmtm->tm_mday);
    fileName += "_";
    fileName += stringPut(gmtm->tm_hour);
    fileName += stringPut(gmtm->tm_min);
    fileName += stringPut(gmtm->tm_sec);
    fileName += ".csv";

    //QFile outFile("testProcedureResults.txt");
    QDir curDirectory = qApp->applicationDirPath();
    QString finalFileName = curDirectory.filePath(fileName.c_str());
    QFile outFile(finalFileName);
    QTextStream oStream(&outFile);
    std::string outString;
    std::string loggerString;
    bool writeToFile = true;
    if (!outFile.open(QFile::WriteOnly | QFile::Truncate)) {
        qWarning() << "Unable to write test procedure results to file.";
        std::string headerString;
        headerString.append("Failed to open output file.\n");
        headerString.append("Output to: ");
        headerString.append(finalFileName.toStdString());
        headerString.append("\n");
        Logger::write(headerString);
        qDebug( )<< headerString.c_str();
        writeToFile = false;
    }
    else {
        std::string headerString;
        headerString.append("Output to: ");
        headerString.append(finalFileName.toStdString());
        headerString.append("\n");
        headerString.append("H1, H2, H3, TopoTraining, Activation, History, Max Epoc, Delta, Delta Window\n");
        QStringList expList = expParser.getExpList();
        for (int i = 0; i < expList.length(); i++){
            if (expList[i].length() == 0){continue;}
            headerString.append(expList[i].toStdString());
            headerString.append("\n");
        }
        headerString.append("Test Index, Trial, Hidden Layers, H1, H2, H3, Accuracy, F-Score, Percision, Sensivity, Specificity");
        loggerString = headerString;
        headerString.append("\n");
        oStream << headerString.c_str();
        Logger::write(loggerString);
        loggerString.clear();
    }

    /** Auto test section **/
    qDebug() << "Auto Section: ";

    /** Manual Test Section **/
    qDebug() << "Manual Section: ";
    if (writeToFile){
        outString.append("Manual Tests: ");
        outString.append(stringPut(expParser.getParamsList().size()));
        oStream << outString.c_str();
        Logger::write(outString);
        outString.clear();
    }
    std::vector<TrainingParameters> proposedNewTests;
    qDebug() << "Param list size: " << expParser.getParamsList().size();
    qDebug() << "Trials per Exp: " << trialsPerExp;
    qDebug( )<< "Total Tests: O(" << expParser.getParamsList().size() * 2.0 * trialsPerExp << ")";
    for (size_t i = 0; i < expParser.getParamsList().size(); i++) {

        if (!_runningAutomatedTestProcedure) {
            break;
        }

        qDebug() << "Test: " << i;
        AvgExperimentData thisExp;
        *_params = expParser.getParamsList()[i];
        updateParameterGui();
        std::vector<BestTopoData> resultsPerExp;
        std::vector<BestTopoData> topoTrainTrials;

        for (size_t k = 0; k < trialsPerExp; k++) {

            if (!_runningAutomatedTestProcedure) {
                break;
            }

            qDebug() << "Trial: " << k;

            clearPSOState();
            runNeuralPso();

            BestTopoData d;
            d.proposedTopology = _neuralPsoTrainer->neuralNet()->proposedTopology();
            d.activationFunction = _params->np.act;
            d.result = _neuralPsoTrainer->getOverallBest();
            if (_params->fp.enableTopologyTraining){
                // log each topo trained result
                topoTrainTrials.push_back(d);
            }
            // log each result
            resultsPerExp.push_back(d);

            if (writeToFile){
                outString.append(QString::number(i+1).toStdString());
                outString.append(",");
                outString.append(QString::number(k+1).toStdString());
                outString.append(",");
                outString.append(QString::number(d.proposedTopology.size()).toStdString());
                outString.append(",");
                for (size_t j = 0; j < d.proposedTopology.size(); j++) {
                    outString.append(QString::number(d.proposedTopology[j]).toStdString());
                    outString.append(",");
                }
                if (d.proposedTopology.size() < 3){
                    for (size_t k = d.proposedTopology.size(); k < 3; k++){
                        outString.append(QString::number(0).toStdString());
                        outString.append(",");
                    }
                }
                outString.append(QString::number(d.result.cm.overallError().accuracy).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().f_score).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().precision).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().sensitivity).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().specificity).toStdString());
                loggerString = outString;
                outString.append("\n");
                oStream << outString.c_str();
                Logger::write(loggerString);
                outString.clear();
                loggerString.clear();
            }

        }

        // Copy into the final result vector
        resultingNets.push_back(resultsPerExp);
        // Save the average result object
        thisExp.delta = _params->pp.delta;
        thisExp.deltaWindow = _params->pp.windowSize;
        thisExp.activationFunction = _params->np.act;
        thisExp.history = _params->inputHistorySize;
        thisExp.maxEpochs = _params->pp.maxEpochs;
        thisExp.topoTrainingEnabled = _params->fp.enableTopologyTraining;
        thisExp.topo = _params->np.innerNetNodes;
        avgResults.push_back(thisExp);
        // Copy into topo vector
        resultingTopos.push_back(topoTrainTrials);

        // Average results and print

        if (i < resultingNets.size()) {
            std::vector<ConfusionMatrix> trialStats;
            for (size_t j = 0; j < resultingNets[i].size(); j++){
                trialStats.push_back(resultingNets[i][j].result.cm);
            }
            avgResults[i].stats = ConfusionMatrix::average(trialStats);
            if (writeToFile){
                outString.append(QString::number(i+1).toStdString());
                outString.append(",");
                outString.append(QString::number(0).toStdString());
                outString.append(",");
                outString.append(QString::number(avgResults[i].topo.size()).toStdString());
                outString.append(",");
                for (size_t j = 0; j < avgResults[i].topo.size(); j++) {
                    outString.append(QString::number(avgResults[i].topo[j]).toStdString());
                    outString.append(",");
                }
                if (avgResults[i].topo.size() < 3){
                    for (size_t k = avgResults[i].topo.size(); k < 3; k++){
                        outString.append(QString::number(0).toStdString());
                        outString.append(",");
                    }
                }
                outString.append(QString::number(avgResults[i].stats.overallError().accuracy).toStdString());
                outString.append(",");
                outString.append(QString::number(avgResults[i].stats.overallError().f_score).toStdString());
                outString.append(",");
                outString.append(QString::number(avgResults[i].stats.overallError().precision).toStdString());

                outString.append(",");
                outString.append(QString::number(avgResults[i].stats.overallError().sensitivity).toStdString());
                outString.append(",");
                outString.append(QString::number(avgResults[i].stats.overallError().specificity).toStdString());
                loggerString = outString;
                outString.append("\n");
                oStream << outString.c_str();
                Logger::write(loggerString);
                outString.clear();
                loggerString.clear();
            }
        }
        else{
            std::string errorString;
            errorString.append("Failed to append resultingNets on i=");
            errorString.append(stringPut(i));
            errorString.append(".\n");
            Logger::write(errorString);
        }

    }

    // Make new runs from proposed topologies
    for (size_t i = 0; i < resultingTopos.size(); i++){

        std::vector<int> proposedTopo;

        if (resultingTopos[i].size() != 0) {
            size_t kLim = resultingTopos[i][0].proposedTopology.size();
            for (size_t k = 0; k < kLim; k++){
                // Collect all layer counts
                std::vector<int> numNodes;
                size_t jLim = resultingTopos[i].size();
                for(size_t j = 0; j < jLim; j++){
                    numNodes.push_back(resultingTopos[i][j].proposedTopology[k]);
                }
                // Get mode of each layer
                int modeVal = mode(numNodes);
                proposedTopo.push_back(modeVal);
            }

            TrainingParameters newRun = *_params;
            newRun.np.innerNetNodes = proposedTopo;
            newRun.fp.enableTopologyTraining = false;

            // Check that proposed topo is not already in the list.
            bool notDuplicate = true;
            for (size_t l = 0; l < proposedNewTests.size(); l++){
                bool sameVect = true;
                for (size_t m = 0; m < proposedNewTests[l].np.innerNetNodes.size(); m++){
                    if (proposedNewTests[l].np.innerNetNodes[m] != proposedTopo[m]){
                        sameVect = false;
                    }
                }
                if (sameVect == true){
                    notDuplicate = false;
                }
            }
            if(notDuplicate){
                proposedNewTests.push_back(newRun);
            }
        } else {
            std::string errorString;
            errorString.append("Resulting Topos size at i=");
            errorString.append(stringPut(i));
            errorString.append(" is 0.\n");
            Logger::write(errorString);
        }
    }

    /** Proposed New Tests Section **/
    qDebug() << "Proposed Topos Section: ";
    if (writeToFile){
        std::string outString;
        outString.append("Proposed Tests: ");
        outString.append(stringPut(proposedNewTests.size()));
        oStream << outString.c_str();
        Logger::write(outString);
        outString.clear();
    }
    const size_t startIdx = resultingTopos.size();
    for (size_t i = 0; i < proposedNewTests.size(); i++) {
        if (!_runningAutomatedTestProcedure) {
            break;
        }

        qDebug() << "Test: " << i;
        AvgExperimentData thisExp;
        *_params = proposedNewTests[i];
        updateParameterGui();
        std::vector<BestTopoData> resultsPerTopoExp;

        thisExp.delta = _params->pp.delta;
        thisExp.deltaWindow = _params->pp.windowSize;
        thisExp.activationFunction = _params->np.act;
        thisExp.history = _params->inputHistorySize;
        thisExp.maxEpochs = _params->pp.maxEpochs;
        thisExp.topoTrainingEnabled = _params->fp.enableTopologyTraining;
        thisExp.topo = _params->np.innerNetNodes;
        avgResults.push_back(thisExp);

        for (size_t k = 0; k < trialsPerExp; k++) {

            if (!_runningAutomatedTestProcedure) {
                break;
            }

            qDebug() << "Trial: " << k;

            clearPSOState();
            runNeuralPso();

            BestTopoData d;
            d.proposedTopology = _neuralPsoTrainer->neuralNet()->proposedTopology();
            d.activationFunction = _params->np.act;
            d.result = _neuralPsoTrainer->getOverallBest();
            // log each result
            resultsPerTopoExp.push_back(d);

            if (writeToFile){
                outString.append(QString::number(startIdx+i+1).toStdString());
                outString.append(",");
                outString.append(QString::number(k+1).toStdString());
                outString.append(",");
                outString.append(QString::number(d.proposedTopology.size()).toStdString());
                outString.append(",");
                for (size_t j = 0; j < d.proposedTopology.size(); j++) {
                    outString.append(QString::number(d.proposedTopology[j]).toStdString());
                    outString.append(",");
                }
                if (d.proposedTopology.size() < 3){
                    for (size_t k = d.proposedTopology.size(); k < 3; k++){
                        outString.append(QString::number(0).toStdString());
                        outString.append(",");
                    }
                }
                outString.append(QString::number(d.result.cm.overallError().accuracy).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().f_score).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().precision).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().sensitivity).toStdString());
                outString.append(",");
                outString.append(QString::number(d.result.cm.overallError().specificity).toStdString());
                loggerString = outString;
                outString.append("\n");
                oStream << outString.c_str();
                Logger::write(loggerString);
                outString.clear();
                loggerString.clear();
            }
        }

        // Copy into the final result vector
        resultingNets.push_back(resultsPerTopoExp);

        // Average results and print
        std::vector<ConfusionMatrix> trialStats;
        for (size_t j = 0; j < trialsPerExp; j++){
            trialStats.push_back(resultingNets[startIdx+i][j].result.cm);
        }
        avgResults[startIdx+i].stats = ConfusionMatrix::average(trialStats);
        if (writeToFile){
            outString.append(QString::number(startIdx+i+1).toStdString());
            outString.append(",");
            outString.append(QString::number(0).toStdString());
            outString.append(",");
            outString.append(QString::number(avgResults[startIdx+i].topo.size()).toStdString());
            outString.append(",");
            for (size_t j = 0; j < avgResults[startIdx+i].topo.size(); j++) {
                outString.append(QString::number(avgResults[startIdx+i].topo[j]).toStdString());
                outString.append(",");
            }
            if (avgResults[startIdx+i].topo.size() < 3){
                for (size_t k = avgResults[startIdx+i].topo.size(); k < 3; k++){
                    outString.append(QString::number(0).toStdString());
                    outString.append(",");
                }
            }
            outString.append(QString::number(avgResults[startIdx+i].stats.overallError().accuracy).toStdString());
            outString.append(",");
            outString.append(QString::number(avgResults[startIdx+i].stats.overallError().f_score).toStdString());
            outString.append(",");
            outString.append(QString::number(avgResults[startIdx+i].stats.overallError().precision).toStdString());
            outString.append(",");
            outString.append(QString::number(avgResults[startIdx+i].stats.overallError().sensitivity).toStdString());
            outString.append(",");
            outString.append(QString::number(avgResults[startIdx+i].stats.overallError().specificity).toStdString());
            loggerString = outString;
            outString.append("\n");
            oStream << outString.c_str();
            Logger::write(loggerString);
            outString.clear();
            loggerString.clear();
        }
    }

    // Close the file
    if (writeToFile){
        outFile.close();
    }
}

void MainWindow::on_testBaseCase_btn_clicked() {
    _params->enableBaseCase = true;
    clearPSOState();
    runNeuralPso();
    _params->enableBaseCase = false;
}

void MainWindow::on_resetAndRun_btn_clicked() {
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setText("Will run after clearing state.\nClear state and run?");
    msgBox->addButton(QMessageBox::Yes);
    msgBox->addButton(QMessageBox::Cancel);
    int choice = msgBox->exec();

    switch(choice) {
    case QMessageBox::Yes:
        applyElementSkips();
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
        applyElementSkips();
        clearPSOState();
        break;
    default:
        return;
    }
}

void MainWindow::clearPSOState() {
    applyParameterChanges();
    _neuralPsoTrainer = std::make_unique<OutageTrainer>(_params, _inputCache);
    //_neuralPsoTrainer->build(_inputData, _labelsData);
    _neuralPsoTrainer->setFunctionMsg("Outage Data");
    qDebug() << "New File: " << _neuralPsoTrainer->inputCache()->cacheParams().inputFileName;
    qDebug() << "Items in file: " << _neuralPsoTrainer->inputCache()->length();
    qDebug() << "Total Inputs: " << _neuralPsoTrainer->neuralNet()->inputs().size();
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
    qApp->processEvents();
    updateFileLabel();
    if (_neuralPsoTrainer != nullptr) {
        NeuralNet::State * state = nullptr;

        getGlobalBestSelectionFromBox();
        switch(_params->showBestSelected) {
        case TrainingParameters::Recent_Global_Best:
            state = &(_neuralPsoTrainer->getRecentGlobalBest().state);
            break;
        case TrainingParameters::Selected_Global_Best:
            state = &(_neuralPsoTrainer->getOverallBest().state);
            //state = &(_neuralPsoTrainer->getSelectedGlobalBest().state);
            break;
        case TrainingParameters::Sanity_Check_Best:
            state = &(_neuralPsoTrainer->sanityCheckGb().state);
            break;
        case TrainingParameters::Current_Net:
            if (_trainedNeuralNet) {
                state = &(_trainedNeuralNet->state());
            }
            break;
        }

        ui->neuralNetPlot->setState(state, _params->np.type);
        updateConfusionMatrix();

        if (_runPso) {
            if (_neuralPsoTrainer->checkTermProcess()) {
                stopPso();
            } else {

                QString completionMsg;
                completionMsg.append("Training running. ");
                completionMsg.append(QString::number(_neuralPsoTrainer->epochs(), 10));
                completionMsg.append(" epochs\t");
                completionMsg.append(QString::number(
                    _neuralPsoTrainer->epochs() * static_cast<size_t>(_params->np.trainingIterations),
                    10));
                completionMsg.append(" total iterations");
                completionMsg.append("\tFold: " );
                completionMsg.append(QString::number(_neuralPsoTrainer->currentFold() + 1));
                setOutputLabel(completionMsg);
            }
        }
    }
}

void MainWindow::updateFitnessPlotWindowSize() {
    ui->fitnessPlot->setWindowSize(ui->fitnessPlotWindow_sb->value());
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
    outputString += stringPut(_params->np.inputs);
    outputString += "\nInner Nets: ";
    outputString += stringPut(_params->np.innerNetNodes.size());
    outputString += "\n";
    for (size_t i = 0; i < _params->np.innerNetNodes.size(); i++) {
        outputString += " - ";
        outputString += stringPut(_params->np.innerNetNodes[i]);
        outputString += "\n";
        //cout << " - " << nParams.innerNetNodes[i] << endl;
    }
    outputString += "Tests per train(min): ";
    outputString += stringPut(_params->np.testIterations);
    outputString += "\n";
    outputString += "Particles: ";
    outputString += stringPut(_params->pp.population);
    outputString += "\nNeighbors: ";
    outputString += stringPut(_params->pp.neighbors);
    outputString += "\n";
    outputString += "Minimum Particle Iterations: ";
    outputString += stringPut(_params->pp.minEpochs);
    outputString += "\n";
    outputString += "Maximum Particle Iterations: ";
    outputString += stringPut(_params->pp.maxEpochs);
    outputString += "\n";

    //cout << "Tests per train(min): " << nParams.testIterations << endl;
    //cout << "Particles: " << pParams.particles << "\nNeighbors: " << pParams.neighbors << endl;
    //cout << "Minimum Particle Iterations: " << pParams.iterations << endl;
    Logger::write(outputString);
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

void MainWindow::resizeEvent(QResizeEvent * event) {
    qDebug() << "MainWindow Size: " << this->size();
    qDebug() << "MainWindow Pos: " << this->pos();
    //qDebug() << "Dock Position: " << ui->dockWidget->pos();
    //QPoint relativePosition = pos() - ui->dockWidget->pos();
    //relativePosition.rx() -= size().rwidth();
    //qDebug() << "Dock Relative Position: " << relativePosition;
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

