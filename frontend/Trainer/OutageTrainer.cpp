#include "OutageTrainer.h"

#include "PSO/pso.cpp"

OutageTrainer::OutageTrainer(const std::shared_ptr<TrainingParameters> & pe, const std::shared_ptr<InputCache> & inputCache) :
    NeuralPso(pe->pp, pe->np, pe->fp),
    _inputCache(inputCache),
    _params(pe)
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

    _outputCount.resize(2, 0);
    _outputIterators.resize(2);

    // Run through the training inputs
    for (size_t i = 0; i < _trainingInputs.size(); i++) {
        size_t it = _trainingInputs.at(i);
        OutageDataWrapper & dataItem = (*_inputCache)[it];
        if (dataItem.empty()) {
            continue;
        }

        // Count the true and false outages
        bool outage = confirmOutage(dataItem.outputize());
        if (outage) {
            _outputCount[1]++;
            _outputIterators[1].push_back(i);
        } else {
            _outputCount[0]++;
            _outputIterators[0].push_back(i);
        }
    }
}

void OutageTrainer::biasAgainstLOA() {

}

void OutageTrainer::runTrainer() {

  run();    // Pso

  // Termination
  printGB();
  _neuralNet->setWeights(gb()->_x);
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
    size_t outputNodes = static_cast<size_t>(_neuralNet->nParams()->outputs);
    std::vector<real> mse;
    mse.resize(outputNodes, 0);

//    vector<real> outputError;
    std::vector<real> expectedOutput;
    expectedOutput.resize(outputNodes);

    size_t totalSetsToRun = static_cast<size_t>(_neuralNet->nParams()->trainingIterations);
//    uint correctCount = totalSetsToRun;

//    TestStatistics testStats;

    // First, test each output and store to the vector of results;
    for (uint someSets = 0; someSets < totalSetsToRun; someSets++) {
        qApp->processEvents();
        // Set a random input
        size_t I = randomizeTrainingInputs();
        //    real tConfidence = (real) outputNodes;
        //int I = It[someSets];
        //loadTestInput(I);

        // Get the result from random input
        vector<real> output = _neuralNet->process();
        if (output.size() != outputNodes) {
            cout << "Size mismatch on nodes. " << endl;
            cout << " - Output: " << output.size() << ", Expected: " << outputNodes << endl;
        }

        //expectedOutput = _output->at(I);
        expectedOutput = (*_inputCache)[I].outputize();

        //    real maxVal = -1.0;
        //    int maxNode = 0;
        //    // Compare the output to expected
        //    for (int i = 0; i < outputSize; i++) {
        //      real expected = expectedOutput[i];
        //      real got = output[i];
        //      if (output[i] > maxVal) {
        //        maxVal = output[i];
        //        maxNode = i;
        //      }
        //       real dif = expectedOutput[i] - output[i];
        //       tConfidence -= output[i];
        //       outputError[i] += pow(dif,2);
        //    }

        std::vector<real> mse_outputs = OutageDataWrapper::splitMSE(output, expectedOutput);
        for (size_t output_nodes = 0; output_nodes < mse_outputs.size(); output_nodes++) {
            mse[output_nodes] += mse_outputs[output_nodes];
        }

        /*
        bool correctOutput;
        if (!validateOutput(output, expectedOutput, outputError, testStats, correctOutput)) {
            cout << "Validation failed to complete.";
            return 0;
        }

        // Get the MSE
        for (uint i = 0; i < outputError.size(); i++) {
            mse += pow(outputError[i], 2);
        }

        // If we have a correct answer, then we're heading in the right direction
        if (!correctOutput) {
            correctCount--;
        }

        // Expand the search if we get 100% correct
        if ((totalSetsToRun == correctCount) && (someSets == (totalSetsToRun - 1))) {
            if (totalSetsToRun < _input->size()) {
                ++totalSetsToRun;
                ++correctCount;
            }
        }

        //confidence += tConfidence;
        */
    }
    //confidence /= totalSetsToRun;

    for (size_t i = 0; i < mse.size(); i++) {
        mse[i] /= static_cast<real>(totalSetsToRun);
    }

    real cost = _params->alpha * mse[0] + _params->beta * mse[1];
    cost /= (_params->alpha + _params->beta);
    return 1-cost;


    /*

    mse = sqrt(mse)/outputError.size() / totalSetsToRun;

    //qDebug() << "MSE: " << mse;

    TestStatistics::ClassificationError ce;
    testStats.getClassError(&ce);



    correctRatio = (real) correctCount / (real) totalSetsToRun;
    totalCount = totalSetsToRun;

    real penalty = 1;
    if ((1-mse) < _fParams.mse_floor)
        penalty *= 0.00001;
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

    real cost =  penalty *
            (_fParams.mse_weight*(1.0 - mse)
             + (_fParams.weights.accuracy*ce.accuracy)
             + (_fParams.weights.sensitivity*ce.sensitivity)
             + (_fParams.weights.specificity*ce.specificity)
             + (_fParams.weights.precision*ce.precision)
             + (_fParams.weights.f_score*ce.f_score));
    return cost;
*/
  /* Previous tests

//  vector<real> w = {1, 0, 100};
//  real probSum = 0;
//  for (uint i = 0; i < w.size(); i++) {
//    probSum += w[i];
//  }

  return penalty *(1.0 - mse) * (1.0 - fn) * tp;

  return (5*accuracy + precision + 2*sensitivity + specificity  + f_score) * penalty * (1.0-mse);

  return sqrt(pow(accuracy,2) + pow(specificity,2) + pow(sensitivity, 2))/3;

  return (w[0]*((real) correctCount) + ((w[1]*confidence)+(w[2]*(1.0-mse)))) / probSum;

  */
}

/*
bool OutageTrainer::validateOutput(
        const std::vector<real> &outputs,
        const std::vector<real> & expectedResult,
        std::vector<real> & outputError,
        TestStatistics & testStats,
        bool & correctOutput)
{

    if (outputs.size() != expectedResult.size()) {
        std::cout<< "Error, output size does not match expected size.";
        return false;
    }

    // Setup the output error buffer.
    outputError.resize(outputs.size());

    // Initialize to true, set to false when one breaks
    correctOutput = true;

    for (size_t i = 0; i < outputs.size(); i++) {
        bool result = confirmOutage(outputs.at(0));
        bool expRes = confirmOutage(expectedResult[i]);

        // Collect stats
        if (result) {
            if (expRes) {
                testStats.addTp();
            } else {
                correctOutput = false;
                testStats.addFp();
            }
        } else {
            if (expRes) {
                correctOutput = false;
                testStats.addFn();
            } else {
                testStats.addTn();
            }
        }

        // Set the output error for sum of squares later
        outputError[i] = expectedResult[i] - outputs[i];
    }
    return true;
}
*/

void OutageTrainer::validateGB() {
    _neuralNet->setWeights(_gb._x);
    TestStatistics::ClassificationError ce;
    classError(_validationInputs, _validationStats, ce, _neuralNet->nParams()->validationIterations);
}

/**
 * @brief OutageTrainer::randomizeTestInputs
 * @return Iterator for testInput list
 */
size_t OutageTrainer::randomizeTrainingInputs() {
    _neuralNet->resetInputs();

    size_t minIt = 0;
    size_t maxIt = _outputIterators.size()-1;

//    size_t uniformOutputIt = _randomEngine.uniformUnsignedInt(minIt, maxIt);
    size_t uniformOutputIt = 1; /////BROKEN ON PURPOSE todo FIXME
    maxIt = _outputIterators[uniformOutputIt].size() -1;
    size_t I = _randomEngine.uniformUnsignedInt(minIt, maxIt);
    I = _outputIterators[uniformOutputIt][I];

    size_t it = _trainingInputs[I];

    OutageDataWrapper & item = (*_inputCache)[it];
    std::vector<real> inputItems = item.inputize();

    for (uint i = 0; i < inputItems.size(); i++) {
        _neuralNet->loadInput(inputItems[i], i);
    }

    return it;
}

OutageDataWrapper &OutageTrainer::loadTestInput(const size_t & I) {
    if (I >= _testInputs.size()) {
        OutageDataWrapper empty;
        return empty;
    }
    size_t it = _testInputs[I];

    _neuralNet->resetInputs();

    OutageDataWrapper & item = (*_inputCache)[it];
    std::vector<real> inputItems = item.inputize();

    for (uint i = 0; i < inputItems.size(); i++) {
        _neuralNet->loadInput(inputItems[i], i);
    }
    return item;
}

OutageDataWrapper & OutageTrainer::loadValidationInput(const size_t & I) {
    if (I >= _validationInputs.size()) {
        OutageDataWrapper empty;
        return empty;
    }
    size_t it = _validationInputs[I];

    _neuralNet->resetInputs();

    OutageDataWrapper & item = (*_inputCache)[it];
    std::vector<real> inputItems = item.inputize();

    for (uint i = 0; i < inputItems.size(); i++) {
      _neuralNet->loadInput(inputItems[i], i);
    }
}

void OutageTrainer::testGB() {
    /** TEST **/
  _neuralNet->setWeights(_gb._x);

    TestStatistics::ClassificationError ce;
    classError(_testInputs, _testStats, ce, _neuralNet->nParams()->testIterations);
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
        _neuralNet->resetInputs();

        OutageDataWrapper & outageData = (*_inputCache)[it];
        std::vector<real> inputItems = outageData.inputize();
        for (uint i = 0; i < inputItems.size(); i++) {
            _neuralNet->loadInput(inputItems[i], i);
        }

        std::vector<real> expectedOutput = outageData.outputize();
        std::vector<real> output = _neuralNet->process();

        mse += OutageDataWrapper::MSE(output, expectedOutput) / 2.0L;

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
  testStats.setMse(mse / static_cast<real>(iterations));

  string outputString = testStats.outputString(ce);
  Logger::write(outputString);

}

bool OutageTrainer::confirmOutage(const std::vector<real> & output) {
    if (output.size() < 2) {
        qWarning()<< "Error, wrong vector size for output.";
        return false;
    }
    if (output[0] < 0.5L) {
        return false;
    } else {
        return true;
    }
}
