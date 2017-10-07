#include "OutageTrainer.h"

#include "PSO/pso.cpp"

OutageTrainer::OutageTrainer(const std::shared_ptr<TrainingParameters> & pe) :
    NeuralPso(pe->pp, pe->np, pe->fp),
    _params(pe)
{
    build();
}

void OutageTrainer::build() {
    /** TEST **/
    srand(time(NULL));
    _inputCache = std::make_shared<InputCache>(_params->cp);

    // Split between training, testing, and validation sets
    randomlyDistributeData();

    // Setup bias for rare events
    biasAgainstOutputs();

    buildPso();
}

void OutageTrainer::randomlyDistributeData() {
    /** TEST **/

    std::vector<real> splitPdf = {0.7225, .1275, .15};    // Setup PDF
    std::vector<real> splitCdf = cdfUniform(splitPdf);    // Calculate CDF

    std::default_random_engine gen;
    std::uniform_real_distribution<double> dist(0, 1);

    const size_t & inputSize = _inputCache->length();       // Get total inputs
    _trainingInputs.clear();                                // Clear input lists
    _testInputs.clear();
    _validationInputs.clear();
    for (size_t i = 0; i < inputSize; i++) {
        double randChoice = dist(gen);                      // Separate data
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
            _outputIterators[1].push_back(it);
        } else {
            _outputCount[0]++;
            _outputIterators[0].push_back(it);
        }
    }
}

void OutageTrainer::testGB() {
    /** TEST **/
  _neuralNet->setWeights(_gb._x);
  int I = randomizeTestInputs();
  size_t it = _testInputs[I];

  vector<real> res = _neuralNet->process();
//  cout << _functionMsg << endl;
//  cout << "Input: " << endl;
//  for (uint i = 0; i < (*_input)[I].size(); i++) {
//    cout << " - " << (*_input)[I][i] << endl;
//  }
//  cout << endl;

//  cout << "Expected: " << (*_output)[I] << endl;
    int answer = -1;
    if (convertOutput(res[0])) {
        answer = 1;
    } else {
        answer = 0;
    }
//    cout << "Got: " << answer << endl;
//    cout << endl;

//    cout << "Result: " << endl;
//    for (uint i = 0; i < res.size(); i++) {
//      cout << "- (" << i << "): " << res[i] << endl;
//    }
//    cout << endl;

    TestStatistics::ClassificationError ce;
    classError(&ce);
}

/// Passing null parameters to return stored values.
/// Correct ratio gives the ratio of correct runs.
/// Total Count gives the total runs that were executed.
/// Confidence returns how confident the net believes its answer is.
real OutageTrainer::testRun(real &correctRatio, uint &totalCount, real &confidence) {

    real mse = 0;
    uint outputNodes = _neuralNet->nParams()->outputs;
    //!TODO Reconsider confidence calculation
    confidence = 0;
    vector<real> outputError;
    std::vector<real> expectedOutput;
    expectedOutput.resize(outputNodes);

    uint totalSetsToRun = _neuralNet->nParams()->testIterations;
    uint correctCount = totalSetsToRun;

    TestStatistics testStats;

    // First, test each output and store to the vector of results;
    for (uint someSets = 0; someSets < totalSetsToRun; someSets++) {
        // Set a random input
        int I = randomizeTestInputs();
        //    real tConfidence = (real) outputNodes;
        //int I = It[someSets];
        loadTestInput(I);

        // Get the result from random input
        vector<real> output = _neuralNet->process();
        if (output.size() != outputNodes) {
            cout << "Size mismatch on nodes. " << endl;
            cout << " - Output: " << output.size() << ", Expected: " << outputNodes << endl;
        }

        expectedOutput[0] = _output->at(I);

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
    }
    //confidence /= totalSetsToRun;

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
        bool result = convertOutput(outputs.at(0));
        bool expRes = convertOutput(expectedResult[i]);

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

/**
 * @brief OutageTrainer::randomizeTestInputs
 * @return Iterator for testInput list
 */
int OutageTrainer::randomizeTestInputs() {
    _neuralNet->resetInputs();

    int uniformOutputIt = rand() % _outputIterators.size();
    int I = rand() % _outputIterators[uniformOutputIt].size();
    I = _outputIterators[uniformOutputIt][I];

    size_t it = _testInputs[I];

    OutageDataWrapper & item = (*_inputCache)[it];
    std::vector<real> inputItems = item.inputize();

    for (uint i = 0; i < inputItems.size(); i++) {
        _neuralNet->loadInput(inputItems[i], i);
    }

    return I;
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

void OutageTrainer::runTrainer() {

  run();

  printGB();

  _neuralNet->setWeights(gb()->_x);
}

void OutageTrainer::classError(TestStatistics::ClassificationError *ce) {
  //LEAVEOFFHERE
  _testStats.clear();

  size_t testIterations = _neuralNet->nParams()->testIterations;
  size_t inputSize = _testInputs->size();
  if (testIterations > inputSize) {
      testIterations = inputSize;
  }

  for (uint i = 0; i < testIterations; i++) {
    _neuralNet->resetInputs();
    loadTestInput(i);
    real expectedAnswer = (*_output)[i];
    vector<real> output = _neuralNet->process();

    // Calling validateOutput would be expensive
    bool result = convertOutput(output[0]);
    bool expectedResult = convertOutput(expectedAnswer);

    if (expectedResult) {
      if (result) {
        _testStats.addTp();
      } else {
        _testStats.addFn();
      }
    } else {
      if (result) {
        _testStats.addFp();
      } else {
        _testStats.addTn();
      }
    }
    if (_testStats.fp() + _testStats.fn() == 0.0) {
        if (++clampMax > _output->size()) {
            break;
        }
    }
  }

  _testStats.getClassError(ce);

  string outputString = _testStats.outputString(ce);
  Logger::write(outputString);

}

bool OutageTrainer::confirmOutage(const std::vector<real> & output) {
    if (output.size() < 2) {
        qWarning()<< "Error, wrong vector size for output.";
        return false;
    }
    if (output[0] < 0.5) {
        return false;
    } else {
        return true;
    }
}
