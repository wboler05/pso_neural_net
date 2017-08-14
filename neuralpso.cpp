#include "neuralpso.h"

#include "PSO/pso.cpp"
//#include "NeuralNet/NeuralNet.cpp"

volatile bool stopProcessing = false;
bool NeuralPso::printGBFlag = false;
boost::mutex NeuralPso::printGBMtx;

NeuralPso::NeuralPso(PsoParams pp, NeuralNetParameters np) :
  Pso(pp),
  _neuralNet(new NeuralNet(np)),
  _images(nullptr),
  _labels(nullptr),
  _input(nullptr),
  _output(nullptr),
  _outputCount(nullptr)
{
  resetProcess();
}

NeuralPso::~NeuralPso() {
  delete _neuralNet;
}

void NeuralPso::buildPso() {
  _gb._fit_pb = numeric_limits<double>::min();

  _particles.empty();
  vector<vector<vector<double>>> &edges = _neuralNet->getWeights();

  // Create N particles
  _particles.resize(_psoParams.particles);
  for (uint i = 0; i < _psoParams.particles; i++) {
    // Create the number of inner columns
    _particles[i]._fit_pb = numeric_limits<double>::min();
    _particles[i]._fit_lb = numeric_limits<double>::min();

    _particles[i]._x.resize(edges.size());
    _particles[i]._v.resize(edges.size());
    _particles[i]._x_pb.resize(edges.size());
    _particles[i]._x_lb.resize(edges.size());
    _particles[i]._worstFlag = false;
    _particles[i]._points = _psoParams.startPoints;

    if (i == 0) {
      _gb._x.resize(edges.size());
    }
    // Create each inner column edge
    for (uint j = 0; j < edges.size(); j++) {
      // Create left edges
      _particles[i]._x[j].resize(edges[j].size());
      _particles[i]._v[j].resize(edges[j].size());
      _particles[i]._x_pb[j].resize(edges[j].size());
      _particles[i]._x_lb[j].resize(edges[j].size());
      if (i == 0) {
        _gb._x[j].resize(edges[j].size());
      }
      for (uint k = 0; k < edges[j].size(); k++) {
        // Create right edges
        _particles[i]._x[j][k].resize(edges[j][k].size());
        _particles[i]._v[j][k].resize(edges[j][k].size());
        _particles[i]._x_pb[j][k].resize(edges[j][k].size());
        _particles[i]._x_lb[j][k].resize(edges[j][k].size());
        if (i == 0) {
          _gb._x[j][k].resize(edges[j][k].size());
        }
        for (uint m = 0; m < edges[j][k].size(); m++) {
          _particles[i]._x[j][k][m] = (double) (rand() % 10000) / 10000;
          _particles[i]._v[j][k][m] = 0;
          _particles[i]._x_pb[j][k][m] = 0;
          _particles[i]._x_lb[j][k][m] = 0;

          if (i == 0) {
            _gb._x[j][k][m] = 0;
          }
        }
      }
    }
  }
}

void NeuralPso::build(vector<vector<vector<byte> > > &images, vector<byte> &labels) {
  srand(time(NULL));
  _images = &images;
  _labels = &labels;

  buildPso();
}

void NeuralPso::build(vector<vector<double>> &input, vector<double> &output) {
  srand(time(NULL));
  _input = &input;
  _output = &output;
  _outputCount = new vector<double>();

  _outputCount->resize(_neuralNet->nParams()->outputs);
  _outputIterators.resize(_neuralNet->nParams()->outputs);
  for(int i = 0; i < _neuralNet->nParams()->outputs; i++) {
    (*_outputCount)[i] = 0;
  }

  for (uint i = 0; i < _output->size(); i++) {
    (*_outputCount)[(*_output)[i]]++;
    _outputIterators[(*_output)[i]].push_back(i);
  }


  buildPso();
}

void NeuralPso::fly() {

  double velSum = 0;
  bool term = true;

  if (_particles.size() == 0) return;
  if (_particles[0]._v.size() == 0) return;

  static int innerNetAccessCount = _psoParams.iterationsPerLevel;
  static uint innerNetIt = _particles[0]._v.size()-1;

  double choice;
  bool worstFlag;

  double vLimit = _psoParams.vLimit;

                                // Random, pb, lb, gb
  static std::vector<double> pdf = {.001, 2, 8, 4};
  static std::vector<double> cdf = cdfUniform(pdf);

  // For each particle
  for (uint i = 0; i < _particles.size(); i++) {
    double C1 = 2.495, C2 = 2.495, C3 = 0.5;
    double dt = 1;
    Particle<vector<vector<vector<double>>>> *p = &_particles[i];
//p->_worstFlag = false;
    worstFlag = p->_worstFlag;

    if (worstFlag) {
      p->_worstFlag = false;
      if (p->_points <= 0) {
        choice = (double) (rand() % 10000) / 10000.0;
        p->_fit_pb = 0;
        p->_fit_lb = 0;
        p->_points = _psoParams.startPoints;
/*
        std::string callWeak;
        callWeak += "Switching particle ";
        callWeak += stringPut(i);
        callWeak += " with option ";
        callWeak += stringPut(choice);
        callWeak += "\n";
        Logger::write(callWeak);
*/

      } else {
        worstFlag = false;
        p->_points -= _psoParams.weakPoints;
      }
    }

    // For each inner net
    for (uint inner_net = 0; inner_net < p->_v.size(); inner_net++) {
      if (_psoParams.backPropagation && (inner_net != innerNetIt)) continue;
      // For each edge (left side) of that inner net
      for (uint left_edge = 0; left_edge < p->_v[inner_net].size(); left_edge++) {
        // For each edge (right side) of that inner net
        for (uint right_edge = 0; right_edge < p->_v[inner_net][left_edge].size(); right_edge++) {
          /// Concept based on Genetic Algorithms, idea based on Alex
          /// Find the worst of the particles and reset it.
          if (worstFlag) { // Reset the worst one
            if (choice < cdf[0]) {
              p->_x[inner_net][left_edge][right_edge] = ((double) (rand() % 20000) - 10000) / 10000.0;
            } else if (choice < cdf[1]) {
              p->_x[inner_net][left_edge][right_edge] = p->_x_pb[inner_net][left_edge][right_edge];
            } else if (choice < cdf[2]) {
              p->_x[inner_net][left_edge][right_edge] = p->_x_lb[inner_net][left_edge][right_edge];
            } else {
              p->_x[inner_net][left_edge][right_edge] = _gb._x[inner_net][left_edge][right_edge];
            }
            p->_v[inner_net][left_edge][right_edge] = ((double) (rand() % 20000) - 10000) / 10000.0;
            continue;
          }

          double *w_v = &p->_v[inner_net][left_edge][right_edge];
          double *w_x = &p->_x[inner_net][left_edge][right_edge];
          double *w_pb = &p->_x_pb[inner_net][left_edge][right_edge];
          double *w_lb = &p->_x_lb[inner_net][left_edge][right_edge];
          double *w_gb = &_gb._x[inner_net][left_edge][right_edge];

          double inertia = ((double) ((rand() % 50000) + 50000)) / 100000.0;
          double c1 = C1 * ((double) (rand() % 10000)) / 10000.0;
          double c2 = C2 * ((double) (rand() % 10000)) / 10000.0;
          double c3 = C3 * ((double) (rand() % 10000)) / 10000.0;

          *w_v = (inertia * (*w_v) + (c1*(*w_pb - *w_x)) + (c2*(*w_lb - *w_x)) + (c3*(*w_gb - *w_x))) * dt;
          velSum += *w_v;
          term = term && (*w_pb == *w_x) && (*w_lb == *w_x);

          if (*w_v > vLimit) {
            *w_v = vLimit;
          } else if (*w_v < -vLimit) {
            *w_v = -vLimit;
          }

          *w_x += *w_v;

          if (*w_x > 1.0) {
            *w_x = -1.0;
            //*w_v *= -0.01;
          } else if (*w_x < -1.0) {
            *w_x = 1.0;
            //*w_v *= -0.01;
          }
        }
      }
    }
  }

  if (innerNetAccessCount-- == 0) {
    innerNetAccessCount = _psoParams.iterationsPerLevel;
    if (innerNetIt-- == 0) {
      innerNetIt = _neuralNet->nParams()->innerNetNodes.size()-1;
    }
  }

//  if (velSum < _psoParams.vDelta)
//    _overideTermFlag = true;
  if (term)
    interruptProcess();

  // If stagnant, mix it up a bit
  if (abs(velSum) < (uint) _psoParams.vDelta) {
    for (uint i = 0; i < _particles.size(); i++) {
    // Reset bests
    _particles[i]._fit_pb = numeric_limits<double>::max();
    _particles[i]._fit_lb = numeric_limits<double>::max();
      for (uint j = 0; j < _particles[i]._x.size(); j++) {
        for (uint k = 0; k < _particles[i]._x[j].size(); k++) {
          for (uint m = 0; m < _particles[i]._x[j][k].size(); m++) {
            _particles[i]._x[j][k][m] = ((double)(rand() % 20000)/10000.0) - 1.0;
          }
        }
      }
    }
    cout << endl;
  }
  //cout << velSum << endl;

  /// For termination through user control
  if (stopProcessing) {
    interruptProcess();
  }
}

void NeuralPso::getCost() {
  double correctRatio=0;
  int totalCount=0;
  bool printChange = false;
  double worstFit = numeric_limits<double>::max();
  double worstFitIt = 0;
  static double prevBest = 0;

  for (uint i = 0; i < _particles.size(); i++) {
    Particle<vector<vector<vector<double>>>> *p = &_particles[i];

    if (!_neuralNet->setWeights(&p->_x)) {
      std::cout<< "Failure to set weights." << endl;
    }

    // Initialize storage variables
    double tempCorrectRatio = 0;
    uint tempTotalCount = 0;
    double confidence = 0;

    // Get fitness
    double fit = testRun(tempCorrectRatio, tempTotalCount, confidence);

    // Track worst fitness
    if (worstFit >= fit) {
      worstFit = fit;
      worstFitIt = i;
    }

    p->_points -= _psoParams.decayPoints;

    // Find personal best
    if (fit > p->_fit_pb) {
      p->_fit_pb = fit;
      p->_points += _psoParams.pbPoints;
      printChange = true;

      for (uint i = 0; i < p->_x_pb.size(); i++) {
        for (uint j = 0; j < p->_x_pb[i].size(); j++) {
          for (uint k = 0; k < p->_x_pb[i][j].size(); k++) {
            p->_x_pb[i][j][k] = p->_x[i][j][k];
          }
        }
      }
    } // end personal best

    // Find global best
    if (fit > _gb._fit_pb) {
      _gb._fit_pb = fit;
      p->_points += _psoParams.gbPoints;
      printChange = true;

      for (uint i = 0; i < p->_x.size(); i++) {
        for (uint j = 0; j < p->_x[i].size(); j++) {
          for (uint k = 0; k < p->_x[i][j].size(); k++) {
            _gb._x[i][j][k] = p->_x[i][j][k];
          }
        }
      }
      correctRatio = tempCorrectRatio;
      totalCount = tempTotalCount;
    } // End global best


    // Find local best
    int left_i = i - (_psoParams.neighbors / 2);
    if (left_i < 0) left_i += _particles.size();
    for (uint j = 0; j < _psoParams.neighbors; j++) {
      uint it = (uint) left_i + j;
      if (it < 0) {
        it += _particles.size();
      } else if (it >= _particles.size()) {
        it -= _particles.size();
      }

      Particle<vector<vector<vector<double>>>> *p_n = &_particles[it];
      if (fit > p_n->_fit_lb) {
        p_n->_fit_lb = fit;
        p->_points += _psoParams.lbPoints;
        printChange = true;

        for (uint i = 0; i < p_n->_x_lb.size(); i++) {
          uint size2 = p_n->_x_lb[i].size();
          for (uint j = 0; j < p_n->_x_lb[i].size(); j++) {
            for (uint k = 0; k < p_n->_x_lb[i][j].size(); j++) {
              if (j >= size2) continue;
              p_n->_x_lb[i][j][k] = p->_x[i][j][k];
            }
          }
        }
      }
    } // End local best

    // Handle logging
    if (printChange) {
      printChange = false;
      std::string outputString;
      outputString += "Particle (";
      outputString += stringPut(i);
      outputString += "):: Fit: ";
      outputString += stringPut(fit);
      outputString += "\tPB: ";
      outputString += stringPut(p->_fit_pb);
      outputString += "\tLB: ";
      outputString += stringPut(p->_fit_lb);
      outputString += "\tGB: ";
      outputString += stringPut(gb()->_fit_pb);
      outputString += "\tCor: ";
      outputString += stringPut(correctRatio);
      outputString += "\tPts: ";
      outputString += stringPut(p->_points);
      outputString += "\n";
      Logger::write(outputString);

    }
  } // end for each particle

  // Print out global change made
  if (prevBest != gb()->_fit_pb) {
      prevBest = gb()->_fit_pb;

      std::string outputString;
      outputString += "Global: ";
      outputString += stringPut(gb()->_fit_pb);
      outputString += " - Correct: ";
      outputString += stringPut(correctRatio * 100.0);
      outputString += "% of ";
      outputString += stringPut(totalCount);
      outputString += " tests\n";
      Logger::write(outputString);

      printGB();
      testGB();
  }

  // Set the worst fit flag for particle
  _particles[worstFitIt]._worstFlag = true;

  // Notify exit condition
  ///FIXME: Set the correct exit condition.
  if (totalCount > 400 && correctRatio > 0.9995)
    interruptProcess();


  if (checkForPrint()) {
    testGB();
  }

  printGBMtx.lock();
  if (printGBFlag) {
    printGBFlag = false;
    printGB();
  }
  printGBMtx.unlock();
} // end getCost()

void NeuralPso::testGB() {
  _neuralNet->setWeights(&_gb._x);
  int I = randomizeTestInputs();

  vector<double> res = _neuralNet->process();
  cout << _functionMsg << endl;
  cout << "Input: " << endl;
  for (uint i = 0; i < (*_input)[I].size(); i++) {
    cout << " - " << (*_input)[I][i] << endl;
  }
  cout << endl;

  cout << "Expected: " << (*_output)[I] << endl;
    double maxVal = -1;
    int answer = -1;
    for (uint i = 0; i < res.size(); i++) {
      if (res[i] > maxVal) {
        maxVal = res[i];
        answer = (int) i;
      }
    }
    cout << "Got: " << answer << endl;
    cout << endl;

    cout << "Result: " << endl;
    for (uint i = 0; i < res.size(); i++) {
      cout << "- (" << i << "): " << res[i] << endl;
    }
    cout << endl;

    classError();
}

/// Passing null parameters to return stored values.
/// Correct ratio gives the ratio of correct runs.
/// Total Count gives the total runs that were executed.
/// Confidence returns how confident the net believes its answer is.
double NeuralPso::testRun(double &correctRatio, uint &totalCount, double &confidence) {

  double mse = 0;
  double answerFailPenalty = 1.0;
  uint outputNodes = _neuralNet->nParams()->outputs;
  confidence = 0;
  vector<double> outputError;
  outputError.resize(outputNodes);
  for (uint i = 0; i < outputNodes; i++) {
    outputError[i] = 0;
  }

  double bias = 0;
  double biasSum = 0;
  for (uint i = 0; i < _outputCount->size(); i++) {
    biasSum+= (*_outputCount)[i];
  }

  uint totalSetsToRun = _neuralNet->nParams()->testIterations;
  uint correctCount = totalSetsToRun;

  double tp = 0, tn = 0, fp = 0, fn = 0;

  vector<int> *answer = new vector<int>();
  answer->resize(totalSetsToRun);
  for (uint im = 0; im < totalSetsToRun; im++) {
    (*answer)[im] = 0;
  }

  // First, test each output and store to the vector of results;
  for (uint someSets = 0; someSets < totalSetsToRun; someSets++) {
    // Set a random input
    int I = randomizeTestInputs();
    double tConfidence = (double) outputNodes;
    //int I = It[someSets];
    //loadTestInput(I);

    // Get the result from random input
    vector<double> output = _neuralNet->process();
    if (output.size() != outputNodes) {
      cout << "Size mismatch on nodes. " << endl;
      cout << " - Output: " << output.size() << ", Expected: " << outputNodes << endl;
    }
    vector<double> expectedOutput;

    int outputSize = output.size();
    if (_input == nullptr) {
      answer->at(someSets) = _labels->at(I);
    } else {
      answer->at(someSets) = _output->at(I);
    }

    expectedOutput.resize(outputSize);


    for (int i = 0; i < outputSize; i++) {
      if (i == answer->at(someSets)) {
        expectedOutput[i] = 1;
      } else {
        expectedOutput[i] = 0;
      }
    }

    double maxVal = -1.0;
    int maxNode = 0;
    // Compare the output to expected
    for (int i = 0; i < outputSize; i++) {
      double expected = expectedOutput[i];
      bias += expected / biasSum;
      double got = output[i];
      if (output[i] > maxVal) {
        maxVal = output[i];
        maxNode = i;
      }
       double dif = expectedOutput[i] - output[i];
       tConfidence -= output[i];
       outputError[i] += pow(dif,2);
    }

    // If we have a correct answer, then we're heading in the right direction
    // Give him a cookie.
    if (maxNode != answer->at(someSets)) {
      correctCount--;
    } else {

    }

    /// Only works for binary

    if (output[0] == 1) {
      if (expectedOutput[0] == 1) {
        tn++;
      } else {
        fp++;
      }
    } else {
      if (expectedOutput[0] == 1) {
        fn++;
      } else {
        tp++;
      }
    }

    /// End bs

    if ((totalSetsToRun == correctCount) && (someSets == (totalSetsToRun - 1))) {
      if (totalSetsToRun < _input->size()) {
        ++totalSetsToRun;
        ++correctCount;
        answer->push_back(0);
      }
    }

    confidence += tConfidence;
  }
  confidence /= totalSetsToRun;
  bias /= totalSetsToRun;

  double accuracy = N_Accuracy(tp, tn, fp, fn);
  double precision = N_Precision(tp, fp);
  double sensitivity = N_Sensitivity(tp, fn);
  double specificity = N_Specificity(tn, fp);
  double f_score = N_F_Score(tp, fp, fn);



  ///TEMP: Use mean square of output errors instead.
  mse = 0;
  for (uint i = 0; i < outputError.size(); i++) {
    mse += pow(outputError[i]/totalSetsToRun, 2);
  }
  mse = sqrt(mse)/outputError.size();

  delete answer;

  correctRatio = (double) correctCount / (double) totalSetsToRun;
  totalCount = totalSetsToRun;

  vector<double> w = {1, 0, 100};
  double probSum = 0;
  for (uint i = 0; i < w.size(); i++) {
    probSum += w[i];
  }

  double penalty = 1;
  if (accuracy < 0.7)
    penalty *= 0.00001;
  if (precision < .15)
    penalty *= 0.1;
  if (sensitivity < 0.6)
    penalty *= 0.01;
  if (specificity < 0.5)
    penalty *= 0.1;

  double population = tp + tn + fp + tn;
  fn /= population;
  fp /= population;
  tp /= population;
  tn /= population;

  if (fn > 0.05) {
    penalty *= 0.1;
  }

 // return tp * tn * (1.0 - fp) * (1.0 - fn);

 // Weights :       // Best
 double w_err = 1; // 10
 double w_acc = 0;//1000; // 20000
 double w_pre = 0;//10; // 55
 double w_sen = 0;//100000; // 500
 double w_spe = 0;//100; // 17
 double w_fsc = 0;//1;


  return penalty *(w_err*(1.0 - mse) + w_acc*accuracy + w_sen*sensitivity + w_spe*specificity + w_pre*precision + w_fsc*f_score);

  /* Previous tests

  return penalty *(1.0 - mse) * (1.0 - fn) * tp;

  return (5*accuracy + precision + 2*sensitivity + specificity  + f_score) * penalty * (1.0-mse);

  return sqrt(pow(accuracy,2) + pow(specificity,2) + pow(sensitivity, 2))/3;

  return (w[0]*((double) correctCount) + ((w[1]*confidence)+(w[2]*(1.0-mse)))) / probSum;

  */
}

int NeuralPso::randomizeTestInputs() {
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

void NeuralPso::loadTestInput(uint I) {
  if (I >= _input->size()) return;

  _neuralNet->resetInputs();

  for (uint i = 0; i < (*_input)[I].size(); i++) {
    _neuralNet->loadInput((*_input)[I][i], i);
  }
}

void NeuralPso::runTrainer() {

  run();

  printGB();

  _neuralNet->setWeights(&gb()->_x);
}

void NeuralPso::printGB() {
  std::string printString;
  printString += "Global Best: \n";
  for (uint i = 0; i < _gb._x.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < gb()->_x[i].size(); j++) {
      for (uint k = 0; k < gb()->_x[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut(gb()->_x[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

void NeuralPso::printParticle(uint I) {
  if (I > _particles.size()) return;

  std::string printString;
  printString += "Particle (";
  printString += stringPut(I);
  printString += "): \n";
  for (uint i = 0; i < _particles[I]._x.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < _particles[I]._x[i].size(); j++) {
      for (uint k = 0; k < _particles[I]._x[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut(_particles[I]._x[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

void NeuralPso::printParticlePBest(uint I) {
  if (I > _particles.size()) return;

  std::string printString;
  printString += "Particle pBest (";
  printString += stringPut(I);
  printString += "): \n";
  for (uint i = 0; i < _particles[I]._x_pb.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < _particles[I]._x_pb[i].size(); j++) {
      for (uint k = 0; k < _particles[I]._x_pb[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut(_particles[I]._x_pb[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

void NeuralPso::printParticleLBest(uint I) {
  if (I > _particles.size()) return;

  std::string printString;
  printString += "Particle lBest (";
  printString += stringPut(I);
  printString += "): \n";
  for (uint i = 0; i < _particles[I]._x_lb.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < _particles[I]._x_lb[i].size(); j++) {
      for (uint k = 0; k < _particles[I]._x_lb[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut(_particles[I]._x_lb[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

void NeuralPso::setToPrintGBNet() {
  printGBMtx.lock();
  printGBFlag = true;
  printGBMtx.unlock();
}

void NeuralPso::classError() {
  double tp = 0, tn = 0,
         fp = 0, fn = 0;

  for (uint i = 0; i < _input->size(); i++) {
    _neuralNet->resetInputs();
    for (uint j = 0; j < (*_input)[i].size(); j++) {
      _neuralNet->loadInput((*_input)[i][j], j);
    }
    double expectedAnswer = (*_output)[i];
    vector<double> output = _neuralNet->process();

    double maxVal = 0;
    double answer = 0;
    for (uint j = 0; j < output.size(); j++) {
      if (output[j] > maxVal) {
        maxVal = output[j];
        answer = j;
      }
    }

    if (expectedAnswer == 1) {
      if (answer == 1) {
        tp++;
      } else {
        fn++;
      }
    } else {
      if (answer == 1) {
        fp++;
      } else {
        tn++;
      }
    }
  }

  double accuracy = N_Accuracy(tp, tn, fp, fn);
  double precision = N_Precision(tp, fp);
  double sensitivity = N_Sensitivity(tp, fn);
  double specificity = N_Specificity(tn, fp);
  double f_score = N_F_Score(tp, fp, fn);
  double g_score = N_G_Score(tp, fp, fn);

  double inputSize = _input->size();
  tp /= inputSize;
  tn /= inputSize;
  fp /= inputSize;
  fn /= inputSize;

  string outputString;
  outputString += "Act\\Out\tFalse\t\tTrue\n";
  outputString += "False\t";
  outputString += stringPut(tn);
  outputString += "\t";
  outputString += stringPut(fp);
  outputString += "\t";
  outputString += stringPut(tn + fp);
  outputString += "\nTrue\t";
  outputString += stringPut(fn);
  outputString += "\t";
  outputString += stringPut(tp);
  outputString += "\t";
  outputString += stringPut(fn + tp);
  outputString += "\n\t";
  outputString += stringPut(tn + fn);
  outputString += "\t";
  outputString += stringPut(fp + tp);

  outputString += "\n\tAccuracy [ (tp + tn) / testSize ]: ";
  outputString += stringPut(accuracy);

  outputString += "\n\tPrecision [ tp / (tp + fp) ]:      ";
  outputString += stringPut(precision);

  outputString += "\n\tSensitivity [ tp / (tp + fn) ]:    ";
  outputString += stringPut(sensitivity);

  outputString += "\n\tSpecificity [ tn / (tn + fp) ]:    ";
  outputString += stringPut(specificity);

  outputString += "\n\tF-Score [ 2tp / (2tp + fp + fn) ]: ";
  outputString += stringPut(f_score);

  outputString += "\n\tG-Score [ tp / sqrt((tp+fp)(tp+fn)]: ";
  outputString += stringPut(g_score);

  outputString += "\n";
  Logger::write(outputString);

}
