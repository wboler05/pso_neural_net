#include "OutageTrainer.h"

#include "PSO/pso.cpp"

OutageTrainer::OutageTrainer(const std::shared_ptr<TrainingParameters> & pe, const std::shared_ptr<InputCache> & inputCache) :
    NeuralPso(pe->pp, pe->np, pe->fp),
    _params(pe),
    _inputCache(inputCache)
{
    _inputSkips = pe->ep.inputSkips();
    if (_params->enableBaseCase) {
        neuralNet()->setTotalInputs(5);
        neuralNet()->buildANN();
    }
    _outputNodeStats.resize(5);
    build();
}

void OutageTrainer::build() {
    /** TEST **/
    //_inputCache = std::make_shared<InputCache>(_params->cp);

    // Split between training, testing, and validation set.
    partitionData();

    // Calculate the implicit bias weights of each class
    calcImplicitBiasWeights();

    buildPso();

    updateMinMax();
}

int OutageTrainer::getNextValidationSet(){
    _foldIdx++;
    if(_foldIdx < _kFolds){
        for(int i = 0; i < _numElePerValidationRound; i++){
            std::swap(&_validationInputs[i], &_trainingInputs[(_foldIdx-1)*_numElePerValidationRound + i]);
        }
        return 1;
    }
    return -1;
}

void OutageTrainer::partitionData(int kFolds){

    const real boundRatio = 0.90;
    _kFolds = kFolds;

    std::vector<int> indicies;
    int numInputSamples = static_cast<int>(_inputCache->totalInputItemsInFile());
    int swpIdx;
    int temp;

    // Initialize the vector
    for (int i = 0; i < numInputSamples; i++){
        indicies.push_back(i);
    }

    // Shuffle it
    for (int i = 1; i < numInputSamples; i++){
        swpIdx = _randomEngine.uniformUnsignedInt(0,i);
        if (swpIdx == i){
            continue;
        }
        else{
            temp = indicies[swpIdx];
            indicies[swpIdx] = indicies[i];
            indicies[i] = temp;
        }
    }

    // Calculate the bounding index (inclusive)
    int testBound = numInputSamples * boundRatio;
    _numElePerValidationRound = std::ceil(testBound/(_kFolds+1));


    // Clear input lists
    _trainingInputs.clear();
    _testInputs.clear();
    _validationInputs.clear();

    // Fill the vectors
    for (int i = 0; i < _numElePerValidationRound; i++){
        _validationInputs.push_back(indicies[i]);
    }
    for (int i = _numElePerValidationRound; i < testBound; i++){
        _trainingInputs.push_back(indicies[i]);
    }
    for (int i = testBound; i < numInputSamples; i++){
        _testInputs.push_back(indicies[i]);
    }
}

void OutageTrainer::randomlyDistributeData() {
    /** TEST **/

    std::vector<real> splitPdf = {0.81L, .09L, .10L};          // Setup PDF
    std::vector<real> splitCdf = cdfUniform(splitPdf);      // Calculate CDF

//    std::uniform_real_distribution<double> dist(0, 1);

    const size_t & inputSize = _inputCache->length();       // Get total inputs
    _trainingInputs.clear();                                // Clear input lists
    _testInputs.clear();
    _validationInputs.clear();
    for (size_t i = 0; i < inputSize; i++) {
//        double randChoice = dist(_randomEngine.engine());   // Separate data
        real randChoice = _randomEngine.uniformReal(0.0L, 1.0L);
        if (randChoice < splitCdf[0]) {
            _trainingInputs.push_back(i);
        } else if (randChoice < splitCdf[1]) {
            _testInputs.push_back(i);
        } else {
            _validationInputs.push_back(i);
        }
    }
}

void OutageTrainer::calcImplicitBiasWeights() {

    OutageDataWrapper dataItem = (*_inputCache)[0];
    vector<real> outputClassVector = dataItem.outputize();
    real numClasses = outputClassVector.size();
    _implicitBiasWeights.resize(static_cast<size_t>(numClasses),0);
    _trueNumElesPerClass.resize(static_cast<size_t>(numClasses),0);
    _equalizationFactors.resize(static_cast<size_t>(numClasses),0);
    _fitnessNormalizationFactor = 0;

    // Run through the data to get count of each class
    for (size_t i = 0; i < _inputCache->length(); i++) {
        dataItem = (*_inputCache)[i];
        if (dataItem.empty()) {
            continue;
        }
        outputClassVector = dataItem.outputize();
        for (size_t j = 0; j < static_cast<size_t>(numClasses); j++){
            if (outputClassVector[j] == 1.0){
                _trueNumElesPerClass[j]++;
                break;
            }
        }
    }
    // Calculate Ratios
    for (size_t i = 0; i < _implicitBiasWeights.size(); i++){
        _implicitBiasWeights[i] = static_cast<real>(_trueNumElesPerClass[i]) / static_cast<real>(_inputCache->length());
    }
    // Calculate Normalization Factor
    real a;
    for (size_t i = 0; i < static_cast<size_t>(numClasses); i++){
        a = ((static_cast<real>(_trueNumElesPerClass[i] - 1)/static_cast<real>(_inputCache->length()))/_implicitBiasWeights[i]);
        _equalizationFactors[i] = 1.0 / (1.0-(1.0/numClasses)*(numClasses - 1.0 + a));
        _fitnessNormalizationFactor += (1.0/numClasses) * _equalizationFactors[i];
    }
}

void OutageTrainer::biasAgainstLOA() {

}

void OutageTrainer::runTrainer() {

  run();    // Pso

  // Termination
  printGB();
  _neuralNet->setState(gb()->_x);
}

void OutageTrainer::trainingRun() {

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
        real fit = trainingStep(_trainingInputs);
        p->_fit = fit;
    }
}


/// Passing null parameters to return stored values.
/// Correct ratio gives the ratio of correct runs.
/// Total Count gives the total runs that were executed.
/// Confidence returns how confident the net believes its answer is.
/**
* @brief OutageTrainer::trainingRun
* @details Runs training for a single particle.
* @param correctRatio
* @param totalCount
* @param confidence
* @return
* @todo Need to swap iterators.  Train all particles on same dataset
*/
real OutageTrainer::trainingStep(const std::vector<size_t> & trainingInputs) {
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
    for (size_t someSets = 0; someSets < trainingIterations; someSets++) {
        qApp->processEvents();

        // Get the appropiate training input index
        size_t I = _trainingInputs[((_epochs-1)*trainingIterations + someSets) % _trainingInputs.size()];
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

        //TODO Replace this part with the cm
        bool result = confirmOutage(output);
        bool expectedResult = confirmOutage(expectedOutput);

        if (expectedResult) {
            if (result) {
                trainingStats.addTp();
            } else {
                trainingStats.addFn();
            }
        } else {
            if (result) {
                trainingStats.addFp();
            } else {
                trainingStats.addTn();
            }
        }
        // END TODO

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

    real acc = 0;
    cm.costlyComputeClassStats();
    for (size_t i = 0; i < cm.numberOfClassifiers(); i++) {
        if (_implicitBiasWeights[i] != 0.0) {
            acc += (cm.getTruePositiveRatios()[i] / _implicitBiasWeights[i]) * (_equalizationFactors[i]);
        } else {
            qWarning() << "OutageTrainer: Fitness Function is dividing by zero!!!";
            exit(1);
        }
    }
    acc /= static_cast<real>(cm.numberOfClassifiers());
    acc /= _fitnessNormalizationFactor;
    return acc;








    for (size_t i = 0; i < cm.numberOfClassifiers(); i++) {
        acc += CustomMath::pow(cm.classErrors()[i].accuracy,2);
    }
    return sqrt(acc);

std::vector<real> zeroVec(cm.numberOfClassifiers(), 0.0);
return ConfusionMatrix::MSE(cm.getTruePositiveRatios(), zeroVec);

return (_params->alpha * cm.overallError().accuracy - _params->beta * finalMse) /
        (_params->alpha + _params->beta);


    real penalty = 0.0;
    if (trainingStats.tp() < 1)
    {
        penalty += trainingStats.fp();
    }
    if (trainingStats.tn() < 1)
    {
        penalty += trainingStats.fn();
    }

    // Calculate the weighted MSE
    real costA = finalMse;

    // Life is a balancing act
    real costB = sqrt(pow(ce.specificity, 2) + pow(ce.sensitivity, 2));

    real cost = (_params->alpha * (-costA)) +
                (_params->beta * costB) -
                (_params->gamma * penalty);
    return cost;
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
    classError(_validationInputs, _validationConfusionMatrix, _neuralNet->nParams()->validationIterations);
}

TestStatistics::ClassificationError && OutageTrainer::validateCurrentNet() {
    TestStatistics::ClassificationError ce;
    classError(_validationInputs, _validationConfusionMatrix, _neuralNet->nParams()->validationIterations);
    return std::move(ce);
}

OutageDataWrapper && OutageTrainer::loadTestInput(const size_t & I) {
    OutageDataWrapper wrapper;
    if (I >= _testInputs.size()) {
        return std::move(wrapper);
    }
    size_t it = _testInputs[I];

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
    if (I >= _validationInputs.size()) {
        OutageDataWrapper empty;
        return std::move(empty);
    }
    size_t it = _validationInputs[I];

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

    classError(_testInputs, _testConfusionMatrix, _neuralNet->nParams()->testIterations);

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
        if (newAcc > gbAcc || _best_gb.state.size() == 0) {
//        if (ce.mse < _best_gb.ce.mse || _best_gb.state.size() == 0) {
            _best_gb.state = _gb._x;
            _best_gb.cm = _testConfusionMatrix;
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
    classError(_testInputs, _best_gb.cm, _neuralNet->nParams()->testIterations);
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
    cm.print();

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
    std::vector<real> testVector = (*_inputCache)[0].inputize(_inputSkips);
    _minInputData.resize(testVector.size(),  std::numeric_limits<real>::max());
    _maxInputData.resize(testVector.size(), -std::numeric_limits<real>::max());

    for (size_t i = 0; i < _inputCache->totalInputItemsInFile(); i++) {
        std::vector<real> input = (*_inputCache)[i].inputize(_inputSkips);
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
        std::vector<real> tempBuff = (*_inputCache)[id].inputize(_inputSkips);
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
