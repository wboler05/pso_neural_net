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

    buildPso();

    updateMinMax();
}

void OutageTrainer::partitionData(){

    const real boundRatio = 0.90;

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

    // Clear input lists
    _trainingInputs.clear();
    _testInputs.clear();
    _validationInputs.clear(); // Unused as of yet

    // Fill the vectors
    for (int i = 0; i < testBound; i++){
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

void OutageTrainer::biasAgainstOutputs() {
    /** TEST **/
/*
    _biasedTrainingInputsCounts.resize(2, 0);
    _biasedTrainingInputs.resize(2);

    // Run through the training inputs
    for (size_t i = 0; i < _trainingInputs.size(); i++) {
        size_t it = _trainingInputs.at(i);
        OutageDataWrapper dataItem = (*_inputCache)[it];
        if (dataItem.empty()) {
            continue;
        }

        // Count the true and false outages
        bool outage = confirmOutage(dataItem.outputize(_outputSkips));
        if (outage) {
            _biasedTrainingInputsCounts[1]++;
            _biasedTrainingInputs[1].push_back(it);
        } else {
            _biasedTrainingInputsCounts[0]++;
            _biasedTrainingInputs[0].push_back(it);
        }
    }
*/
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

        std::vector<real> mse_outputs = OutageDataWrapper::splitMSE(output, expectedOutput);
        for (size_t output_nodes = 0; output_nodes < mse_outputs.size(); output_nodes++) {
            mse[output_nodes] += mse_outputs[output_nodes];
        }

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

    }

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

    TestStatistics::ClassificationError ce;
    ce.mse = finalMse;
    trainingStats.getClassError(ce);

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
    classError(_validationInputs, _validationStats, ce, _neuralNet->nParams()->validationIterations);
}

TestStatistics::ClassificationError && OutageTrainer::validateCurrentNet() {
    TestStatistics::ClassificationError ce;
    classError(_validationInputs, _validationStats, ce, _neuralNet->nParams()->validationIterations);
    return std::move(ce);
}

/**
 * @brief OutageTrainer::randomizeTestInputs
 * @return Iterator for testInput list
 */
size_t OutageTrainer::randomizeTrainingInputs() {
    _neuralNet->resetAllNodes();

    size_t minIt = 0;
    size_t maxIt = _biasedTrainingInputs.size()-1;

    size_t uniformOutputIt = _randomEngine.uniformUnsignedInt(minIt, maxIt);
    size_t maxBiasIt = _biasedTrainingInputs[uniformOutputIt].size() -1;
    size_t I = _randomEngine.uniformUnsignedInt(minIt, maxBiasIt);
    size_t it = _biasedTrainingInputs[uniformOutputIt][I];

    std::vector<real> inputItems = normalizeInput(it);

    for (size_t i = 0; i < inputItems.size(); i++) {
        _neuralNet->loadInput(inputItems[i], i);
    }

    return it;
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

    TestStatistics::ClassificationError ce;
    classError(_testInputs, _testStats, ce, _neuralNet->nParams()->testIterations);

    _recent_gb.state = _gb._x;
    _recent_gb.testStats = _testStats;
    _recent_gb.ce = ce;

    if (_testStats.tn() > 0 &&
        _testStats.tp() > 0
        ) {
        if (ce.accuracy > _best_gb.ce.accuracy || _best_gb.state.size() == 0) {
            _best_gb.state = _gb._x;
            _best_gb.testStats = _testStats;
            _best_gb.ce = ce;
        }
    }
    /// test
    qDebug() << "Test GB For: " << _epochs;
    for (size_t i = 0; i < _outputNodeStats.size(); i++) {
        qDebug() << " - (" << i << "): Mean: " << _outputNodeStats[i].avg() << "\tStd: " << _outputNodeStats[i].std_dev();
    }
}

void OutageTrainer::testSelectedGB() {
    _neuralNet->setState(_best_gb.state);
    TestStatistics::ClassificationError ce;
    classError(_testInputs, _testStats, ce, _neuralNet->nParams()->testIterations);
    _best_gb.testStats = _testStats;
    _best_gb.ce = ce;
}

/**
 * @brief OutageTrainer::classError
 * @param ce
 * @todo Only needs to be implemented for classification node (0).
 * @todo Take out the training part and pass a list of results.
 */
void OutageTrainer::classError(const std::vector<size_t> & testInputs,
                               TestStatistics & testStats,
                               TestStatistics::ClassificationError & ce,
                               const size_t & testIterations) {
    //!TEST//
    testStats.clear();

    size_t iterations = testIterations;
    size_t inputSize = testInputs.size();
    if (iterations > inputSize) {
        iterations = inputSize;
    }

    real mse = 0L;

    for (size_t i = 0; i < iterations; i++) {
        size_t it = testInputs[i];
        _neuralNet->resetAllNodes();

        OutageDataWrapper outageData = (*_inputCache)[it];
//        std::vector<real> inputItems = outageData.inputize(_inputSkips);
        std::vector<real> inputItems = normalizeInput(it);
        for (size_t i = 0; i < inputItems.size(); i++) {
            _neuralNet->loadInput(inputItems[i], i);
        }

        std::vector<real> expectedOutput = outageData.outputize();
        std::vector<real> output = _neuralNet->process();

        mse += OutageDataWrapper::MSE(output, expectedOutput);

        bool result = confirmOutage(output);
        bool expectedResult = confirmOutage(expectedOutput);
//qDebug() << "Affected People: \t-Expected: " << expectedOutput[1] << "\t-Predicted: " << output[1] << "\t-Exp Outage: " << expectedOutput[0] << "\t-Act Outage: " << output[0];
        if (expectedResult) {
            if (result) {
                testStats.addTp();
            } else {
                testStats.addFn();
            }
        } else {
            if (result) {
                testStats.addFp();
            } else {
                testStats.addTn();
            }
        }
    }

  testStats.getClassError(ce);
  ce.mse = mse / static_cast<real>(iterations);

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
    return input;   // Yes, it's passing reference AND returning.  Look at polymorphic method.
}
