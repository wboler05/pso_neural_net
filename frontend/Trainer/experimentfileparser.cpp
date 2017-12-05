#include "frontend/Trainer/experimentfileparser.h"
using namespace NeuralPsoStream;

ExperimentFileParser::ExperimentFileParser(QMainWindow * parent) : _parent(parent)
{
    if (_parent == nullptr) {
        qWarning() << "Failed to build ExperimentFileParser: parent cannot be null";
        exit(1);
    }
}

bool ExperimentFileParser::readFile(const std::shared_ptr<TrainingParameters> & defaultParams) {

    //_defaultParams = *defaultParams;

    QFileInfo fileInfo(_fileLocation);
    QDir currentDirectory = fileInfo.absolutePath();
    QString inputFileString = QFileDialog::getOpenFileName(_parent,
            "Test Procedure", currentDirectory.absolutePath(), "Experiment File (*.exp);;All Files (*.*)");
    QFile inFile(inputFileString);
    if (!inFile.open(QFile::ReadOnly)) {
        qWarning() << "Unable to open test procedure file. on_testProcedure_btn_clicked()";
        return false;
    }

    _fileLocation = inputFileString;
    QTextStream * inputStream = new QTextStream(&inFile);
    std::string fullFile = inputStream->readAll().toStdString();

    if (!readOptions(fullFile)) {
        qDebug() << "Failed to read options: ExperimentFileParser::readFile()";
        inFile.close();
        return false;
    }
    if (!readAutoTest(fullFile)) {
        qDebug() << "Failed to read auto test: ExperimentFileParser::readFile()";
        inFile.close();
        return false;
    }
    if (!readExperiments(fullFile)) {
        qDebug() << "Failed to read experiments: ExperimentFileParser::readFile()";
        inFile.close();
        return false;
    }

    inFile.close();
    return true;
}

bool ExperimentFileParser::readOptions(const std::string & fullFile) {
    if (fullFile.empty()) {
        qDebug() << "Failed to read preamble: ExperimentFileParser";
        return false;
    }

    std::string optionsString = NeuralPsoStream::subStringByToken(fullFile, "options");
    cleanInputString(optionsString);

    qDebug() << "Options: \n" << optionsString.c_str();

    std::string autoTrainString = NeuralPsoStream::subStringByToken(optionsString, "auto_train");
    if (autoTrainString == "true") {
        _experimentParams.autoTrain = true;
    } else {
        _experimentParams.autoTrain = false;
    }

    std::string trialsPerExperimentString = NeuralPsoStream::subStringByToken(optionsString, "trials_per_experiment");
    bool ok;
    int val = QString(trialsPerExperimentString.c_str()).toInt(&ok);
    if (ok && val > 0) {
        _experimentParams.trials_per_experiment = val;
    } else {
        _experimentParams.trials_per_experiment = 1;
    }

    return true;
}

bool ExperimentFileParser::readAutoTest(const std::string & fullFile) {
    if (fullFile.empty()) {
        qDebug() << "Failed to read auto test: ExperimentFileParser";
        return false;
    }

    std::string autoTestString = NeuralPsoStream::subStringByToken(fullFile, "autotest");
    cleanInputString(autoTestString);

    qDebug() << "Auto Test: \n" << autoTestString.c_str();

    std::string testSingleInputsString = NeuralPsoStream::subStringByToken(autoTestString, "test_single_inputs");
    if (testSingleInputsString == "true") {
        _experimentParams.test_single_inputs = true;
    } else {
        _experimentParams.test_single_inputs = false;
    }

    std::string testCombinationInputsString = NeuralPsoStream::subStringByToken(autoTestString, "test_combination_inputs");
    if (testCombinationInputsString == "true") {
        _experimentParams.test_combination_inputs = true;
    } else {
        _experimentParams.test_combination_inputs = false;
    }

    std::string testActivationFunctionString = NeuralPsoStream::subStringByToken(autoTestString, "test_activation_functions");
    if (testActivationFunctionString == "true") {
        _experimentParams.test_activation_functions = true;
    } else {
        _experimentParams.test_activation_functions = false;
    }

    return true;
}

bool ExperimentFileParser::readExperiments(const std::string & fullFile) {
    if (fullFile.empty()) {
        qDebug( )<< "Failed to read experiments: ExperimentFileParser";
        return false;
    }

    qDebug() << fullFile.c_str();

    std::string experimentString = NeuralPsoStream::subStringByToken(fullFile, "experiments");

    qDebug( )<< "Experiments: \n" << experimentString.c_str();

    std::string badChars(" \t\r");
    cleanInputString(experimentString, badChars);
    QStringList experimentList = QString(experimentString.c_str()).split('\n');

    bool valid = false;
    _paramsList.clear();
    for (int i = 0; i < experimentList.length(); i++) {
        TrainingParameters tp = parseExperimentLine(experimentList[i], valid);
        if (valid) {
            _paramsList.push_back(tp);
        }
    }

    return true;
}

TrainingParameters ExperimentFileParser::parseExperimentLine(const QString & line, bool & valid) {
    TrainingParameters newParams = _defaultParams;
    bool ok = false;

    QStringList parameterStrings = line.split(',');
    if (parameterStrings.length() < 37) {
        valid = false;
        return newParams;
    }

    // Topology
    std::vector<int> topology;
    for (size_t i = 0; i < 3; i++) {
        int val = parameterStrings[i].toInt(&ok);
        if (ok && val > 0) {
            topology.push_back(val);
        } else {
            break;
        }
    }
    if (topology.size() > 0) {
        newParams.np.innerNetNodes = topology;
    }

    // Topo Training
    {
        QString topoTraining = parameterStrings[3];
        if (topoTraining == "true") {
            newParams.fp.enableTopologyTraining = true;
        } else if (topoTraining == "false") {
            newParams.fp.enableTopologyTraining = false;
        }
    }

    // Activation
    {
        QString activationFunction = parameterStrings[4];
        newParams.np.act = parseActivationFunction(activationFunction);
    }

    // Max Epochs
    {
        QString maxEpochs = parameterStrings[5];
        int val = maxEpochs.toInt(&ok);
        if (ok && val > 0) {
            newParams.pp.maxEpochs = val;
        }
    }

    // Delta
    {
        QString delta = parameterStrings[6];
        double val = delta.toDouble(&ok);
        if (ok && val >= 0) {
            newParams.pp.delta = val;
        }
    }

    // Delta Window
    {
        QString deltaWindow = parameterStrings[7];
        int val = deltaWindow.toInt(&ok);
        if (ok && val > 0) {
            newParams.pp.windowSize = val;
        }
    }

    valid = true;
    return newParams;
}

NeuralNet::Activation ExperimentFileParser::parseActivationFunction(const QString & s) {
    if (s.length() == 0) {
        return _defaultParams.np.act;
    }
    if (s == "ReLU") {
        return NeuralNet::ReLU;
    } else if (s == "Sin") {
        return NeuralNet::Sin;
    } else if (s == "Sigmoid") {
        return NeuralNet::Sigmoid;
    } else if (s == "HypTan") {
        return NeuralNet::HypTan;
    } else if (s == "Gaussian") {
        return NeuralNet::Gaussian;
    } else if (s == "Sinc") {
        return NeuralNet::Sinc;
    } else if (s == "Step") {
        return NeuralNet::Step;
    } else {
        return _defaultParams.np.act;
    }
}
