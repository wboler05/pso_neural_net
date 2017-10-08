#include "NeuralNet.h"

NeuralNet::NeuralNet(const NeuralNetParameters & p) :
  _nParams(p)
{
    initialize(p);
}

NeuralNet::NeuralNet(const NeuralNetParameters &p, const CombEdgeType & n) :
    _nParams(p)
{
    initialize(p);
    setCombinedWeights(n);
}

NeuralNet::NeuralNet(const NeuralNet &n) {
    _inputNodes = n._inputNodes;
    _innerNodes = n._innerNodes;
    _outputNodes = n._outputNodes;
    _edges = n._edges;
    _localEdgesFlag = n._localEdgesFlag;
    _nParams = n._nParams;
}

NeuralNet::NeuralNet(NeuralNet && n) :
    _inputNodes(n._inputNodes),
    _innerNodes(n._innerNodes),
    _outputNodes(n._outputNodes),
    _edges(n._edges),
    _localEdgesFlag(n._localEdgesFlag),
    _nParams(n._nParams)
{
}

NeuralNet & NeuralNet::operator=(const NeuralNet & n) {
    _inputNodes = n._inputNodes;
    _innerNodes = n._innerNodes;
    _outputNodes = n._outputNodes;
    _edges = n._edges;
    _localEdgesFlag = n._localEdgesFlag;
    _nParams = n._nParams;
    return *this;
}

NeuralNet & NeuralNet::operator=(NeuralNet && n) {
    if (this != &n) {
        _inputNodes = n._inputNodes;
        _innerNodes = n._innerNodes;
        _outputNodes = n._outputNodes;
        _edges = n._edges;
        _localEdgesFlag = n._localEdgesFlag;
        _nParams = n._nParams;
    }
    return *this;
}

NeuralNet::~NeuralNet() {
}

void NeuralNet::initialize(const NeuralNetParameters & p) {
    _nParams = p;

    setTotalInputs(_nParams.inputs);
    setTotalInnerNets(_nParams.innerNets);
    for (int i = 0; i < _nParams.innerNets; i++) {
      setInnerNetNodes(_nParams.innerNetNodes[i], i);
    }
    setTotalOutputs(_nParams.outputs);

    buildNets();

    resetInputs();
    resetWeights();
}

void NeuralNet::resetInputs() {

  for (uint i = 0; i < _inputNodes.size(); i++) {
    _inputNodes[i] = 0;
  }

  for (uint i = 0; i < _innerNodes.size(); i++) {
    for (uint j = 0; j < _innerNodes[i].size(); j++) {
      _innerNodes[i][j] = 0;
    }
  }

  for (uint i = 0; i < _outputNodes.size(); i++) {
    _outputNodes[i] = 0;
  }
}

void NeuralNet::resetWeights() {
  for (uint i = 0; i < _edges.size(); i++) {
    for (uint j = 0; j < _edges[i].size(); j++) {
      for (uint k = 0; k < _edges[i][j].size(); k++) {
        _edges[i][j][k] = 0;
      }
    }
  }
}

void NeuralNet::setTotalInputs(uint n) {
    _inputNodes.clear();
    _inputNodes.resize(n, 0);
}

// Clears all nodes
/// Need to call setInnerNetNodes() after
void NeuralNet::setTotalInnerNets(uint n) {
    _innerNodes.clear();
    _innerNodes.resize(n);
    _edges.clear();
    _edges.resize(n+1);
}
void NeuralNet::setInnerNetNodes(uint nodes, uint i) {
    if (i >= 0 && i < _innerNodes.size()) {
        _innerNodes[i].clear();
        _innerNodes[i].resize(nodes, 0);
    }
}

void NeuralNet::setTotalOutputs(uint n) {
    _outputNodes.clear();
    _outputNodes.resize(n, 0);
}


bool NeuralNet::buildNets() {
  // Verify nodes have been built
  if (_inputNodes.size() == 0 || _innerNodes.size() == 0 || _outputNodes.size() == 0) {
    return false;
  }
  // Verify at least one inner net is built
  if (_innerNodes[0].size() == 0) {
    return false;
  }

  //std::uniform_real_distribution<real> dist(-1,1);

  // Connect the input to the inner nodes
  for (uint i = 0; i < _edges.size(); i++) {
    if (i == 0) { // Input to inner
      _edges[i].clear();
      _edges[i].resize(_inputNodes.size());
      for (uint j = 0; j < _inputNodes.size(); j++) {
        _edges[i][j].clear();
        _edges[i][j].resize(_innerNodes[0].size());
      }
    } else if (i == _edges.size()-1) { // inner to output
      _edges[i].clear();
      _edges[i].resize(_innerNodes[i-1].size());
      for (uint j = 0; j < _innerNodes[i-1].size(); j++) {
        _edges[i][j].clear();
        _edges[i][j].resize(_outputNodes.size());
      }
    } else {
      _edges[i].clear();
      _edges[i].resize(_innerNodes[i-1].size());
      for (uint j = 0; j < _innerNodes[i-1].size(); j++) {
        _edges[i][j].clear();
        _edges[i][j].resize(_innerNodes[i].size());
      }
    }
  }

  for (uint i = 0; i < _edges.size(); i++) {
    for (uint j = 0; j < _edges[i].size(); j++) {
      for (uint k = 0; k < _edges[i][j].size(); k++) {
        _edges[i][j][k] = (real) (rand() % 10000-5000) / 10000.0;
        //_edges[i][j][k] = dist(gen);
      }
    }
  }

  _recEdges.resize(_innerNodes.size());
  _recBuffer.resize(_innerNodes.size());
  for (size_t i = 0; i < _innerNodes.size(); i++) {
      _recEdges[i].resize(_innerNodes[i].size());
      _recBuffer[i].resize(_innerNodes[i].size());
      for (size_t j = 0; j < _innerNodes[i].size(); j++) {
          //_recEdges[i][j] = dist(gen);
          _recEdges[i][j] = (real) (rand() % 10000-5000) / 10000.0;
          _recBuffer[i][j] = 0;
      }
  }

  return true;
}

void NeuralNet::loadInput(real in, uint i) {
  if (i >= 0 && i < _inputNodes.capacity()) {
    _inputNodes[i] = in;
  }
}

real NeuralNet::activation(real in) {
  //real act = in / (1 + abs(in));  // Softsign
  //real act = 1 / (1 + exp(-in));  // Logistics
//  real mean = 1.0;
  //real act = exp(-pow((in - mean)/sigma, 2));      // Gaussian
  //real act = 4 * in * in - 4 * in + 1;

//    real sigma = 0.35;
//    real act = getSign(in) * (1 - exp(-pow(in / sigma, 2)));


    /**
    // 9th order approximation to tanh
    static real coeffs[10] = {
        4.42727699125780,
        -2.78592124641418e-14,
        -12.3878821958288,
        4.49018884445568e-14,
        13.4092500380177,
        -2.26603901811258e-14,
        -7.48382418797760,
        3.72216702500625e-15,
        3.04221199452273,
        -5.55537460355953e-17
    };

    //real act = tanh(in * M_PI);
    // Faster than tanh function
    real act = CustomMath::poly(in * M_PI, coeffs, 9);
    act = max(min(act, (real)1.0), (real)-1);
    **/


    // 9th order approximation of logistic sigmoid
    static real coeffs[10] = {
        0.938400463413615,
        -1.32105382144212e-14,
        -2.77909962352499,
        2.29080010982935e-14,
        3.32023231311074,
        -1.29358310591960e-14,
        -2.21971364932927,
        2.63988723756627e-15,
        1.23541839801387,
        0.500000000000000
    };
    real act = CustomMath::poly(in, coeffs, 9);
    act = max(min(act, (real)1.0), (real) 0);


  return act;
}

real NeuralNet::getSign(const real &in) {
    if (in < 0) {
        return -1;
    } else {
        return 1;
    }
}

const vector<real> & NeuralNet::process() {

  // If the edges aren't built, it's broke
  if (_edges.size() == 0)
    return _outputNodes;

//  cout << _inputNodes.size() << "\t" << _innerNodes.size();

  if (_nParams.type == Recurrent) {
      for (uint i = 0; i < _innerNodes.size(); i++) {
          for (uint j = 0; j < _innerNodes[i].size(); j++) {
              _recBuffer[i][j] = _recEdges[i][j] * activation(_innerNodes[i][j]);
          }
      }
      for (uint i = 0; i < _innerNodes.size(); i++) {
          for (uint j = 0; j < _innerNodes[i].size(); j++) {
              _innerNodes[i][j] = _recBuffer[i][j];
          }
      }
  } else if (_nParams.type == Feedforward){
      resetInnerNodes();
  }


  // Handle the input to the inner
  for (uint i = 0; i < _inputNodes.size(); i++) {
    for (uint j = 0; j < _innerNodes[0].size(); j++) {
      _innerNodes[0][j] += _inputNodes[i] * _edges[0][i][j];;
    }
  }

    // Handle the rest of the inner nodes
  for (uint i = 0; i < _innerNodes.size()-1; i++) {
    for (uint j = 0; j < _innerNodes[i].size(); j ++) {
      for (uint k = 0; k < _innerNodes[i+1].size(); k++) {
        _innerNodes[i+1][k] += _edges[i+1][j][k] * activation(_innerNodes[i][j]);
      }
    }
  }

  // Handle the inner nodes to the output
  for (uint i = 0; i < _outputNodes.size(); i++) {
    uint innerIndex = _innerNodes.size() - 1 ;
    uint innerSize = _innerNodes[innerIndex].size();
    for (uint j = 0; j < innerSize; j++) {
      _outputNodes[i] += _edges[_edges.size()-1][j][i] * activation(_innerNodes[innerIndex][j]);
    }
  }

  return _outputNodes;
}

void NeuralNet::resetInnerNodes() {
  for (uint i = 0; i < _innerNodes.size(); i++) {
    for (uint j = 0; j < _innerNodes[i].size(); j++) {
      _innerNodes[i][j] = 0;
    }
  }

  for (uint i = 0; i < _outputNodes.size(); i++) {
    _outputNodes[i] = 0;
  }
}

/**
 * @brief NeuralNet::setWeights
 * @param w
 * @return
 * @todo Change it so that it can get a pointer to weights.
 */
bool NeuralNet::setWeights(const EdgeType & w) {
  //cout << "Edges: " << _edges.size() << " Setting: " << w->size() << endl;
  if (_edges.size() != w.size()) return false;
  for (uint i = 0; i < w.size(); i++) {
    if (w[i].size() != _edges[i].size()) return false;
    for (uint j = 0; j < w[i].size(); j++) {
      if (w[i][j].size() != _edges[i][j].size()) return false;
      for (uint k = 0; k < w[i][j].size(); k++) {
        _edges[i][j][k] = w[i][j][k];
 //       cout << i << ": " << j << ", " << k << ": " << _edges[i][j][k];
 //       cout << endl;
      }
    }
  }
  return true;
}

bool NeuralNet::setRecWeights(const RecEdgeType & w) {
    if (_recEdges.size() != w.size()) {
        return false;
    }
    for (uint i = 0; i < _recEdges.size(); i++) {
        for (uint j = 0; j < _recEdges[i].size(); j++) {
            _recEdges[i][j] = w[i][j];
        }
    }
    return true;
}

bool NeuralNet::setCombinedWeights(const EdgeType & w) {
    if (_edges.size() == 0) return false;
    if (_edges.size() + 1 != w.size()) {
        return false;
    }
    if (_recEdges.size() != w.at(w.size()-1).size()) {
        return false;
    }

    for (uint i = 0; i < _edges.size(); i++) {
        if (w[i].size() != _edges[i].size()) {
            return false;
        }
        for (uint j = 0; j < _edges[i].size(); j++) {
            if (w[i][j].size() != _edges[i][j].size()) {
                return false;
            }
            for (uint k = 0; k < _edges[i][j].size(); k++) {
                _edges[i][j][k] = w[i][j][k];
            }
        }
    }

    int it = w.size() - 1;
    for (uint j = 0; j < w[it].size(); j++) {
        for (uint k = 0; k < w[it][j].size(); k++) {

            _recEdges[j][k] = w[it][j][k];
        }
    }
    return true;
}

bool NeuralNet::splitCombinedWeights(const CombEdgeType &c, EdgeType & e, RecEdgeType & r) {
    if (c.size() <= 1) {
        return false;
    }

    r = c[c.size()-1];
    e = c;
    e.pop_back();

    return true;
}

void NeuralNet::printEdges() {
  cout << "An edge: " << endl;
  for (uint i = 0; i < _edges.size(); i++) {
    cout << "  Inner Net " << i+1 << endl;
    for (uint j = 0; j < _edges[i].size(); j++) {
      for (uint k = 0; k < _edges[i][j].size(); k++) {
        cout << "  -- " << j+1 << " : " << k+1 << " = " << _edges[i][j][k] << endl;
      }
    }
  }
}

