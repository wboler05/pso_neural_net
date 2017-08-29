#include "petrainer.h"

PETrainer::PETrainer(const PEParameters & pe) :
    NeuralPso(pe.pp, pe.np, pe.fp),
    _images(nullptr),
    _labels(nullptr),
    _input(nullptr),
    _output(nullptr),
    _outputCount(nullptr)
{

}

void PETrainer::build(vector<vector<vector<byte> > > &images, vector<byte> &labels) {
  srand(time(NULL));
  _images = &images;
  _labels = &labels;

  buildPso();
}

void PETrainer::build(vector<vector<double>> &input, vector<double> &output) {
  srand(time(NULL));
  _input = &input;
  _output = &output;

  if (_outputCount != nullptr) {
      delete _outputCount;
  }
  _outputCount = new vector<double>();

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

void PETrainer::testGB() {
  _neuralNet->setWeights(_gb._x);
  int I = randomizeTestInputs();

  vector<double> res = _neuralNet->process();
  cout << _functionMsg << endl;
  cout << "Input: " << endl;
  for (uint i = 0; i < (*_input)[I].size(); i++) {
    cout << " - " << (*_input)[I][i] << endl;
  }
  cout << endl;

  cout << "Expected: " << (*_output)[I] << endl;
//    double maxVal = -1;
//    int answer = -1;
//    for (uint i = 0; i < res.size(); i++) {
//      if (res[i] > maxVal) {
//        maxVal = res[i];
//        answer = (int) i;
//      }
//    }
    int answer = -1;
    if (res[0] < 0.5) {
        answer = 0;
    } else if (res[0] >= 0.5) {
        answer = 1;
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
double PETrainer::testRun(double &correctRatio, uint &totalCount, double &confidence) {

    double mse = 0;
    uint outputNodes = _neuralNet->nParams()->outputs;
    //!TODO Reconsider confidence calculation
    confidence = 0;
    vector<double> outputError;

    uint totalSetsToRun = _neuralNet->nParams()->testIterations;
    uint correctCount = totalSetsToRun;

    TestStatistics testStats;

    vector<int> *answer = new vector<int>();
    answer->resize(totalSetsToRun);
    for (uint im = 0; im < totalSetsToRun; im++) {
        (*answer)[im] = 0;
    }

    // First, test each output and store to the vector of results;
    for (uint someSets = 0; someSets < totalSetsToRun; someSets++) {
        // Set a random input
        int I = randomizeTestInputs();
        //    double tConfidence = (double) outputNodes;
        //int I = It[someSets];
        //loadTestInput(I);

        // Get the result from random input
        vector<double> output = _neuralNet->process();
        if (output.size() != outputNodes) {
            cout << "Size mismatch on nodes. " << endl;
            cout << " - Output: " << output.size() << ", Expected: " << outputNodes << endl;
        }
        vector<double> expectedOutput;

        // Check if we have labels loaded or not
        int outputSize = output.size();
        if (_input == nullptr) {
            answer->at(someSets) = _labels->at(I);
        } else {
            answer->at(someSets) = _output->at(I);
        }

        // Initialize the expected output buffer
        expectedOutput.resize(outputSize);
        for (int i = 0; i < outputSize; i++) {
            if (i == answer->at(someSets)) {
                expectedOutput[i] = 1;
            } else {
                expectedOutput[i] = 0;
            }
        }

        //    double maxVal = -1.0;
        //    int maxNode = 0;
        //    // Compare the output to expected
        //    for (int i = 0; i < outputSize; i++) {
        //      double expected = expectedOutput[i];
        //      double got = output[i];
        //      if (output[i] > maxVal) {
        //        maxVal = output[i];
        //        maxNode = i;
        //      }
        //       double dif = expectedOutput[i] - output[i];
        //       tConfidence -= output[i];
        //       outputError[i] += pow(dif,2);
        //    }

        bool correctOutput;
        if (!validateOutput(output, expectedOutput, outputError, testStats, correctOutput)) {
            cout << "Validation failed to complete.";
            return 0;
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
                answer->push_back(0);
            }
        }

        //confidence += tConfidence;
    }
    //confidence /= totalSetsToRun;

    TestStatistics::ClassificationError ce;
    testStats.getClassError(&ce);

    // Get the MSE
    mse = 0;
    for (uint i = 0; i < outputError.size(); i++) {
        mse += pow(outputError[i]/totalSetsToRun, 2);
    }
    mse = sqrt(mse)/outputError.size();

    delete answer;

    correctRatio = (double) correctCount / (double) totalSetsToRun;
    totalCount = totalSetsToRun;

    double penalty = 1;
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

    double cost =  penalty *
            (_fParams.mse_weight*(1.0 - mse)
             + (_fParams.weights.accuracy*ce.accuracy)
             + (_fParams.weights.sensitivity*ce.sensitivity)
             + (_fParams.weights.specificity*ce.specificity)
             + (_fParams.weights.precision*ce.precision)
             + (_fParams.weights.f_score*ce.f_score));
    return cost * (double)totalSetsToRun;

  /* Previous tests

//  vector<double> w = {1, 0, 100};
//  double probSum = 0;
//  for (uint i = 0; i < w.size(); i++) {
//    probSum += w[i];
//  }

  return penalty *(1.0 - mse) * (1.0 - fn) * tp;

  return (5*accuracy + precision + 2*sensitivity + specificity  + f_score) * penalty * (1.0-mse);

  return sqrt(pow(accuracy,2) + pow(specificity,2) + pow(sensitivity, 2))/3;

  return (w[0]*((double) correctCount) + ((w[1]*confidence)+(w[2]*(1.0-mse)))) / probSum;

  */
}

bool PETrainer::validateOutput(
        std::vector<double> & outputs,
        std::vector<double> & expectedOutputs,
        std::vector<double> & outputError,
        TestStatistics & testStats,
        bool & correctOutput)
{
    // Check the output and expected output size are the same
    if (outputs.size() != expectedOutputs.size()) {
        // Return false if not
        std::cout << "Error validating: outputs do not match.";
        return false;
    }

    // Setup the output error buffer.
    outputError.resize(outputs.size());

    // Initialize to true, set to false when one breaks
    correctOutput = true;

    // For each output node
    for (size_t i = 0; i < outputs.size(); i++) {
        // Compare the output node to the expected answer
        // For our case, we just need between 0 and 1, with 0.5 being the threshold
        int result = convertOutput(outputs[i]);

        // Collect stats
        if (result == 1) {
            if (expectedOutputs[0] == 1) {
                testStats.addTn();
            } else {
                correctOutput = false;
                testStats.addFp();
            }
        } else {
            if (expectedOutputs[0] == 1) {
                correctOutput = false;
                testStats.addFn();
            } else {
                testStats.addTp();
            }
        }
        // Set the output error for sum of squares later
        outputError[i] = expectedOutputs[i] - outputs[i];
    }
    return true;
}

int PETrainer::randomizeTestInputs() {
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

void PETrainer::loadTestInput(uint I) {
  if (I >= _input->size()) return;

  _neuralNet->resetInputs();

  for (uint i = 0; i < (*_input)[I].size(); i++) {
    _neuralNet->loadInput((*_input)[I][i], i);
  }
}

void PETrainer::runTrainer() {

  run();

  printGB();

  _neuralNet->setWeights(gb()->_x);
}

void PETrainer::classError(TestStatistics::ClassificationError *ce) {
  _testStats.clear();

  for (uint i = 0; i < _input->size(); i++) {
    _neuralNet->resetInputs();
    for (uint j = 0; j < (*_input)[i].size(); j++) {
      _neuralNet->loadInput((*_input)[i][j], j);
    }
    double expectedAnswer = (*_output)[i];
    vector<double> output = _neuralNet->process();

    // Calling validateOutput would be expensive
    int answer = convertOutput(output[0]);

    if (expectedAnswer == 1) {
      if (answer == 1) {
        _testStats.addTp();
      } else {
        _testStats.addFn();
      }
    } else {
      if (answer == 1) {
        _testStats.addFp();
      } else {
        _testStats.addTn();
      }
    }
  }

  _testStats.getClassError(ce);

  string outputString = _testStats.outputString(ce);
  Logger::write(outputString);

}

int PETrainer::convertOutput(const double & output) {
    // Specific for the definition that below 0.5 is a negative result
    // above 0.5 is a positive result
    if (output < 0.5) {
        return 0;
    } else {
        return 1;
    }
}
