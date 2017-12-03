#include "OutageTrainer.h"

#include "PSO/pso.cpp"

OutageTrainer::OutageTrainer(const std::shared_ptr<TrainingParameters> & pe, const std::shared_ptr<InputCache> & inputCache) :
    NeuralPso(pe->pp, pe->np, pe->fp),
    _params(pe),
    _inputCache(inputCache)
{
    updateEnableParameters();
    if (_params->enableBaseCase) {
        neuralNet()->setTotalInputs(_params->np.outputs);
        neuralNet()->buildANN();
    }

    // Querry and get the number of classes
    OutageDataWrapper dataItem = (*_inputCache)[0];
    vector<real> outputClassVector = dataItem.outputize();
    _numClasses = outputClassVector.size();
    _outputNodeStats.resize(_numClasses);

    // Build
    build();
}

void OutageTrainer::build() {
    /** TEST **/
    //_inputCache = std::make_shared<InputCache>(_params->cp);

    // Split between training, testing, and validation set.
    partitionData(_params->kFolds, _numClasses);

    buildPso();

    _dataSets.updateMinMax();
}

size_t OutageTrainer::getNextValidationSet(){
    return _dataSets.nextFold();
}

void OutageTrainer::partitionData(int kFolds, size_t numClasses){
    _dataSets = DataPartioner(kFolds, _params, numClasses, _inputCache);
}


void OutageTrainer::runTrainer() {

    _stopValidation = false;
    _validatedBests.clear();
    _validatedBests.resize(_dataSets.numFolds());

    size_t report = 1;
    while (report){
        _epochs = 0;
        resetFitnessScores();

        run();    // Pso

        if (!_neuralNet->setState(gb()->_x)) {
            qDebug( )<< "Unable to set NeuralNet State: runTrainer()";
            return;
        }

        // Switch sets for next fold
        report = getNextValidationSet();

        qDebug( )<< "Validation Fold: " << _dataSets.foldIndex();

        if (!report) {
            qDebug() << "Report is done.";
        }

        if (_stopValidation) {
            break;
        }
    }
    testGb();
    fullTestState();
    interruptProcess();

    _inputCache->printHistogram();
}

void OutageTrainer::resetFitnessScores() {
    _recent_gb.cm.reset();
    for (size_t i = 0; i < _particles->size(); i++) {
        (*_particles)[i]._fit_pb = -std::numeric_limits<real>::max();
        (*_particles)[i]._fit_lb = -std::numeric_limits<real>::max();
        (*_particles)[i]._fit = -std::numeric_limits<real>::max();
    }
    _gb._fit_pb = -std::numeric_limits<real>::max();

}

void OutageTrainer::trainingRun() {

    std::vector<size_t> trainingVector;
    _dataSets.getTrainingVector(trainingVector,_params->np.trainingIterations);

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
real OutageTrainer::trainingStep(const std::vector<size_t> & trainingVector) {
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
        std::vector<real> inputs = _dataSets.normalizeInput(I);
        _neuralNet->loadInputs(inputs);

        // Get the result from random input
        vector<real> output = _neuralNet->process();

        if (output.size() != outputNodes) {
            cout << "Size mismatch on nodes. " << endl;
            cout << " - Output: " << output.size() << ", Expected: " << outputNodes << endl;
            exit(1);
        }

        //expectedOutput = _output->at(I);
        expectedOutput = _dataSets.getDataWrapper(I).outputize();

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
        finalMse += mse[i] / _dataSets.getImplicitBiasWeight(i);
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

if (outputNodes == 2) {
    return -finalMse;
} else {
    return fitC-finalMse;
}

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

TestStatistics::ClassificationError && OutageTrainer::validateCurrentNet() {
    TestStatistics::ClassificationError ce;
    classError(_dataSets.getValidationSet(), _validationConfusionMatrix, _neuralNet->nParams()->validationIterations);
    return std::move(ce);
}

void OutageTrainer::validateGb() {
    if (!_neuralNet->setState(_gb._x)) {
        qDebug() << "Unable to validate net, cannot set weights: validateGb()";
        return;
    }
    TestStatistics::ClassificationError ce;
    classError(_dataSets.getValidationSet(), _validationConfusionMatrix, _dataSets.getValidationSet().size());

    _recent_gb.state = _gb._x;
    _recent_gb.cm = _validationConfusionMatrix;

    bool validated_gb_flag = true;

    _validationConfusionMatrix.costlyComputeClassStats();

    for (size_t i = 0; i < _validationConfusionMatrix.numberOfClassifiers(); i++) {
        int tp = static_cast<int> (_validationConfusionMatrix.getTruePositiveValues()[i]);
        if (tp == 0) {
            validated_gb_flag = false;
            break;
        }
    }

    if (validated_gb_flag) {
        if(_validatedBests[_dataSets.foldIndex()].cm.overallError().accuracy <= _recent_gb.cm.overallError().accuracy){
            _validatedBests[_dataSets.foldIndex()] = _recent_gb;
        }
    }


}

void OutageTrainer::testGb() {

    _best_overall_gb.cm.reset();
    _best_overall_gb.state.clear();

    _best_overall_gb = _recent_gb;

    for (size_t i = 0; i < _validatedBests.size(); i++) {

        if (!_neuralNet->setState(_validatedBests[i].state)) {
            qDebug( )<< "Unable to set NeuralNet: testGb()";
            continue;
        }
        classError(_dataSets.getTestSet(), _testConfusionMatrix, _dataSets.getTestSet().size());

        _testConfusionMatrix.costlyComputeClassStats();

        real newAcc = _testConfusionMatrix.overallError().accuracy;
        real gbAcc = _best_overall_gb.cm.overallError().accuracy;

        if (newAcc > gbAcc || _best_overall_gb.state.size() == 0) {
            _best_overall_gb.state = _validatedBests[i].state;
            _best_overall_gb.cm = _testConfusionMatrix;
        }
        //_selectedBestList.push_back(_validatedBests[i]);
    }
    _selectedBestList.push_back(_best_overall_gb);
}

void OutageTrainer::testSelectedGB() {
    if (!_neuralNet->setState(_best_overall_gb.state)) {
        qDebug() << "Unable to set NeuralNet: testSelectedGb()";
        return;
    }
    classError(_dataSets.getTestSet(), _best_overall_gb.cm, _neuralNet->nParams()->testIterations);
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
        std::vector<real> inputItems = _dataSets.normalizeInput(it);
        for (size_t i = 0; i < inputItems.size(); i++) {
            _neuralNet->loadInput(inputItems[i], i);
        }

        std::vector<real> expectedOutput = _dataSets.getDataWrapper(it).outputize();
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

void OutageTrainer::fullTestState(/*pass the gb or selected best option*/) {
    NeuralNet::State state;

    GlobalBestObject bestGb = _best_overall_gb;
    real acc = 0.0;

    for (size_t i = 0; i < _selectedBestList.size(); i++) {

        if (!_neuralNet->setState(_selectedBestList[i].state)) {
            qDebug() << "Error: Unable to set neural net. fullTestState()";
        }
        //_neuralNet->setState(state);

        std::vector<std::vector<real>> predictions, actuals;
        for (size_t j = 0; j < (*_inputCache).length(); j++) {
            qApp->processEvents();

            OutageDataWrapper data = (*_inputCache)[j];
            if (_params->enableBaseCase) {
                _neuralNet->loadInputs(data.outputize());
            } else {
                _neuralNet->loadInputs(_dataSets.normalizeInput(j));
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
        qDebug() << "AccX: " << acc << " New guy: " << cm.overallError().accuracy;
        if (cm.overallError().accuracy > acc) {
            acc = cm.overallError().accuracy;
            bestGb.state = _selectedBestList[i].state;
            bestGb.cm = _selectedBestList[i].cm;
        }
    }
   _sanityCheck_gb.cm = bestGb.cm; ///TODO: Replace this with its own object
   _sanityCheck_gb.state = bestGb.state;

}
