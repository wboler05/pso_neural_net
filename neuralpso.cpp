#include "neuralpso.h"

#include "PSO/pso.cpp"
//#include "NeuralNet/NeuralNet.cpp"

NeuralPso::NeuralPso(PsoParams pp, NeuralNetParameters np) :
  Pso(pp),
  _neuralNet(new NeuralNet(np))
{

}

NeuralPso::~NeuralPso() {
  delete _neuralNet;
}

void NeuralPso::build(vector<vector<vector<byte> > > &images, vector<byte> &labels) {
  srand(time(NULL));
  _images = &images;
  _labels = &labels;

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
    // Create each inner column edge
    for (uint j = 0; j < edges.size(); j++) {
      // Create left edges
      _particles[i]._x[j].resize(edges[j].size());
      _particles[i]._v[j].resize(edges[j].size());
      _particles[i]._x_pb[j].resize(edges[j].size());
      _particles[i]._x_lb[j].resize(edges[j].size());
      for (uint k = 0; k < edges[j].size(); k++) {
        // Create right edges
        _particles[i]._x[j][k].resize(edges[j][k].size());
        _particles[i]._v[j][k].resize(edges[j][k].size());
        _particles[i]._x_pb[j][k].resize(edges[j][k].size());
        _particles[i]._x_lb[j][k].resize(edges[j][k].size());
        for (uint m = 0; m < edges[j][k].size(); m++) {
          _particles[i]._x[j][k][m] = (double) (rand() % 10000) / 10000;
          _particles[i]._v[j][k][m] = 0;
          _particles[i]._x_pb[j][k][m] = 0;
          _particles[i]._x_lb[j][k][m] = 0;
        }
      }
    }
  }


}

void NeuralPso::fly() {
  // For each particle
  for (uint i = 0; i < _particles.size(); i++) {
    double C1 = 2.495, C2 = 2.495;
    Particle<vector<vector<vector<double>>>> *p = &_particles[i];

    // For each inner net
    for (uint inner_net = 0; inner_net < p->_v.size(); inner_net++) {
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

          *w_v += inertia * (*w_v) + (c1*(*w_pb - *w_x)) + (c2*(*w_lb - *w_x));
          *w_x += *w_v / 6.0;

          if (*w_x > 1.0) *w_x = 1.0;
          else if (*w_x < 0) *w_x = 0;
        }
      }
    }
  }
}

void NeuralPso::getCost() {
  for (uint i = 0; i < _particles.size(); i++) {
    Particle<vector<vector<vector<double>>>> *p = &_particles[i];

    if (!_neuralNet->setWeights(&p->_x)) {
      std::cout<< "Failure to set weights." << endl;
    }
    double sqrErr = testRun();

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
      if (fit > 0.005) _overideTermFlag = false;
      else _overideTermFlag = true;

      for (uint i = 0; i < _gb._x.size(); i++) {
        for (uint j = 0; j < _gb._x[i].size(); j++) {
          for (uint k = 0; k < _gb._x[k].size(); k++) {
            _gb._x[i][j][k] = p->_x[i][j][k];
          }
        }
      }
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

    cout << "Particle (" << i << "):: Fit: " << fit << "\tPersonal: " << p->_fit_pb << "\tLocal: " << p->_fit_lb << "\tGlobal: " << gb()->_fit_pb << endl;

  } // end for each particle

} // end getCost()

double NeuralPso::testRun() {
  double errSqr = 0;

  int runThemSets = 50;

  for (int someSets = 0; someSets < runThemSets; someSets++) {
    // Set a random input
    int I = randomizeTestInputs();

    // Get the result from random input
    vector<double> output = _neuralNet->process();
    vector<double> expectedOutput;

    int outputSize = output.size();
    int answer = _labels->at(I);

    expectedOutput.resize(outputSize);

    for (int i = 0; i < outputSize; i++) {
      if (i == answer) {
        expectedOutput[i] = 1;
      } else {
        expectedOutput[i] = 0;
      }
    }

    // Compare the output to expected
    for (int i = 0; i < outputSize; i++) {
      errSqr += pow(expectedOutput[i] - output[i], 2);
    }
  }

  // return mean sqr error
  return sqrt(errSqr) / sqrt(runThemSets);
}

int NeuralPso::randomizeTestInputs() {
  _neuralNet->resetInputs();

  int I = rand() % _images->size();
  int N = (*_images)[I].size();

  for (uint i = 0; i < (*_images)[I].size(); i++) {
    for (uint j = 0; j < (*_images)[I][0].size(); j++) {
      byte socrates = (*_images)[I][i][j];
      uint plato = N*i + j;
      _neuralNet->loadInput(((double) socrates) / 255.0, plato);
    }
  }

  return I;
}

void NeuralPso::runTrainer() {
  run();
}
