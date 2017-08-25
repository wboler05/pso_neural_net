#include "NeuralNet.h"

NeuralNet::NeuralNet(const NeuralNetParameters & p) :
  _nParams(p)
{
    initialize(p);
}

NeuralNet::NeuralNet(const NeuralNetParameters &p, const EdgeType & n) :
    _nParams(p)
{
    initialize(p);
    setWeights(n);
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
  if (n != _inputNodes.capacity()) {
    _inputNodes.clear();
    _inputNodes.resize(n);
    for (uint i = 0; i < n; i++) {
      _inputNodes[i] = 0;
    }
    //buildNets();
  }
}

// Clears all nodes
/// Need to call setInnerNetNodes() after
void NeuralNet::setTotalInnerNets(uint n) {
  if (n != _innerNodes.capacity()) {
    _innerNodes.clear();
    _innerNodes.resize(n);
    _edges.clear();
    _edges.resize(n+1);
  }
}
void NeuralNet::setInnerNetNodes(uint nodes, uint i) {
  if (i >= 0 && i < _innerNodes.size()) {
    if (_innerNodes[i].capacity() != nodes) {
      _innerNodes[i].clear();
      _innerNodes[i].resize(nodes);
      for (uint j = 0; j < nodes; j++) {
        _innerNodes[i][j] = 0;
      }
    }
  }
}

void NeuralNet::setTotalOutputs(uint n) {
  if (n != _outputNodes.capacity()) {
    _outputNodes.clear();
    _outputNodes.resize(n);
    for (uint i = 0; i < n; i++) {
      _outputNodes[i] = 0;
    }
  }
}


bool NeuralNet::buildNets() {
  // Verify nodes have been built
  if (_inputNodes.capacity() == 0 || _innerNodes.capacity() == 0 || _outputNodes.capacity() == 0) {
    return false;
  }
  // Verify at least one inner net is built
  if (_innerNodes[0].capacity() == 0) {
    return false;
  }

  // Connect the input to the inner nodes
  for (uint i = 0; i < _edges.capacity(); i++) {
    if (i == 0) { // Input to inner
      _edges[i].clear();
      _edges[i].resize(_inputNodes.capacity());
      for (uint j = 0; j < _inputNodes.capacity(); j++) {
        _edges[i][j].clear();
        _edges[i][j].resize(_innerNodes[i].capacity());
      }
    } else if (i == _edges.capacity()-1) { // inner to output
      _edges[i].clear();
      _edges[i].resize(_innerNodes[i-1].capacity());
      for (uint j = 0; j < _innerNodes[i-1].capacity(); j++) {
        _edges[i][j].clear();
        _edges[i][j].resize(_outputNodes.capacity());
      }
    } else {
      _edges[i].clear();
      _edges[i].resize(_innerNodes[i-1].capacity());
      for (uint j = 0; j < _innerNodes[i-1].capacity(); j++) {
        _edges[i][j].clear();
        _edges[i][j].resize(_innerNodes[i].capacity());
      }
    }
  }

  for (uint i = 0; i < _edges.capacity(); i++) {
    for (uint j = 0; j < _edges[i].capacity(); j++) {
      for (uint k = 0; k < _edges[i][j].capacity(); k++) {
        _edges[i][j][k] = (double) (rand() % 10000) / 10000.0;
      }
    }
  }

  return true;
}

void NeuralNet::loadInput(double in, uint i) {
  if (i >= 0 && i < _inputNodes.capacity()) {
    _inputNodes[i] = in;
  }
}

double NeuralNet::activation(double in) {
  //double act = in / (1 + abs(in));  // Softsign
  //double act = 1 / (1 + exp(-in));  // Logistics
//  double mean = 1.0;
  //double act = exp(-pow((in - mean)/sigma, 2));      // Gaussian
  //double act = 4 * in * in - 4 * in + 1;

//    double sigma = 0.35;
//    double act = getSign(in) * (1 - exp(-pow(in / sigma, 2)));


    /**
    // 9th order approximation to tanh
    static double coeffs[10] = {
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

    //double act = tanh(in * M_PI);
    // Faster than tanh function
    double act = CustomMath::poly(in * M_PI, coeffs, 9);
    act = max(min(act, (double)1.0), (double)-1);
    **/


    // 9th order approximation of logistic sigmoid
    static double coeffs[10] = {
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
    double act = CustomMath::poly(in, coeffs, 9);
    act = max(min(act, (double)1.0), (double) 0);


  return act;
}

double NeuralNet::getSign(const double &in) {
    if (in < 0) {
        return -1;
    } else {
        return 1;
    }
}

const vector<double> & NeuralNet::process() {

  resetInnerNodes();

  // If the edges aren't built, it's broke
  if (_edges.capacity() == 0)
    return _outputNodes;

//  cout << _inputNodes.size() << "\t" << _innerNodes.size();

  // Handle the input to the inner
  for (uint i = 0; i < _inputNodes.size(); i++) {
    for (uint j = 0; j < _innerNodes[0].size(); j++) {
      double w = _inputNodes[i] * _edges[0][i][j];
      _innerNodes[0][j] += w / _inputNodes.size();
    }
  }

/*
  cout << "Inputs (" << _inputNodes.size() << "): ";
  for (uint i = 0; i < _inputNodes.size(); i++) {
    cout << _inputNodes[i] << ", ";
  }
  cout << "\n" << endl;

  cout << "Stage 1 Inner Node (" << _innerNodes[0].size() << "): ";
  for (uint i = 0; i < _innerNodes[0].size(); i++) {
    cout << _innerNodes[0][i] << ", ";
  }
  cout << "\n" << endl;
*/

  // Handle the rest of the inner nodes
  for (uint i = 0; i < _innerNodes.size()-1; i++) {
    for (uint j = 0; j < _innerNodes[i].size(); j ++) {
      for (uint k = 0; k < _innerNodes[i+1].size(); k++) {
        _innerNodes[i+1][k] += _edges[i+1][j][k] * activation(_innerNodes[i][j]) / _innerNodes[i].size();
      }
    }
  }

/*
    cout << "Stage " << i+1 << " Inner Node (" << _innerNodes[i+1].size() << "): ";
    for (uint f = 0; f < _innerNodes[i+1].size(); f++) {
      cout << _innerNodes[i+1][f] << ", ";
    }
    cout << "\n" << endl;


  }
*/
  // Handle the inner nodes to the output
  double maxVal = 0;
  for (uint i = 0; i < _outputNodes.size(); i++) {
    uint innerIndex = _innerNodes.size() - 1 ;
    uint innerSize = _innerNodes[innerIndex].size();
    for (uint j = 0; j < innerSize; j++) {
//        double wuz = _innerNodes[innerIndex][j];
      _outputNodes[i] += _edges[_edges.size()-1][j][i] * activation(_innerNodes[innerIndex][j]) / innerSize;
    }
    //_outputNodes[i] = abs(_outputNodes[i]);
//    _outputNodes[i] = (_outputNodes[i] / 2.0) + 0.5;
//    if (_outputNodes[i] > maxVal) {
//      maxVal = _outputNodes[i];
//    }
  }

//  if (maxVal != 0) {
//    for (uint i = 0; i < _outputNodes.size(); i++) {
//      _outputNodes[i] /= maxVal;
//    }
//  }

/*
  cout << "Output (" << _outputNodes.size() << "): ";
  for (uint i = 0; i < _outputNodes.size(); i++) {
    cout << _outputNodes[i] << ", ";
  }
  cout << "\n" << endl;
*/

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

