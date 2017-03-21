#include "neuralpso.h"

#include "PSO/pso.cpp"
//#include "NeuralNet/NeuralNet.cpp"

volatile bool stopProcessing = false;

NeuralPso::NeuralPso(PsoParams pp, NeuralNetParameters np) :
  Pso(pp),
  _neuralNet(new NeuralNet(np)),
  _images(nullptr),
  _labels(nullptr),
  _input(nullptr),
  _output(nullptr)
{
  resetProcess();
}

NeuralPso::~NeuralPso() {
  delete _neuralNet;
}

void NeuralPso::buildPso() {
  _gb._fit_pb = numeric_limits<double>::max();

  _particles.empty();
  vector<vector<vector<double>>> &edges = _neuralNet->getWeights();

  // Create N particles
  _particles.resize(_psoParams.particles);
  for (uint i = 0; i < _psoParams.particles; i++) {
    // Create the number of inner columns
    _particles[i]._fit_pb = numeric_limits<double>::max();
    _particles[i]._fit_lb = numeric_limits<double>::max();

    _particles[i]._x.resize(edges.size());
    _particles[i]._v.resize(edges.size());
    _particles[i]._x_pb.resize(edges.size());
    _particles[i]._x_lb.resize(edges.size());

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

  buildPso();
}

void NeuralPso::fly() {

  double velSum = 0;
  bool term = true;

  if (_particles.size() == 0) return;
  if (_particles[0]._v.size() == 0) return;

  static int innerNetAccessCount = _psoParams.iterationsPerLevel;
  static uint innerNetIt = _particles[0]._v.size()-1;

  // For each particle
  for (uint i = 0; i < _particles.size(); i++) {
    double C1 = 2.495, C2 = 2.495;
    Particle<vector<vector<vector<double>>>> *p = &_particles[i];

    // For each inner net
    for (uint inner_net = 0; inner_net < p->_v.size(); inner_net++) {
      if (_psoParams.backPropagation && (inner_net != innerNetIt)) continue;
      // For each edge (left side) of that inner net
      for (uint left_edge = 0; left_edge < p->_v[inner_net].size(); left_edge++) {
        // For each edge (right side) of that inner net
        for (uint right_edge = 0; right_edge < p->_v[inner_net][left_edge].size(); right_edge++) {
          double *w_v = &p->_v[inner_net][left_edge][right_edge];
          double *w_x = &p->_x[inner_net][left_edge][right_edge];
          double *w_pb = &p->_x_pb[inner_net][left_edge][right_edge];
          double *w_lb = &p->_x_lb[inner_net][left_edge][right_edge];

          double inertia = ((double) ((rand() % 50000) + 50000)) / 100000.0;
          double c1 = C1 * ((double) (rand() % 10000)) / 10000.0;
          double c2 = C2 * ((double) (rand() % 10000)) / 10000.0;

          *w_v += (inertia * (*w_v) + (c1*(*w_pb - *w_x)) + (c2*(*w_lb - *w_x))) / 100.0;
          velSum += *w_v;
          term = term && (*w_pb == *w_x) && (*w_lb == *w_x);
          *w_x += *w_v;

          if (*w_x > 1.0) {
            *w_x = 1.0;
            *w_v *= -0.01;
          } else if (*w_x < -1.0) {
            *w_x = -1.0;
            *w_v *= -0.01;
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
  double correctRatio;
  int totalCount;
  for (uint i = 0; i < _particles.size(); i++) {
    Particle<vector<vector<vector<double>>>> *p = &_particles[i];

    if (!_neuralNet->setWeights(&p->_x)) {
      std::cout<< "Failure to set weights." << endl;
    }
    double tempCorrectRatio;
    uint tempTotalCount;
    double sqrErr = testRun(tempCorrectRatio, tempTotalCount);

    // Minimize error
    double fit = sqrErr;

    // Find personal best
    if (fit < p->_fit_pb) {
      p->_fit_pb = fit;

      for (uint i = 0; i < p->_x_pb.size(); i++) {
        for (uint j = 0; j < p->_x_pb[i].size(); j++) {
          for (uint k = 0; k < p->_x_pb[i][j].size(); k++) {
            p->_x_pb[i][j][k] = p->_x[i][j][k];
          }
        }
      }
    } // end personal best

    // Find global best
    if (fit < _gb._fit_pb) {
      _gb._fit_pb = fit;
      //if (fit > 0.005) _overideTermFlag = false;
      //else _overideTermFlag = true;

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
      if (fit < p_n->_fit_lb) {
        p_n->_fit_lb = fit;

//        uint size1 = p_n->_x_lb.size();
        for (uint i = 0; i < p_n->_x_lb.size(); i++) {
          uint size2 = p_n->_x_lb[i].size();
          for (uint j = 0; j < p_n->_x_lb[i].size(); j++) {
//            uint size3 = p_n->_x_lb[i][j].size();
            for (uint k = 0; k < p_n->_x_lb[i][j].size(); j++) {
              if (j >= size2) continue;
              p_n->_x_lb[i][j][k] = p->_x[i][j][k];
            }
          }
        }
      }
    } // End local best

    //cout << "Particle (" << i << "):: Fit: " << fit << "\tPersonal: " << p->_fit_pb << "\tLocal: " << p->_fit_lb << "\tGlobal: " << gb()->_fit_pb << endl;

  } // end for each particle

  static double prevBest = 0;
  if (prevBest != gb()->_fit_pb) {
      cout << "Global: " << gb()->_fit_pb;
      cout << " - Correct: " << correctRatio * 100.0 << "% of " <<
        totalCount << " tests" << endl;
      prevBest = gb()->_fit_pb;
  }


  // Debugging test prints
  //printGB();
/*
  for (uint i = 0; i < _particles.size(); i++) {
    printParticlePBest(i);
    cout << endl;
  }
  cout << endl;
  */

  if (totalCount > 400 && correctRatio > 0.9995)
    interruptProcess();

  /**
  if (1.0-abs(gb()->_fit_pb) < _psoParams.delta)
    interruptProcess();
    **/

  if (checkForPrint()) {
    testGB();
  }
} // end getCost()

void NeuralPso::testGB() {
  _neuralNet->setWeights(&_gb._x);
  int I = randomizeTestInputs();

  vector<double> res = _neuralNet->process();
  cout << "X AND Y = ?? " << endl;
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
}

double NeuralPso::testRun(double &correctRatio, uint &totalCount) {
  double errSqr = 0;
  double answerFailPenalty = 1.0;
  uint outputNodes = _neuralNet->nParams()->outputs;

  uint totalSetsToRun = _neuralNet->nParams()->testIterations;
  uint correctCount = totalSetsToRun;

  vector<bool> *scannedOnce = new vector<bool>();
  vector<int> *resultCount = new vector<int>();
  scannedOnce->resize(outputNodes);
  resultCount->resize(outputNodes);
  for (uint it = 0; it < outputNodes; it++) {
    (*scannedOnce)[it] = false;
    (*resultCount)[it] = 0;
  }

  vector<double> *err = new vector<double>();
  vector<int> *answer = new vector<int>();
  err->resize(totalSetsToRun);
  answer->resize(totalSetsToRun);
  for (uint im = 0; im < totalSetsToRun; im++) {
    (*err)[im] = 0;
    (*answer)[im] = 0;
  }

  // First, test each output and store to the vector of results;
  for (uint someSets = 0; someSets < totalSetsToRun; someSets++) {
    // Set a random input
    int I = randomizeTestInputs();
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
      //errSqr += pow(expectedOutput[i] - output[i], 2);
      // Just try Gaussian???
      double expected = expectedOutput[i];
      double got = output[i];
      if (output[i] > maxVal) {
        maxVal = output[i];
        maxNode = i;
      }
      err->at(someSets) = expectedOutput[i] - output[i];
      //errSqr += ( -exp(-pow((expectedOutput[i] - output[i])/0.2, 2)) / (double) outputSize );
    }

    // If we have a correct answer, then we're heading in the right direction
    // Give him a cookie.
    if (maxNode != answer->at(someSets)) {
      //errSqr *= 0.1;
      answerFailPenalty *= 0.1;
      correctCount--;
    } else {
//      (*resultCount)[(*answer)[someSets]]++;
      scannedOnce->at(answer->at(someSets)) = true;
    }
    (*resultCount)[(*answer)[someSets]]++;

    if ((totalSetsToRun == correctCount) && (someSets == (totalSetsToRun - 1))) {
      if (totalSetsToRun < _input->size()) {
        correctCount = ++totalSetsToRun;
        err->push_back(0);
        answer->push_back(0);
      }
    }

  }

  for (uint i = 0; i < totalSetsToRun; i++) {
    double divisor = (double) (*resultCount)[(*answer)[i]]+1;
    //divisor = divisor != 0 ? divisor : 1;
    errSqr += ( -exp(-pow((abs(err->at(i)))/0.2, 2)) / divisor );
    //errSqr += ( -exp(-pow((abs(err->at(i)))/0.2, 2)) * ((1 - ((double) (*resultCount)[(*answer)[i]])/(double)totalSetsToRun)));
  }
  //errSqr *= answerFailPenalty;


  ///TODO: Temporary ending check
  //   Not necessarily correct if right 20 times in a row
  if (correctCount == totalSetsToRun) {
    bool scannedAtLeastOnce = true;
    for (uint i = 0; i < scannedOnce->size(); i++) {
      scannedAtLeastOnce &= (*scannedOnce)[i];
    }
    if (scannedAtLeastOnce) {
      //interruptProcess();
    }
  }

  // return mean sqr error
  //return sqrt(errSqr) / sqrt(totalSetsToRun);
  delete scannedOnce;
  delete resultCount;
  delete err;
  delete answer;

  correctRatio = (double) correctCount / (double) totalSetsToRun;
  totalCount = totalSetsToRun;

  return -((double) correctCount);

  return errSqr;// / (totalSetsToRun);
}

int NeuralPso::randomizeTestInputs() {
  _neuralNet->resetInputs();

  int I = 0;

  if (_input == nullptr) {
    I = rand() % _images->size();
    int N = (*_images)[I].size();

    for (uint i = 0; i < (*_images)[I].size(); i++) {
      for (uint j = 0; j < (*_images)[I][0].size(); j++) {
        byte socrates = (*_images)[I][i][j];
        uint plato = N*i + j;
        _neuralNet->loadInput(((double) socrates) / 255.0, plato);
      }
    }
  } else {
    I = rand() % _input->size();
    _neuralNet->loadInput((*_input)[I][0], 0);
    _neuralNet->loadInput((*_input)[I][1], 1);
  }

  return I;
}

void NeuralPso::loadTestInput(uint I) {
  _neuralNet->resetInputs();

  if (_input == nullptr) {
    int N = (*_images)[I].size();

    for (uint i = 0; i < (*_images)[I].size(); i++) {
      for (uint j = 0; j < (*_images)[I][0].size(); j++) {
        byte socrates = (*_images)[I][i][j];
        uint plato = N*i + j;
        _neuralNet->loadInput(((double) socrates) / 255.0, plato);
      }
    }
  } else {
    _neuralNet->loadInput((*_input)[I][0], 0);
    _neuralNet->loadInput((*_input)[I][1], 1);
  }
}

void NeuralPso::runTrainer() {

  run();

  printGB();

  _neuralNet->setWeights(&gb()->_x);
}

void NeuralPso::printGB() {
  cout << "Global Best: " << endl;
  for (uint i = 0; i < _gb._x.size(); i++) {
    cout << "  Inner Net " << i+1 << endl;
    for (uint j = 0; j < gb()->_x[i].size(); j++) {
      for (uint k = 0; k < gb()->_x[i][j].size(); k++) {
        cout << "  -- " << j+1 << " : " << k+1 << " = " << gb()->_x[i][j][k] << endl;
      }
    }
  }
}

void NeuralPso::printParticle(uint I) {
  if (I > _particles.size()) return;

  cout << "Particle (" << I << "): " << endl;
  for (uint i = 0; i < _particles[I]._x.size(); i++) {
    cout << "  Inner Net " << i+1 << endl;
    for (uint j = 0; j < _particles[I]._x[i].size(); j++) {
      for (uint k = 0; k < _particles[I]._x[i][j].size(); k++) {
        cout << "  -- " << j+1 << " : " << k+1 << " = " << _particles[I]._x[i][j][k] << endl;
      }
    }
  }
}

void NeuralPso::printParticlePBest(uint I) {
  if (I > _particles.size()) return;

  cout << "Particle pBest (" << I << "): " << endl;
  for (uint i = 0; i < _particles[I]._x_pb.size(); i++) {
    cout << "  Inner Net " << i+1 << endl;
    for (uint j = 0; j < _particles[I]._x_pb[i].size(); j++) {
      for (uint k = 0; k < _particles[I]._x_pb[i][j].size(); k++) {
        cout << "  -- " << j+1 << " : " << k+1 << " = " << _particles[I]._x_pb[i][j][k] << endl;
      }
    }
  }
}

void NeuralPso::printParticleLBest(uint I) {
  if (I > _particles.size()) return;

  cout << "Particle lBest (" << I << "): " << endl;
  for (uint i = 0; i < _particles[I]._x_lb.size(); i++) {
    cout << "  Inner Net " << i+1 << endl;
    for (uint j = 0; j < _particles[I]._x_lb[i].size(); j++) {
      for (uint k = 0; k < _particles[I]._x_lb[i][j].size(); k++) {
        cout << "  -- " << j+1 << " : " << k+1 << " = " << _particles[I]._x_lb[i][j][k] << endl;
      }
    }
  }
}
