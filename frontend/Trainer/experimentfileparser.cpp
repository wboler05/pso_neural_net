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

    _defaultParams = *defaultParams;

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
//    if (!readAutoTest(fullFile)) {
//        qDebug() << "Failed to read auto test: ExperimentFileParser::readFile()";
//        inFile.close();
//        return false;
//    }
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

    std::string experimentString = NeuralPsoStream::subStringByToken(fullFile, "experiments");

    qDebug( )<< "Experiments: \n" << experimentString.c_str();

    std::string badChars(" \t\r");
    cleanInputString(experimentString, badChars);
    QStringList experimentList = QString(experimentString.c_str()).split('\n');

    bool valid = false;
    _paramsList.clear();
    for (int i = 0; i < experimentList.length(); i++) {
        std::vector<TrainingParameters> tp = parseExperimentLine(experimentList[i], valid);
        if (valid) {
            _paramsList.insert(std::end(_paramsList), std::begin(tp), std::end(tp));
        }
    }

    return true;
}

std::vector<TrainingParameters> ExperimentFileParser::parseExperimentLine(const QString & line, bool & valid) {
#define NUMACTIVATIONFUNCTIONS  7
#define         h1              0
#define         h2              1
#define         h3              2
#define         topoTraining    3
#define         activation      4
#define         history         5
#define         maxEpoc         6
#define         del             7
#define         deltaWindow     8
#define         year            9
#define         month           10
#define         day             11
#define         hTemp           12
#define         aTemp           13
#define         lTemp           14
#define         hHumidity       15
#define         aHumidity       16
#define         lHumidity       17
#define         hDP             18
#define         aDP             19
#define         lDP             20
#define         prec            21
#define         hPress          22
#define         aPress          23
#define         lPress          24
#define         hVis            25
#define         aVis            26
#define         lVis            27
#define         fog             28
#define         rain            29
#define         snow            30
#define         thunderstorm    31
#define         hWind           32
#define         aWind           33
#define         gWind           34
#define         pop             35
#define         LOA             36
#define         latitude        37
#define         longitude       38

    bool ok = true;
    valid = true;
    std::vector<TrainingParameters> runParams;

    // Split The Input String
    QStringList parameterStrings = line.split(',');
    if (parameterStrings.length() < 39) {
        valid = false;
        return runParams;
    }

    // Get all the iterated values
    std::vector<std::pair<int,int>> expansions;
    std::vector<int> counts;
    counts.resize(parameterStrings.length(),0);
    expansions.resize(parameterStrings.length());
    size_t totalVariations = 1;
    for (size_t i = 0; i < static_cast<size_t>(parameterStrings.length()); i++){
        if(parameterStrings[i].length() == 0){
            expansions[i] = std::pair<int,int>(-1,-1);
            counts[i] = 0;
        }
        else if(parameterStrings[i][0] == '<' && i < 6){
            switch (i){
            case topoTraining:
                expansions[i] = std::pair<int,int>(-1,-1);
                counts[i] = 2;
                totalVariations *= counts[i];
                break;
            case activation:
                expansions[i] = std::pair<int,int>(-1,-1);
                counts[i] = NUMACTIVATIONFUNCTIONS;
                totalVariations *= counts[i];
                break;
            default:
                expansions[i] = std::pair<int,int>(parameterStrings[i][1].digitValue(),parameterStrings[i][3].digitValue());
                counts[i] = expansions[i].second - expansions[i].first + 1;
                totalVariations *= counts[i];
                break;
            }
        }
        else{
            expansions[i] = std::pair<int,int>(-1,-1);
            counts[i] = 1;
        }
    }

    // Init the vector of training params
    runParams.resize(totalVariations);
    for(size_t i = 0; i < totalVariations; i++){
        runParams[i] = _defaultParams;
    }

    // Make a new entry for each itterative parameter
    int idx = 0;
    std::vector<int> topology;
    topology.resize(3,0);
    int topoIdx = 0;
    // Toopo Layer 1
    size_t layer1Lim = (counts[h1] > 1) ? counts[h1]:1;
    for (size_t layer1 = 0; layer1 < layer1Lim; layer1++){

        if (counts[h1] > 1){
            topoIdx = 0;
            topology[topoIdx] = expansions[h1].first + layer1;
        }
        else if (counts[h1] == 1){
            topoIdx = 0;
            topology[topoIdx] = std::max(parameterStrings[h1].toInt(&ok),1);
        }
        if(ok == false){
            valid = false;
        }

        // Topo Layer 2
        size_t layer2Lim = (counts[h2] > 1) ? counts[h2]:1;
        for(size_t layer2 = 0; layer2 < layer2Lim; layer2++){

            if (counts[h2] > 1){
                topoIdx = 1;
                topology[topoIdx] = expansions[h2].first + layer2;
            }
            else if (counts[h1] == 1){
                topoIdx = 1;
                topology[topoIdx] = std::max(parameterStrings[h2].toInt(&ok),1);
            }
            if(ok == false){
                valid = false;
            }

            // Topo Layer 3
            size_t layer3Lim = (counts[h3] > 1) ? counts[h3]:1;
            for(size_t layer3 = 0; layer3 < layer3Lim; layer3++){

                if (counts[h3] > 1){
                    topoIdx = 2;
                    topology[topoIdx] = expansions[h3].first + layer3;
                }
                else if (counts[h3] == 1){
                    topoIdx = 2;
                    topology[topoIdx] = std::max(parameterStrings[h3].toInt(&ok),1);
                }
                if(ok == false){
                    valid = false;
                }
                topology.resize(topoIdx + 1);
                runParams[idx].np.innerNetNodes = topology;

                // Topo Training On/Off
                size_t topoLim = (counts[topoTraining] > 1) ? counts[topoTraining]:1;
                for(size_t topo = 0; topo < topoLim; topo++){

                    if(counts[topoTraining] > 1){
                        switch (topo){
                            case 0:
                                runParams[idx].fp.enableTopologyTraining = true;
                                break;
                            case 1:
                                runParams[idx].fp.enableTopologyTraining = false;
                                break;
                        }
                    }
                    else if (counts[topoTraining] == 1){
                        if (parameterStrings[topoTraining] == "true"){
                            runParams[idx].fp.enableTopologyTraining = true;
                        }
                        else{
                            runParams[idx].fp.enableTopologyTraining = false;
                        }
                    }

                    // Activation function
                    size_t activationLim = (counts[activation] > 1) ? counts[activation]:1;
                    for(size_t act = 0; act < activationLim; act++){

                        if(counts[activation] > 1){
                            switch (act){
                            case 0:
                                runParams[idx].np.act = NeuralNet::ReLU;
                                break;
                            case 1:
                                runParams[idx].np.act = NeuralNet::Sin;
                                break;
                            case 2:
                                runParams[idx].np.act = NeuralNet::Sigmoid;
                                break;
                            case 3:
                                runParams[idx].np.act = NeuralNet::Gaussian;
                                break;
                            case 4:
                                runParams[idx].np.act = NeuralNet::HypTan;
                                break;
                            case 5:
                                runParams[idx].np.act = NeuralNet::Step;
                                break;
                            case 6:
                                runParams[idx].np.act = NeuralNet::Sinc;
                                break;
                            }
                        }
                        else if (counts[activation] == 1){
                            runParams[idx].np.act = parseActivationFunction(parameterStrings[activation]);
                        }

                        // History
                        size_t historyLim = (counts[history] > 1) ? counts[history]:1;
                        for(size_t hist = 0; hist < historyLim; hist++){

                            if(counts[history] > 1){
                                runParams[idx].inputHistorySize = expansions[history].first + hist;
                            }
                            else if (counts[history] == 1){
                                runParams[idx].inputHistorySize = std::max(parameterStrings[history].toInt(&ok),1);
                            }
                            if(ok == false){
                                valid = false;
                            }

                            // Max Epochs
                            if (counts[maxEpoc] > 0){
                                runParams[idx].pp.maxEpochs = std::max((parameterStrings[maxEpoc].toInt(&ok)),1);
                                if(ok == false){
                                    valid = false;
                                }
                            }

                            // Delta
                            if(counts[del] > 0){
                                runParams[idx].pp.delta = std::max(parameterStrings[del].toDouble(&ok),0.0);
                                if(ok == false){
                                    valid = false;
                                }
                            }

                            // Delta Window
                            if(counts[deltaWindow] > 0){
                                runParams[idx].pp.windowSize = std::max((parameterStrings[deltaWindow].toInt(&ok)),1);
                                if(ok == false){
                                    valid = false;
                                }
                            }

                            // Next Input
                            idx++;
                        }
                    }
                }
            }
        }
    }
    return runParams;
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
