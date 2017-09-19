#include "OutageTrainer.h"

#include "PSO/pso.cpp"

OutageTrainer::OutageTrainer(const TrainingParameters & pe) :
    NeuralPso(pe.pp, pe.np, pe.fp),
    _input(nullptr),
    _output(nullptr),
    _outputCount(nullptr)
{

}

void OutageTrainer::build(vector<vector<real>> &input, vector<real> &output) {
  srand(time(NULL));
  _input = &input;
  _output = &output;

  if (_outputCount != nullptr) {
      delete _outputCount;
  }
  _outputCount = new vector<real>();

  int totalCount = 2 * _neuralNet->nParams()->outputs;
  _outputCount->resize(totalCount);
  _outputIterators.resize(totalCount);
  for(int i = 0; i < totalCount; i++) {
    (*_outputCount)[i] = 0;
  }

  //! TODO Make general for any size output nodes
  for (uint i = 0; i < _output->size(); i++) {
    int it = convertOutput(_output->at(i));
      (*_outputCount)[it]++;
    _outputIterators[it].push_back(i);
  }


  buildPso();
}

void OutageTrainer::testGB() {
  _neuralNet->setWeights(_gb._x);
  int I = randomizeTestInputs();

  vector<real> res = _neuralNet->process();
  cout << _functionMsg << endl;
  cout << "Input: " << endl;
  for (uint i = 0; i < (*_input)[I].size(); i++) {
    cout << " - " << (*_input)[I][i] << endl;
  }
  cout << endl;

  cout << "Expected: " << (*_output)[I] << endl;
//    real maxVal = -1;
//    int answer = -1;
//    for (uint i = 0; i < res.size(); i++) {
//      if (res[i] > maxVal) {
//        maxVal = res[i];
//        answer = (int) i;
//      }
//    }
    int answer = -1;
    if (convertOutput(res[0])) {
        answer = 1;
    } else {
        answer = 0;
    }
    cout << "Got: " << answer << endl;
    cout << endl;

    cout << "Result: " << endl;
    for (uint i = 0; i < res.size(); i++) {
      cout << "- (" << i << "): " << res[i] << endl;
    }
    cout << endl;

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

int OutageTrainer::randomizeTestInputs() {
  _neuralNet->resetInputs();

  int uniformOutputIt = rand() % _outputIterators.size();
  int I = rand() % _outputIterators[uniformOutputIt].size();
  I = _outputIterators[uniformOutputIt][I];

//  int I = rand() % _input->size();

  for (uint i = 0; i < (*_input)[I].size(); i++) {
    _neuralNet->loadInput((*_input)[I][i], i);
  }

  return I;
}

void OutageTrainer::loadTestInput(uint I) {
  if (I >= _input->size()) return;

  _neuralNet->resetInputs();

  for (uint i = 0; i < (*_input)[I].size(); i++) {
    _neuralNet->loadInput((*_input)[I][i], i);
  }
}

void OutageTrainer::loadValidationInput(size_t I) {
    //todo
}

void OutageTrainer::runTrainer() {

  run();

  printGB();

  _neuralNet->setWeights(gb()->_x);
}

void OutageTrainer::classError(TestStatistics::ClassificationError *ce) {
  _testStats.clear();

  size_t clampMax = _neuralNet->nParams()->testIterations;
  size_t inputSize = _input->size();
  if (inputSize > clampMax) {
      inputSize = clampMax;
  }

  for (uint i = 0; i < clampMax; i++) {
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

bool OutageTrainer::convertOutput(const real & output) {
    // Specific for the definition that below 0.5 is a negative result
    // above 0.5 is a positive result
    if (output < 0.5) {
        return false;
    } else {
        return true;
    }
}

real OutageTrainer::convertInput(const bool & b) {
    if (b) {
        return 1.0;
    } else {
        return 0.0;
    }
}
