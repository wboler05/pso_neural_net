#include "NeuralNet.h"

//RandomNumberEngine NeuralNet::_randomEngine(
//        std::chrono::system_clock::now().time_since_epoch().count());

NeuralNet::NeuralNet(const NeuralNetParameters & p) :
  _nParams(p)
{
    initialize(p);
}

NeuralNet::NeuralNet(const NeuralNetParameters &p, const State &s) :
    _nParams(p)
{
    initialize(p);
    setState(s);
}

NeuralNet::NeuralNet(const NeuralNet &n) {
    _inputNodes = n._inputNodes;
    _innerNodes = n._innerNodes;
    _outputNodes = n._outputNodes;
    _state = n._state;
    _localEdgesFlag = n._localEdgesFlag;
    _nParams = n._nParams;
}

NeuralNet::NeuralNet(NeuralNet && n) :
    _inputNodes(n._inputNodes),
    _innerNodes(n._innerNodes),
    _outputNodes(n._outputNodes),
    _state(n._state),
    _localEdgesFlag(n._localEdgesFlag),
    _nParams(n._nParams)
{
}

NeuralNet & NeuralNet::operator=(const NeuralNet & n) {
    _inputNodes = n._inputNodes;
    _innerNodes = n._innerNodes;
    _outputNodes = n._outputNodes;
    _state = n._state;
    _localEdgesFlag = n._localEdgesFlag;
    _nParams = n._nParams;
    return *this;
}

NeuralNet & NeuralNet::operator=(NeuralNet && n) {
    if (this != &n) {
        _inputNodes = std::move(n._inputNodes);
        _innerNodes = std::move(n._innerNodes);
        _outputNodes = std::move(n._outputNodes);
        _state = std::move(n._state);
        _localEdgesFlag = std::move(n._localEdgesFlag);
        _nParams = std::move(n._nParams);
    }
    return *this;
}

NeuralNet::~NeuralNet() {
}

void NeuralNet::initialize(const NeuralNetParameters & p) {
    _nParams = p;

    setTotalInputs(static_cast<size_t>(_nParams.inputs));
    setTotalInnerNets(static_cast<size_t>(_nParams.innerNets));
    for (size_t i = 0; i < static_cast<size_t>(_nParams.innerNets); i++) {
      setInnerNetNodes(static_cast<size_t>(_nParams.innerNetNodes[i]), i);
    }
    setTotalOutputs(static_cast<size_t>(_nParams.outputs));

    buildANN();

    resetAllNodes();
    resetWeights();
}

void NeuralNet::resetAllNodes() {

    resetInputNodes();
    resetInnerNodes();
    resetRecurrentNodes();
    resetOutputNodes();
}

void NeuralNet::resetInputNodes() {
    for (size_t i = 0; i < _inputNodes.size(); i++) {
        _inputNodes[i] = 0;
    }
}

void NeuralNet::resetInnerNodes() {
    for (size_t i = 0; i < _innerNodes.size(); i++) {
        for (size_t j = 0; j < _innerNodes[i].size(); j++) {
            _innerNodes[i][j] = 0;
        }
    }
}

void NeuralNet::resetRecurrentNodes() {
    for (size_t i = 0; i < _recurrentNodes.size(); i++) {
        for (size_t j = 0; j < _recurrentNodes[i].size(); j++) {
            _recurrentNodes[i][j] = 0;
        }
    }
}

void NeuralNet::resetOutputNodes() {
    for (size_t i = 0; i < _outputNodes.size(); i++) {
        _outputNodes[i] = 0;
    }
}

void NeuralNet::resetWeights() {
    for (size_t i = 1; i < _state.size(); i++) {
        for (size_t j = 0; j < _state[i].size(); j++) {
            for (size_t k = 0; k < _state[i][j].size(); k++) {
                _state[i][j][k] = 0;
            }
        }
    }
}

void NeuralNet::setTotalInputs(const size_t & n) {
    _inputNodes.clear();
    _inputNodes.resize(n+1, 0);
    _inputNodes.back() = 1;
    _modifiedFlag = true;
}

// Clears all nodes
/// Need to call setInnerNetNodes() after
void NeuralNet::setTotalInnerNets(const size_t & n) {
    _innerNodes.clear();
    _innerNodes.resize(n);
    _recurrentNodes.clear();
    _recurrentNodes.resize(n);
    _modifiedFlag = true;
}
void NeuralNet::setInnerNetNodes(const size_t & nodes, const size_t & i) {
    if (i < static_cast<size_t>(_innerNodes.size())) {
        _innerNodes[i].clear();
        _innerNodes[i].resize(nodes, 0);
        _recurrentNodes[i].clear();
        _recurrentNodes[i].resize(nodes, 0);
        _modifiedFlag = true;
    }
}

void NeuralNet::setTotalOutputs(const size_t & n) {
    _outputNodes.clear();
    _outputNodes.resize(n, 0);
    _modifiedFlag = true;
}


bool NeuralNet::buildANN() {

    // Verify parameters appropriately set
    if (!validateParams()) {
        return false;
    }

    // Clear and resize State
    prepareState();

    // Build Topology
    buildTopology();

    // Build Edges
    buildEdges();

    return true;
}

bool NeuralNet::validateParams() {
    // Verify nodes have been built
    if (_inputNodes.size() == 0 || _innerNodes.size() == 0 || _outputNodes.size() == 0) {
        return false;
    }
    // Verify at least one inner net is built
    if (_innerNodes[0].size() == 0) {
        return false;
    }
    return true;
}

void NeuralNet::prepareState() {
    _state.clear();

    // Generate First Dimension of State
    // Topo(1) + (N+1 innerNodes) + (N-2 innerNodes) = 2N
    size_t totalElements = totalStateElementsFromInnerNodes(_innerNodes.size());
    _state.resize(totalElements);
}

size_t NeuralNet::totalStateElementsFromInnerNodes(const size_t & innerNodes) {
    return 2 * (innerNodes + 1);
}

size_t NeuralNet::totalEdgeLayersFromState(const State & state) {
    return (state.size() / 2) - 1;
}

void NeuralNet::buildTopology() {
    InnerNodes & topology = _state[0];
    size_t totalLayers = _innerNodes.size()+2;
    topology.resize(totalLayers);

    for (size_t i = 0; i < totalLayers; i++) {
        if (i == 0) {
            topology[i].resize(_inputNodes.size());
        } else if (i == totalLayers - 1) {
            topology[i].resize(_outputNodes.size());
        } else {
            topology[i].resize(_innerNodes[i-1].size());
        }
        for (size_t j = 0; j < topology[i].size(); j++) {
            topology[i][j] = 1.0;
        }
    }
}

bool NeuralNet::buildEdges() {
    bool goodBuild = true;
    goodBuild |= buildForwardEdges();
    goodBuild |= buildRecurrentEdges();
    return goodBuild;
}

bool NeuralNet::buildForwardEdges() {
    if (_inputNodes.size() == 0 || _outputNodes.size() == 0) {
        return false;
    }
    //std::uniform_real_distribution<real> dist(-1,1);

    size_t totalInnerLayers = _innerNodes.size()+1;

    for (size_t i = 0, stateIt=1; i < totalInnerLayers; i++, stateIt++) {
        if (i >= _state.size()) {
            return false;
        }
        if (i == 0) {
            // Left Input - Right Hidden0
            _state[stateIt].resize(_inputNodes.size());
            for (size_t left_node = 0; left_node < _state[stateIt].size(); left_node++) {
                if (_innerNodes.size() > 0) {
                    // At least one hidden layer
                    _state[stateIt][left_node].resize(_innerNodes[0].size());
                } else {
                    // Connect to output
                    _state[stateIt][left_node].resize(_outputNodes.size());
                }
            }
        } else if (i == totalInnerLayers-1) {
            // Left HiddenN - Right Output
            _state[stateIt].resize(_innerNodes[i-1].size());
            for (size_t left_node = 0; left_node < _state[stateIt].size(); left_node++) {
                _state[stateIt][left_node].resize(_outputNodes.size(), 0);
            }
        } else {
            // Inner Layer - There must be 2+ inner layers here
            _state[stateIt].resize(_innerNodes[i-1].size());
            for (size_t left_node = 0; left_node < _state[stateIt].size(); left_node++) {
                _state[stateIt][left_node].resize(_innerNodes[i].size(), 0);
            }
        }
    }
    return true;
}

bool NeuralNet::buildRecurrentEdges() {
    //std::uniform_real_distribution<real> dist(-1,1);

    // 0 to N-1 inner edges in layer connect to a r-node
    // Nth edge connects back to a node
    // Total N+1 edges per layer

    // Starts at (1 + L+1)th index
    size_t startIndex = 1 + _innerNodes.size()+1;
    size_t totalLayers = _innerNodes.size();

    qDebug() << "State: " << _state.size() << " elements: " << startIndex << " to " << startIndex + totalLayers;

    for (size_t layer = startIndex, i=0; layer < totalLayers+startIndex; layer++, i++) {
        if (layer > _state.size()) {
            return false;
        }
        _state[layer].resize(_innerNodes[i].size());
        for (size_t recNode = 0; recNode < _state[layer].size(); recNode++) {
            _state[layer][recNode].resize(_innerNodes[i].size()+1);
        }
    }
    return true;
}

void NeuralNet::enableAllNodes(const bool & t) {
    for (size_t i = 0; i < _state[0].size(); i++) {
        for (size_t j = 0; j < _state[0][i].size(); j++) {
            _state[0][i][j] = enableNodeBoolToValue(t);
        }
    }
}

real NeuralNet::enableNodeBoolToValue(const bool & t) {
    if (t) {
        return 1;
    } else {
        return -1;
    }
}

bool NeuralNet::enableNodeValueToBool(const real & val) {
    return val > 0;
}

real NeuralNet::nodeEnabled(const size_t & layer, const size_t & node) {
    if (_state.size() == 0) return 0;

    if (layer < _state[0].size()) {
        if (node < _state[0][layer].size()) {
            return enableNodeValueToBool(_state[0][layer][node]) ? 1 : 0;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

void NeuralNet::randomDropoutNodes(const real &mean=0, const real & sigma=1) {
    std::normal_distribution<real> dist(mean, sigma);
    //std::uniform_real_distribution<real> dist(-1, 1);

    for (size_t i = 0; i < _state[0].size(); i++) {
        for (size_t j = 0; j < _state[0][i].size(); j++) {
      //      _state[0][i][j] = dist(_randomEngine.engine());
        }
    }
}

NeuralNet::EdgeLayer * NeuralNet::forwardEdgeLayer(const size_t & layer) {
    if (layer < _innerNodes.size() + 1) {
        if (layer + 1 < _state.size()) {
            return &_state[layer+1];
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

NeuralNet::EdgeLayer * NeuralNet::recurrentEdgeLayer(const size_t & layer) {
    if (layer < _innerNodes.size()) {
        size_t layerIteration = _innerNodes.size() + 2 + layer;
        if (layerIteration < _state.size()) {
            return &_state[layerIteration];
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

void NeuralNet::loadInput(const real & in, const size_t & i) {
  if (i < _inputNodes.size()) {
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

    if (_modifiedFlag) {
        _modifiedFlag = false;
        buildANN();
    }

  // If the edges aren't built, it's broke
  if (_state.size() == 0)
    return _outputNodes;

//  cout << _inputNodes.size() << "\t" << _innerNodes.size();

  processForwardPropagation();

  return _outputNodes;
}

void NeuralNet::processRecurrentNodes() {
    if (_nParams.type == Recurrent) {
        // For each layer
        for (size_t layer = 0; layer < _innerNodes.size(); layer++) {
            // For each recurrent node
            for (size_t j = 0; j < _innerNodes[layer].size(); j++) {
                // For each connected node;
                for (size_t k = 0; k < _innerNodes[layer].size(); k++) {
                    _recurrentNodes[layer][j] +=
                            nodeEnabled(layer, k) *
                            (*recurrentEdgeLayer(layer))[j][k] *
                            activation(_innerNodes[layer][k]);
                }
            }
        }
    } else if (_nParams.type == Feedforward){
        resetNodesForRerun();
    }
}

void NeuralNet::processForwardPropagation() {
    if (_innerNodes.size() == 0) {
        resetOutputNodes();
        EdgeLayer * edges = forwardEdgeLayer(0);
        for (size_t left_node = 0; left_node < _inputNodes.size(); left_node++) {
            for (size_t right_node = 0; right_node < _outputNodes.size(); right_node++) {
                _outputNodes[right_node] += (*edges)[left_node][right_node]*_inputNodes[left_node];
            }
        }
        return;
    } else {
        resetInnerNodes();
        resetOutputNodes();

        if (_nParams.type == Recurrent) {
            //resetRecurrentNodes();
        }

        for (size_t layer = 0; layer < _innerNodes.size()+1; layer++) {
            EdgeLayer * edges = forwardEdgeLayer(layer);

            if (layer == 0) {
                // Process Input to Hidden
                for (size_t left_node = 0; left_node < _inputNodes.size(); left_node++) {
                    for (size_t right_node = 0; right_node < _innerNodes[0].size(); right_node++) {
                        if (enableNodeValueToBool(nodeEnabled(0, right_node))) {
                            _innerNodes[0][right_node] +=
                                    (*edges)[left_node][right_node]*_inputNodes[left_node];
                        }
                    }
                }
            } else if (layer == _innerNodes.size()) {
                // Hidden to Output
                for (size_t left_node = 0; left_node < _innerNodes[layer-1].size(); left_node++) {
                    if (!enableNodeValueToBool(nodeEnabled(layer-1, left_node))) {
                        continue;
                    }
                    for (size_t right_node = 0; right_node < _outputNodes.size(); right_node++) {
                        _outputNodes[right_node] +=
                                (*edges)[left_node][right_node]*activation(_innerNodes[layer-1][left_node]);
                    }
                }
            } else {
                // Hidden Layers
                for (size_t left_node = 0; left_node < _innerNodes[layer-1].size(); left_node++) {
                    if (!enableNodeValueToBool(nodeEnabled(layer-1, left_node))) {
                        continue;
                    }
                    for (size_t right_node = 0; right_node < _innerNodes[layer].size(); right_node++) {
                        if (enableNodeValueToBool(nodeEnabled(layer, right_node))) {
                            _innerNodes[layer][right_node] +=
                                    (*edges)[left_node][right_node]*activation(_innerNodes[layer-1][left_node]);
                        }
                    }
                }
                // Apply recurrent nodes (handles check for you)
                processRecurrentNodes(layer);
            }
        }
    }
}

/**
 * @brief NeuralNet::processRecurrentNodes
 * @details Process for each layer after updating a forward layers
 * @param layer
 */
void NeuralNet::processRecurrentNodes(const size_t &layer) {
    if (layer < _recurrentNodes.size() && _nParams.type == Recurrent) {
        EdgeLayer * recEdges = recurrentEdgeLayer(layer);
        for (size_t recNode = 0; recNode < _recurrentNodes[layer].size(); recNode++) {
            if (!enableNodeValueToBool(nodeEnabled(layer, recNode))) {
                continue;
            }
            for (size_t conNode = 0; conNode < _recurrentNodes[layer].size(); conNode++) {
                if (!enableNodeValueToBool(nodeEnabled(layer, conNode))) {
                    continue;
                }
                _recurrentNodes[layer][recNode] +=
                        (*recEdges)[recNode][conNode] * activation(_innerNodes[layer][conNode]);
            }
            size_t recNodeEdge = (*recEdges)[recNode].size()-1;
            _innerNodes[layer][recNode] +=
                    (*recEdges)[recNode][recNodeEdge] * activation(_recurrentNodes[layer][recNode]);
        }
    }
}

void NeuralNet::resetNodesForRerun() {
    resetInnerNodes();
    resetOutputNodes();
}

bool NeuralNet::setState(const State & s) {
    // Validate safe to transfer
    if (_state.size() == s.size()) {
        for (size_t i = 0; i < s.size(); i++) {
            if (_state[i].size() == s[i].size()) {
                for (size_t j = 0; j < s[i].size(); j++) {
                    if (_state[i][j].size() != s[i][j].size()) {
                        return false;
                    }
                }
            } else {
                return false;
            }
        }
    } else {
        return false;
    }

    // Transfer
    for (size_t i = 0; i < s.size(); i++) {
        for (size_t j = 0; j < s[i].size(); j++) {
            for (size_t k = 0; k < s[i][j].size(); k++) {
                _state[i][j][k] = s[i][j][k];
            }
        }
    }
    _modifiedFlag = false;
    return true;
}

/*
void NeuralNet::printEdges() {
  qDebug() << "An edge: ";
  for (size_t i = 0; i < _edges.size(); i++) {
    qDebug() << "  Inner Net " << i+1;
    for (size_t j = 0; j < _edges[i].size(); j++) {
      for (size_t k = 0; k < _edges[i][j].size(); k++) {
        qDebug() << "  -- " << j+1 << " : " << k+1 << " = " << _edges[i][j][k];
      }
    }
  }
}*/

void NeuralNet::printState() {
  qDebug() << "State: ";
  for (size_t i = 0; i < _state.size(); i++) {
    qDebug() << "   Dimension: " << i;
    for (size_t j = 0; j < _state[i].size(); j++) {
      for (size_t k = 0; k < _state[i][j].size(); k++) {
        qDebug() << "  -- " << j+1 << " : " << k+1 << " = " << _state[i][j][k];
      }
    }
  }
}
