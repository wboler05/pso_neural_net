#include "OutageTrainer.h"

#include "PSO/pso.cpp"

OutageTrainer::OutageTrainer(const std::shared_ptr<TrainingParameters> & pe, const std::shared_ptr<InputCache> & inputCache) :
    NeuralPso(pe->pp, pe->np, pe->fp),
    _params(pe),
    _inputCache(inputCache)
{
    build();
}

void OutageTrainer::build() {
    /** TEST **/
    //_inputCache = std::make_shared<InputCache>(_params->cp);

    // Split between training, testing, and validation sets
    randomlyDistributeData();

    // Setup bias for rare events
    biasAgainstOutputs();
    //biasAgainstLOA();

    buildPso();

    updateMinMax();
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
}

void OutageTrainer::biasAgainstLOA() {

}

void OutageTrainer::runTrainer() {

  run();    // Pso

  // Termination
  printGB();
  _neuralNet->setState(gb()->_x);
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
real OutageTrainer::trainingRun() {
    // Validate that a path is good first
    if (_fParams.enableTopologyTraining) {
        if (!_neuralNet->validatePaths()) {
            return -std::numeric_limits<real>::max();
        }
    }

    size_t outputNodes = static_cast<size_t>(_neuralNet->nParams()->outputs);
    std::vector<real> mse;
    mse.resize(outputNodes, 0);

    std::vector<real> expectedOutput;
    expectedOutput.resize(outputNodes);

    size_t totalSetsToRun = static_cast<size_t>(_neuralNet->nParams()->trainingIterations);

    TestStatistics trainingStats;

    // First, test each output and store to the vector of results;
    for (size_t someSets = 0; someSets < totalSetsToRun; someSets++) {
        qApp->processEvents();
        // Set a random input
        size_t I = randomizeTrainingInputs();

        // Get the result from random input
        vector<real> output = _neuralNet->process();
        if (output.size() != outputNodes) {
            cout << "Size mismatch on nodes. " << endl;
            cout << " - Output: " << output.size() << ", Expected: " << outputNodes << endl;
            exit(1);
        }

        //expectedOutput = _output->at(I);
        OutageDataWrapper dataItem = (*_inputCache)[I];
        expectedOutput = dataItem.outputize(_outputSkips);

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
        mse[i] /= static_cast<real>(totalSetsToRun);
    }

    TestStatistics::ClassificationError ce = validateCurrentNet();
    trainingStats.getClassError(ce);

    real penalty = 1.0;
    if (trainingStats.tp() < 1)
    {
        penalty *= 10 + trainingStats.fp();
    }
    if (trainingStats.tn() < 1)
    {
        penalty *= 10 + trainingStats.fn();
    }

    // Calculate the weigted MSE
    real costA = -std::numeric_limits<real>::max();
    if (mse.size() == 2) {
        costA = (_params->alpha * mse[0] + _params->beta * mse[1]) /
                (2.0 * (_params->alpha + _params->beta));
    } else {
        costA = mse[0];
    }

    // Life is a balancing act
    real costB = sqrt(pow(ce.specificity, 2) + pow(ce.sensitivity, 2));

    real cost = (_params->gamma * (-costA) - costB) * penalty;
    return cost;

    /*
    // With this cost function, 0 means terminate.
    if (abs(costA) < _psoParams.delta && _psoParams.termDeltaFlag) {
        interruptProcess();
    }
    */

    return -costA * penalty;






    /*
    real penalty = 1;
//    if ((1-mse) < _fParams.mse_floor)
//        penalty *= 0.00001;
    if (ce.accuracy < _fParams.floors.accuracy)
        penalty *= 0.00001;
    if (ce.precision < _fParams.floors.precision)
        penalty *= 0.00001;
    if (ce.sensitivity < _fParams.floors.sensitivity)
        penalty *= 0.00001;
    if (ce.specificity < _fParams.floors.specificity)
        penalty *= 0.00001;
    if (ce.f_score < _fParams.floors.f_score)
        penalty *= 0.00001;

    real costA = -std::numeric_limits<real>::max();
    if (mse.size() == 2) {
        costA = (_params->alpha * mse[0] + _params->beta * mse[1]);
    } else {
        costA = mse[0];
    }

    real costB = ((_fParams.weights.accuracy*ce.accuracy)
                            + (_fParams.weights.sensitivity*ce.sensitivity)
                            + (_fParams.weights.specificity*ce.specificity)
                            + (_fParams.weights.precision*ce.precision)
                            + (_fParams.weights.f_score*ce.f_score) + 1);
    real costC = _params->gamma * ((std::sqrt(CustomMath::pow(ce.sensitivity, 2) +
                                              CustomMath::pow(ce.specificity,2))/2.0));
    costB /= (_fParams.weights.accuracy +
              _fParams.weights.sensitivity +
              _fParams.weights.specificity +
              _fParams.weights.precision +
              _fParams.weights.f_score + 1);
    real cost = (costB - _fParams.mse_weight * costA + costC) /
            (1 + _fParams.mse_weight + _params->alpha + _params->beta);

    return cost;
*/
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

//    OutageDataWrapper item =(*_inputCache)[it];
//    std::vector<real> inputItems = item.inputize(_inputSkips);
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

        std::vector<real> expectedOutput = outageData.outputize(_outputSkips);
        std::vector<real> output = _neuralNet->process();

        mse += OutageDataWrapper::MSE(output, expectedOutput) / static_cast<real>(2.0);

        bool result = confirmOutage(output);
        bool expectedResult = confirmOutage(expectedOutput);

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
    if (output.size() < 2) {
        qWarning()<< "Error, wrong vector size for output.";
        return false;
    }
    return OutageDataWrapper::double2Bool(output[0]);
}

/**
 * @brief OutageTrainer::updateEnableParameters
 * @details Generate which elements to skip.
 */
void OutageTrainer::updateEnableParameters() {
    _inputSkips = _params->ep.inputSkips();
    _outputSkips = _params->ep.outputSkips();
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

std::vector<size_t> EnableParameters::outputSkips() {
    std::vector<size_t> _outputSkips;
    if (!outage) { _outputSkips.push_back(0); }
    if (!affected_people) { _outputSkips.push_back(1); }
    return _outputSkips;
}

void OutageTrainer::updateMinMax() {
    std::vector<real> testVector = (*_inputCache)[0].inputize(_inputSkips);
    _minData.resize(testVector.size(),  std::numeric_limits<real>::max());
    _maxData.resize(testVector.size(), -std::numeric_limits<real>::max());

    for (size_t i = 0; i < _inputCache->totalInputItemsInFile(); i++) {
        std::vector<real> input = (*_inputCache)[i].inputize(_inputSkips);
        for (size_t j = 0; j < input.size(); j++) {
            _minData[j] = min(_minData[j], input[j]);
            _maxData[j] = max(_maxData[j], input[j]);
        }
    }
}

std::vector<real> OutageTrainer::normalizeInput(const size_t & id) {
    std::vector<real> inputVector = (*_inputCache)[id].inputize(_inputSkips);
    return normalizeInput(inputVector);
}

std::vector<real> OutageTrainer::normalizeInput(std::vector<real> & input) {
    for (size_t i = 0; i < input.size(); i++) {
        if (_maxData[i] - _minData[i] != 0) {
            input[i] = (2.0*input[i] - (_minData[i] + _maxData[i])) /
                    (_maxData[i] - _minData[i]);
        }
    }
    return input;   // Yes, it's passing reference AND returning.  Look at polymorphic method.
}
