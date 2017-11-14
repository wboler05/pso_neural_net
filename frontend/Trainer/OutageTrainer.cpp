#include "OutageTrainer.h"

#include "PSO/pso.cpp"

OutageTrainer::OutageTrainer(const std::shared_ptr<TrainingParameters> & pe, const std::shared_ptr<InputCache> & inputCache) :
    NeuralPso(pe->pp, pe->np, pe->fp),
    _params(pe),
    _inputCache(inputCache)
{
    _inputSkips = pe->ep.inputSkips();
    OutageDataWrapper::setInputSkips(_inputSkips);
    updateEnableParameters();
    if (_params->enableBaseCase) {
        neuralNet()->setTotalInputs(5);
        neuralNet()->buildANN();
    }
    _outputNodeStats.resize(5);
    // Querry and get the number of classes
    OutageDataWrapper dataItem = (*_inputCache)[0];
    vector<real> outputClassVector = dataItem.outputize();
    _numClasses = outputClassVector.size();
    // Build
    build();
}

void OutageTrainer::build() {
    /** TEST **/
    //_inputCache = std::make_shared<InputCache>(_params->cp);

    // Split between training, testing, and validation set.
    partitionData(10, _numClasses); // 10 Folds are hard coded but can make a GUI element later

    buildPso();

    updateMinMax();
}

size_t OutageTrainer::getNextValidationSet(){
    return _dataSets.nextFold();
}

void OutageTrainer::partitionData(int kFolds, size_t numClasses){
    _dataSets = DataPartioner(kFolds,static_cast<size_t>(_inputCache->totalInputItemsInFile()), numClasses, _inputCache);
}


void OutageTrainer::runTrainer() {

    _stopValidation = false;

    size_t report = 1;
    while (report){
        _selectedBestList.clear();

        run();    // Pso

        _selectedBestList.push_back(_recent_gb);
        fullTestState();
        _neuralNet->setState(gb()->_x);

        // Save Stats
        _validatedBests.push_back(_best_gb);   ///DO SOMETHING WITH THIS
        /// We need to do validation on the GB here.

        // Switch sets for next fold
        report = getNextValidationSet();

        qDebug( )<< "Validation Fold: " << _dataSets.foldIndex();

        if (_stopValidation) {
            break;
        }
    }
}

void OutageTrainer::trainingRun() {

    std::vector<size_t> trainingVector;

    // Get the cost for each particle's current position
    for (size_t i = 0; i < _particles->size(); i++) {
        if (checkTermProcess()) {
            return;
        }

        NeuralParticle *p = &(*_particles)[i];

        if (!_neuralNet->setState(p->_x)) {
            std::cout<< "Failure to set weights." << endl;
        }

        // Get fitness
        _dataSets.getTrainingVector(trainingVector,_params->np.trainingIterations);
        real fit = trainingStep(trainingVector);
        p->_fit = fit;
    }
}


/**
* @brief OutageTrainer::trainingStep
* @details Executes fitness function calculation.
* @param trainingInputs
* @return
*/
real OutageTrainer::trainingStep(const vector<size_t> & trainingVector) {
    // Validate that a path is good first
    if (!networkPathValidation()) {
        return -std::numeric_limits<real>::max();
    }

    size_t outputNodes = static_cast<size_t>(_neuralNet->nParams()->outputs);
    std::vector<real> mse;
    mse.resize(outputNodes, 0);

    std::vector<real> expectedOutput;
    expectedOutput.resize(outputNodes);

    size_t trainingIterations = static_cast<size_t>(_neuralNet->nParams()->trainingIterations);

    TestStatistics trainingStats;

    std::vector<std::vector<real>> predicted, actual;

    // First, test each output and store to the vector of results;
    for (size_t someSets = 0; someSets < trainingVector.size(); someSets++) {
        qApp->processEvents();

        // Get the appropiate training input index
        size_t I = trainingVector[someSets];
        OutageDataWrapper dataItem = (*_inputCache)[I];
        std::vector<real> inputs = normalizeInput(I);
        _neuralNet->loadInputs(inputs);

        // Get the result from random input
        vector<real> output = _neuralNet->process();

        if (output.size() != outputNodes) {
            cout << "Size mismatch on nodes. " << endl;
            cout << " - Output: " << output.size() << ", Expected: " << outputNodes << endl;
            exit(1);
        }

        //expectedOutput = _output->at(I);
        expectedOutput = dataItem.outputize();

        std::vector<real> mse_outputs = ConfusionMatrix::splitMSE(output, expectedOutput);
        for (size_t output_nodes = 0; output_nodes < mse_outputs.size(); output_nodes++) {
            mse[output_nodes] += mse_outputs[output_nodes];
        }

        predicted.push_back(output);
        actual.push_back(expectedOutput);

    }

    ConfusionMatrix::ClassifierMatrix classifierResults = ConfusionMatrix::evaluateResults(predicted, actual);
    ConfusionMatrix cm(classifierResults);
    //cm.print();

    for (size_t i = 0; i < mse.size(); i++) {
        mse[i] /= static_cast<real>(trainingIterations);
    }
    real finalMse = 0;
    for (size_t i = 0; i < mse.size(); i++) {
        finalMse += mse[i];
    }
    finalMse /= static_cast<real>(mse.size());

    for (size_t i = 0; i < mse.size(); i++) {
        _outputNodeStats[i].add_val(mse[i]);
    }

    ///MORE TESTING CONCEPTS YOLO HELLO
    trainingStats = cm.overallStats();
    TestStatistics::ClassificationError ce = cm.overallError();
    cm.overallError().mse = finalMse;
    real fitC = 0;
    for (size_t i = 0; i < cm.numberOfClassifiers(); i++) {
        if (cm.getTruePositiveValues()[i] > 0) {
            fitC += 1.0;
        }
    }
return fitC-finalMse;

    cm.costlyComputeClassStats();
    std::vector<real> zeroVec(cm.numberOfClassifiers(), 0);
    real fitA = CustomMath::mean(cm.getFalseNegativeRatios());// / (1 + ConfusionMatrix::MSE(cm.getFalseNegativeRatios(), zeroVec));
    real fitB = CustomMath::mean(cm.getFalsePositiveRatios());// / (1 + ConfusionMatrix::MSE(cm.getFalsePositiveRatios(), zeroVec));
    ///real fitC = 0;
    for (size_t i = 0; i < cm.numberOfClassifiers(); i++) {
        if (cm.getTruePositiveValues()[i] > 0) {
            fitC += 1.0;
        }
    }
    return _params->alpha * fitC - (_params->beta * fitA + _params->gamma * fitB);


    /// This one is good.
    real acc = 0;
    cm.costlyComputeClassStats();
    real corCount = 0;
    for (size_t i = 0; i < cm.numberOfClassifiers(); i++) {
        acc += cm.getTruePositiveRatios()[i] / _dataSets.getImplicitBiasWeight(i);
        if (cm.getTruePositiveValues()[i] > 0) {
            corCount += 1.0;
        }

        /*if (_implicitBiasWeights[i] != 0.0) {
            acc += (cm.getTruePositiveRatios()[i] / _implicitBiasWeights[i]) * (_equalizationFactors[i]);
        } else {
            qWarning() << "OutageTrainer: Fitness Function is dividing by zero!!!";
            exit(1);
        }*/
    }
    //qDebug() << "Test output: " << test;
    acc /= CustomMath::pow(static_cast<real>(_numClasses), 2);
    //acc /= _fitnessNormalizationFactor;
    return acc + corCount;
}

bool OutageTrainer::networkPathValidation() {
    if (_fParams.enableTopologyTraining) {
        return _neuralNet->validatePaths();
    } else {
        return true;
    }
}

void OutageTrainer::validateGB() {
    _neuralNet->setState(_gb._x);
    TestStatistics::ClassificationError ce;
    classError(_dataSets.getValidationSet(), _validationConfusionMatrix, _neuralNet->nParams()->validationIterations);
}

TestStatistics::ClassificationError && OutageTrainer::validateCurrentNet() {
    TestStatistics::ClassificationError ce;
    classError(_dataSets.getValidationSet(), _validationConfusionMatrix, _neuralNet->nParams()->validationIterations);
    return std::move(ce);
}

OutageDataWrapper && OutageTrainer::loadTestInput(const size_t & I) {
    OutageDataWrapper wrapper;
    if (I >= _dataSets.testSetSize()) {
        return std::move(wrapper);
    }
    size_t it = _dataSets.testSet(I);

    _neuralNet->resetAllNodes();

    OutageDataWrapper item = (*_inputCache)[it];
//    std::vector<real> inputItems = item.inputize(_inputSkips);
    std::vector<real> inputItems = normalizeInput(it);

    for (size_t i = 0; i < inputItems.size(); i++) {
        _neuralNet->loadInput(inputItems[i], i);
    }
    return std::move(item);
}

OutageDataWrapper && OutageTrainer::loadValidationInput(const size_t & I) {
    if (I >= _dataSets.validationSetSize()) {
        OutageDataWrapper empty;
        return std::move(empty);
    }
    size_t it = _dataSets.validationSet(I);

    _neuralNet->resetAllNodes();

    OutageDataWrapper item = (*_inputCache)[it];
//    std::vector<real> inputItems = item.inputize(_inputSkips);
    std::vector<real> inputItems = normalizeInput(it);

    for (size_t i = 0; i < inputItems.size(); i++) {
      _neuralNet->loadInput(inputItems[i], i);
    }
    return std::move(item);
}

void OutageTrainer::testGB() {
    /** TEST **/
  _neuralNet->setState(_gb._x);

    classError(_dataSets.getTestSet(), _testConfusionMatrix, _neuralNet->nParams()->testIterations);

    _recent_gb.state = _gb._x;
    _recent_gb.cm = _testConfusionMatrix;

    bool validated_gb_flag = true;

    _testConfusionMatrix.costlyComputeClassStats();

    for (size_t i = 0; i < _testConfusionMatrix.numberOfClassifiers(); i++) {
        int tp = static_cast<int> (round(_testConfusionMatrix.classStats()[i].tp()));
        int tn = static_cast<int> (round(_testConfusionMatrix.classStats()[i].tn()));
        if (tp == 0 || tn == 0) {
            validated_gb_flag = false;
        }
    }

    real newAcc = _testConfusionMatrix.overallError().accuracy;
    real gbAcc = _best_gb.cm.overallError().accuracy;

    if (validated_gb_flag) {
        if (newAcc >= gbAcc || _best_gb.state.size() == 0) {
//        if (ce.mse < _best_gb.ce.mse || _best_gb.state.size() == 0) {
            _best_gb.state = _gb._x;
            _best_gb.cm = _testConfusionMatrix;
            _selectedBestList.push_back(_best_gb);
            //fullTestState();
        }
    }
    /// test
    /*
    qDebug() << "Test GB For: " << _epochs;
    for (size_t i = 0; i < _outputNodeStats.size(); i++) {
        qDebug() << " - (" << i << "): Mean: " << _outputNodeStats[i].avg() << "\tStd: " << _outputNodeStats[i].std_dev();
    }
    */
}

void OutageTrainer::testSelectedGB() {
    _neuralNet->setState(_best_gb.state);
    classError(_dataSets.getTestSet(), _best_gb.cm, _neuralNet->nParams()->testIterations);
}

/**
 * @brief OutageTrainer::classError
 * @param ce
 * @todo Only needs to be implemented for classification node (0).
 * @todo Take out the training part and pass a list of results.
 */
void OutageTrainer::classError(const std::vector<size_t> & testInputs,
                               ConfusionMatrix & cm,
                               const size_t & testIterations) {
    //!TEST//
    cm.reset();

    size_t iterations = testIterations;
    size_t inputSize = testInputs.size();
    if (iterations > inputSize) {
        iterations = inputSize;
    }

    real mse = 0L;

    std::vector<std::vector<real>> predictions, actuals;

    for (size_t i = 0; i < iterations; i++) {
        size_t it = testInputs[i];
        _neuralNet->resetAllNodes();

        OutageDataWrapper outageData = (*_inputCache)[it];

        std::vector<real> inputItems = normalizeInput(it);
        for (size_t i = 0; i < inputItems.size(); i++) {
            _neuralNet->loadInput(inputItems[i], i);
        }

        std::vector<real> expectedOutput = outageData.outputize();
        std::vector<real> output = _neuralNet->process();

        actuals.push_back(expectedOutput);
        predictions.push_back(output);
    }
    ConfusionMatrix::ClassifierMatrix results = ConfusionMatrix::evaluateResults(predictions, actuals);
    cm.setResults(results);
    cm.overallError().mse = ConfusionMatrix::MSE(actuals, predictions);
    //cm.print();

//  string outputString = testStats.outputString(ce);
//  Logger::write(outputString);

}

bool OutageTrainer::confirmOutage(const std::vector<real> & output) {

    // Find the maximum index
    size_t severityIndex = 0;
    real maxVal = -std::numeric_limits<real>::max();
    for (size_t i = 0; i < output.size(); i++) {
        if (output[i] > maxVal) {
            severityIndex = i;
            maxVal = output[i];
        }
    }

    // Not an outage if not index 0
    return severityIndex != 0;
}

/**
 * @brief OutageTrainer::updateEnableParameters
 * @details Generate which elements to skip.
 */
void OutageTrainer::updateEnableParameters() {
    _inputSkips = _params->ep.inputSkips();
    OutageDataWrapper::setInputSkips(_inputSkips);
}

std::vector<size_t> EnableParameters::inputSkips() {
    std::vector<size_t> _inputSkips;
    if (!year) { _inputSkips.push_back(0); }
    if (!month) { _inputSkips.push_back(1); }
    if (!day) { _inputSkips.push_back(2); }
    if (!temp_high) { _inputSkips.push_back(3); }
    if (!temp_avg) { _inputSkips.push_back(4); }
    if (!temp_low) { _inputSkips.push_back(5); }
    if (!dew_high) { _inputSkips.push_back(6); }
    if (!dew_avg) { _inputSkips.push_back(7); }
    if (!dew_low) { _inputSkips.push_back(8); }
    if (!humidity_high) { _inputSkips.push_back(9); }
    if (!humidity_avg) { _inputSkips.push_back(10); }
    if (!humidity_low) { _inputSkips.push_back(11); }
    if (!press_high) { _inputSkips.push_back(12); }
    if (!press_avg) { _inputSkips.push_back(13); }
    if (!press_low) { _inputSkips.push_back(14); }
    if (!visibility_high) { _inputSkips.push_back(15); }
    if (!visibility_avg) { _inputSkips.push_back(16); }
    if (!visibility_low) { _inputSkips.push_back(17); }
    if (!wind_high) { _inputSkips.push_back(18); }
    if (!wind_avg) { _inputSkips.push_back(19); }
    if (!wind_gust) { _inputSkips.push_back(20); }
    if (!precipitation) { _inputSkips.push_back(21); }
    if (!fog) { _inputSkips.push_back(22); }
    if (!rain) { _inputSkips.push_back(23); }
    if (!snow) { _inputSkips.push_back(24); }
    if (!thunderstorm) { _inputSkips.push_back(25); }
    if (!loa) { _inputSkips.push_back(26); }
    if (!latitude) { _inputSkips.push_back(27); }
    if (!longitude) { _inputSkips.push_back(28); }
    return _inputSkips;
}

void OutageTrainer::updateMinMax() {
    std::vector<real> testVector = (*_inputCache)[0].inputize();
    _minInputData.resize(testVector.size(),  std::numeric_limits<real>::max());
    _maxInputData.resize(testVector.size(), -std::numeric_limits<real>::max());

    for (size_t i = 0; i < _inputCache->totalInputItemsInFile(); i++) {
        std::vector<real> input = (*_inputCache)[i].inputize();
        for (size_t j = 0; j < input.size(); j++) {
            _minInputData[j] = min(_minInputData[j], input[j]);
            _maxInputData[j] = max(_maxInputData[j], input[j]);
        }
    }
}

std::vector<real> OutageTrainer::normalizeInput(const size_t & id) {
    std::vector<real> inputVector;
    if (_params->enableBaseCase) {
        inputVector = (*_inputCache)[id].outputize();
    } else {
        std::vector<real> tempBuff = (*_inputCache)[id].inputize();
        inputVector = normalizeInput(tempBuff);
    }
    return inputVector;
}

std::vector<real> OutageTrainer::normalizeInput(std::vector<real> & input) {
    for (size_t i = 0; i < input.size(); i++) {
        if (_maxInputData[i] - _minInputData[i] != 0) {
            input[i] = (2.0*input[i] - (_minInputData[i] + _maxInputData[i])) /
                    (_maxInputData[i] - _minInputData[i]);
        }
    }
    return input;
}

void OutageTrainer::fullTestState(/*pass the gb or selected best option*/) {
    NeuralNet::State state;

    GlobalBestObject bestGb = _best_gb;
    real acc = 0.0;

    for (size_t i = 0; i < _selectedBestList.size(); i++) {

        _neuralNet->setState(_selectedBestList[i].state);
        //_neuralNet->setState(state);

        std::vector<std::vector<real>> predictions, actuals;
        for (size_t j = 0; j < (*_inputCache).length(); j++) {
            qApp->processEvents();

            OutageDataWrapper data = (*_inputCache)[j];
            if (_params->enableBaseCase) {
                _neuralNet->loadInputs(data.outputize());
            } else {
                _neuralNet->loadInputs(data.inputize());
            }
            std::vector<real> prediction = _neuralNet->process();
            std::vector<real> actual = data.outputize();

            predictions.push_back(prediction);
            actuals.push_back(actual);
        }
        ConfusionMatrix::ClassifierMatrix results = ConfusionMatrix::evaluateResults(predictions, actuals);
        ConfusionMatrix cm(results);
        cm.costlyComputeClassStats();
        _selectedBestList[i].cm = cm;
        qDebug() << "AccX: " << acc;
        if (cm.overallError().accuracy > acc) {
            acc = cm.overallError().accuracy;
            bestGb.state = _selectedBestList[i].state;
            bestGb.cm = _selectedBestList[i].cm;
        }
    }
   _sanityCheck_gb.cm = bestGb.cm; ///TODO: Replace this with its own object
   _sanityCheck_gb.state = bestGb.state;

}
