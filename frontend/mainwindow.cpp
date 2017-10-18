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
    headerString += "   *                                     *\n";
    headerString += "   *         Course: ECE 57000           *\n";
    headerString += "   *      Created: March 13, 2017        *\n";
    headerString += "   ***************************************\n";
    Logger::write(headerString);


    //cl::Context _context;

    //initializeCL(_cpuDevices, _gpuDevices, _allDevices);

    // Connect signals to slots
    connect(ui->run_btn, SIGNAL(clicked(bool)), this, SLOT(runNeuralPso()));
    connect(ui->stop_btn, SIGNAL(clicked(bool)), this, SLOT(stopPso()));
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

    QTimer * updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updatePlot()));
    updateTimer->start(500);

    ui->OutageInputBox->setVisible(false);

    // Init before running things
    initializeData();

    QSize windowSize(1500, 850);
    QPoint dockOffset(100, 75);
    QPoint dockPos = pos() + dockOffset;
    dockPos.rx() += size().rwidth();
    ui->dockWidget->move(dockPos);

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
    QDir curDir(qApp->applicationDirPath());
    QString filename = QFileDialog::getOpenFileName(this, "Open input file:", curDir.absolutePath(), "CSV (*.csv)");
    if (filename != "") {
        if (!_inputCache->reloadCache(filename)) {
            QMessageBox fileLoadFailed;
            fileLoadFailed.setText("Error: could not load cache from file!");
            fileLoadFailed.exec();
        }
    }
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

void MainWindow::setCurrentNet() {
    if (_neuralPsoTrainer != nullptr) {
        _trainedNeuralNet = _neuralPsoTrainer->buildNeuralNetFromGb();
        //qDebug() << "Test me baby: " << _trainedNeuralNet->getWeights().size();
        qDebug() << "Updated new neural net.";
    }
}

void MainWindow::initializeData() {
    if (!_params) {
        _params = std::make_shared<TrainingParameters>();
    }

    bool iniFileLoaded = loadIniFile();

    setParameterDefaults();
    setInputsForTrainedNetFromGui();

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
    }

    if (_inputCache->validFile()) {

        for (size_t i = 0; i < _inputCache->totalInputItemsInFile(); i++) {
            OutageDataWrapper index = (*_inputCache)[i];
            qDebug() <<
                "(" << i << "): date(" <<
                index._date.day() << "/" <<
                index._date.month() << "/" <<
                index._date.year() << ") Temp(" <<
                static_cast<double>(index._temp.hi()) << "," <<
                static_cast<double>(index._temp.avg()) << "," <<
                static_cast<double>(index._temp.lo()) << ")\tAffected Customers: " <<
                index._affectedCustomers << "\tOutage: " << OutageDataWrapper::bool2Double(index._outage) <<
                "\tLOA: " << index._loa <<
                "\tLat: " << index._latlong.latitude() << "\tLong: " << index._latlong.longitude();
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
    _params->cp.headerSize = 2;

    initializeCache();

    _params->pp.population = 150; // 50
    _params->pp.neighbors = 30; // 10
    _params->pp.minEpochs = 50;
    _params->pp.maxEpochs = 1000;
    _params->pp.delta = 5E-8L;
    _params->pp.vDelta = 5E-200L;
    _params->pp.termIterationFlag = false;
    _params->pp.termDeltaFlag = true;
    _params->pp.windowSize = 1500;

    /*
    NeuralNetParameters nParams;
    nParams.inputs = trainingImages[0].size() * trainingImages[0][0].size();
    nParams.innerNets = 1;
    nParams.innerNetNodes.push_back(100);
    //nParams.innerNetNodes.push_back(50);
    nParams.outputs = 10;
    */

    OutageDataWrapper dataWrapper = (*_inputCache)[0];

    _params->np.inputs = static_cast<int>(
                dataWrapper.inputize(_params->ep.inputSkips()).size());
    _params->np.innerNetNodes.clear();
    _params->np.innerNetNodes.push_back(16);
    _params->np.innerNetNodes.push_back(16);
    _params->np.innerNetNodes.push_back(16);
    _params->np.innerNets = static_cast<int>(_params->np.innerNetNodes.size());
    _params->np.outputs = static_cast<int>(
                dataWrapper.outputize(_params->ep.outputSkips()).size());
    _params->np.trainingIterations = 200;
    _params->np.validationIterations = 200;
    _params->np.testIterations = 500; //500

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

    _params->alpha = 160000.0;
    _params->beta = 1.0;
    _params->gamma = 1.0;

    updateParameterGui();
    setGlobalBestSelectionBox();
}

void MainWindow::applyParameterChanges() {
    _params->pp.population = static_cast<size_t>(ui->totalParticles_sb->value());
    _params->pp.neighbors = static_cast<size_t>(ui->totalNeighbors_sb->value());
    _params->pp.minEpochs = static_cast<size_t>(ui->minEpochs_sb->value());
    _params->pp.maxEpochs = static_cast<size_t>(ui->maxEpochs_sb->value());
    _params->pp.delta = static_cast<real>(ui->delta_dsb->value());
    _params->pp.windowSize = static_cast<size_t>(ui->window_sb->value());
    _params->pp.termIterationFlag = static_cast<size_t>(ui->enableIteration_cb->isChecked());
    _params->pp.termDeltaFlag = static_cast<size_t>(ui->enableDelta_cb->isChecked());

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

    setNetTypeByIndex(ui->netType_cb->currentIndex());

    applyElementSkips();
}

void MainWindow::updateParameterGui() {
    ui->totalParticles_sb->setValue(static_cast<int>(_params->pp.population));
    ui->totalNeighbors_sb->setValue(static_cast<int>(_params->pp.neighbors));
    ui->minEpochs_sb->setValue(static_cast<int>(_params->pp.minEpochs));
    ui->maxEpochs_sb->setValue(static_cast<int>(_params->pp.maxEpochs));
    ui->window_sb->setValue(static_cast<int>(_params->pp.windowSize));
    ui->delta_dsb->setValue(static_cast<double>(_params->pp.delta));
    ui->enableIteration_cb->setChecked(_params->pp.termIterationFlag);
    ui->enableDelta_cb->setChecked(_params->pp.termDeltaFlag);

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

    updateElementSkips();
    getGlobalBestSelectionFromBox();
}

void MainWindow::setGlobalBestSelectionBox() {
    if (_params->showBestSelected) {
        ui->globalBestSelection_cb->setCurrentIndex(1);
    } else {
        ui->globalBestSelection_cb->setCurrentIndex(0);
    }
}

void MainWindow::getGlobalBestSelectionFromBox() {
    if (ui->globalBestSelection_cb->currentIndex() == 0) {
        _params->showBestSelected = false;
    } else {
        _params->showBestSelected = true;
    }
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
    _params->ep.outage = ui->enOutputOutage_cb->isChecked();
    _params->ep.affected_people = ui->enOutputAC_cb->isChecked();

    OutageDataWrapper dataWrapper = (*_inputCache)[0];
    _params->np.inputs = static_cast<int>(
                dataWrapper.inputize(_params->ep.inputSkips()).size());
    _params->np.outputs = static_cast<int>(
                dataWrapper.outputize(_params->ep.outputSkips()).size());
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
    ui->enOutputOutage_cb->setChecked(_params->ep.outage);
    ui->enOutputAC_cb->setChecked(_params->ep.affected_people);
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

bool MainWindow::showBestSelected() {
    if (ui->globalBestSelection_cb->currentIndex() == 0) {
        return false;
    } else {
        return true;
    }
}

void MainWindow::updateConfusionMatrix() {

    TestStatistics ts;
    TestStatistics::ClassificationError ce;

    if (showBestSelected()) {
        SelectedGlobalBest selGb = _neuralPsoTrainer->getSelectedGlobalBest();
        ts = selGb.testStats;
        ce = selGb.ce;
    } else {
        SelectedGlobalBest recGb = _neuralPsoTrainer->getRecentGlobalBest();
        ts = recGb.testStats;
        ce = recGb.ce;
    }

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
            qDebug() << "Something broke";
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
  //!FIXME Not actually updaing parameters.
  applyParameterChanges();
  tellParameters();

  if (_neuralPsoTrainer == nullptr) {
      clearPSOState();  // Initializer for PSO training
  }

  //NeuralNet *net = _neuralPsoTrainer->neuralNet();
  //ui->neuralNetPlot->setEdges(&(net->getWeights()));

  // Train the net
  tryInjectGB();
  _neuralPsoTrainer->runTrainer();

  _neuralPsoTrainer->testGB();
  //TestStatistics::ClassificationError ce;
  //_neuralPsoTrainer->classError(ce);

  stopPso();

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
    _neuralPsoTrainer = std::make_unique<OutageTrainer>(_params, _inputCache);
    //_neuralPsoTrainer->build(_inputData, _labelsData);
    _neuralPsoTrainer->setFunctionMsg("Outage Data");
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
        NeuralNet::State * state = nullptr;
        if (ui->globalBestSelection_cb->currentIndex() == 0) {
            state = &(_neuralPsoTrainer->getRecentGlobalBest().state);
        } else {
            state = &(_neuralPsoTrainer->getSelectedGlobalBest().state);
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
    outputString += stringPut(_params->np.innerNets);
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
    qDebug() << "Dock Position: " << ui->dockWidget->pos();
    QPoint relativePosition = pos() - ui->dockWidget->pos();
    relativePosition.rx() -= size().rwidth();
    qDebug() << "Dock Relative Position: " << relativePosition;
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

