#include "NeuralNet.h"

//RandomNumberEngine NeuralNet::_randomEngine(
//        std::chrono::system_clock::now().time_since_epoch().count());

NeuralNet::NeuralNet() {

}

NeuralNet::NeuralNet(const NeuralNetParameters & p) :
  _nParams(p)
{
    initialize(p);
}

NeuralNet::NeuralNet(const NeuralNetParameters &p, const State &s) :
    _nParams(p)
{
    initialize(p);
    if (!setState(s)) {
        std::cout << "Failed to set NeuralNet: NeuralNet()" << std::endl;
    }
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
    setTotalInnerNets(static_cast<size_t>(_nParams.innerNetNodes.size()));
    for (size_t i = 0; i < static_cast<size_t>(_nParams.innerNetNodes.size()); i++) {
      setInnerNetNodes(static_cast<size_t>(_nParams.innerNetNodes[i]), i);
    }
    setTotalOutputs(static_cast<size_t>(_nParams.outputs));

    buildANN();

    resetAllNodes();
    resetWeights();

    _built = true;
}

void NeuralNet::resetAllNodes() {

    resetInputNodes();
    resetInnerNodes();
    resetRecurrentNodes();
    resetOutputNodes();
}

void NeuralNet::resetInputNodes() {
    for (size_t i = 0; i < _inputNodes.size(); i++) {
        if (i < _inputNodes.size()-1) {
            _inputNodes[i] = 0;
        } else {
            _inputNodes[i] = _inputNodes.size();
        }
    }
}

void NeuralNet::resetInnerNodes() {
    for (size_t i = 0; i < _innerNodes.size(); i++) {
        for (size_t j = 0; j < _innerNodes[i].size(); j++) {
            if (j < _innerNodes[i].size()-1) {
                _innerNodes[i][j] = 0;
            } else {
                _innerNodes[i][j] = _innerNodes[i].size();
            }
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
        _innerNodes[i].resize(nodes+1, 0);
        _innerNodes[i].back() = 1;
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

    // Build activation constants in state
    buildActivationConstants();

    // Randomize the edges
    randomizeState();

    // Randomize activation constants
    randomizeActivationConstants();

    return true;
}

bool NeuralNet::validateParams() {
    // Verify nodes have been built
    if (_inputNodes.size() == 0 || _outputNodes.size() == 0) {
        return false;
    }
    return true;
}

void NeuralNet::prepareState() {
    _state.clear();
    _state.resize(totalStateElementsFromInnerNodes(_innerNodes.size()));
}

size_t NeuralNet::totalStateElementsFromInnerNodes(const size_t & innerNodeLayers) {
    size_t edgeLayers = innerNodeLayers + 1;
    size_t totalElements = 1 + edgeLayers + innerNodeLayers + 1;
    return totalElements;
}

size_t NeuralNet::totalEdgeLayersFromState(const State & state) {
    size_t edgeLayers = (state.size()-1) / 2;
    return edgeLayers;
}

size_t NeuralNet::totalInnerNodeLayersFromState(const State & state) {
    int innerNodes = static_cast<int>(totalEdgeLayersFromState(state)) - 1;
    innerNodes = innerNodes < 0 ? 0 : innerNodes;
    return static_cast<size_t>(innerNodes);
}

/*
size_t NeuralNet::totalInputsFromState(const State & state) {
    ///TODO
    return 0;
}
*/

void NeuralNet::buildTopology() {
    InnerNodes & topology = _state[0];
    size_t totalLayers = _innerNodes.size();
    topology.resize(totalLayers);

    for (size_t i = 0; i < totalLayers; i++) {
        topology[i].resize(_innerNodes[i].size()-1, 1.0);
    }
}

void NeuralNet::buildActivationConstants() {
    InnerNodes & kConstants = getConstantsFromState(_state);
    size_t totalLayers = _innerNodes.size();
    kConstants.resize(totalLayers);

    for (size_t i = 0; i < totalLayers; i++) {
        kConstants[i].resize(_innerNodes[i].size()-1, 0.0);
    }
}

void NeuralNet::randomizeActivationConstants() {
    InnerNodes & kConstants = getConstantsFromState(_state);
    for (size_t i = 0; i < kConstants.size(); i++) {
        for (size_t j = 0; j < kConstants[i].size(); j++) {
            kConstants[i][j] = _randomEngine.uniformReal(0, 10.0);
        }
    }
}

NeuralNet::InnerNodes & NeuralNet::getConstantsFromState() {
    return getConstantsFromState(_state);
}

NeuralNet::InnerNodes & NeuralNet::getConstantsFromState(State &state) {
    return state.back();
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

    bool goodBuild = true;

    size_t totalInnerLayers = _innerNodes.size();

    if (totalInnerLayers == 0) {
        // Connect input to outputs directly without hidden layer
        buildWithoutHiddenLayer();
    } else {
        goodBuild = buildWithHiddenLayer(totalInnerLayers+1);

    }
    return goodBuild;
}

void NeuralNet::buildWithoutHiddenLayer() {
    _state[1].resize(_inputNodes.size());
    for (size_t i = 0; i < _inputNodes.size(); i++) {
        _state[1][i].resize(_outputNodes.size(), 0);
    }
}

bool NeuralNet::buildWithHiddenLayer(const size_t & totalInnerLayers) {
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
                    _state[stateIt][left_node].resize(_innerNodes[0].size()-1);
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
                _state[stateIt][left_node].resize(_innerNodes[i].size()-1, 0);
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
    size_t totalInnerNodeLayers = totalInnerNodeLayersFromState(_state);
    if (totalInnerNodeLayers != _innerNodes.size()) return false;
    if (totalInnerNodeLayers == 0 ) return true;

    size_t startIndex = 1 + totalInnerNodeLayers+1;

    for (size_t layer = startIndex, i=0; layer < totalInnerNodeLayers+startIndex; layer++, i++) {
        if (layer > _state.size()) {
            return false;
        }
        _state[layer].resize(_innerNodes[i].size()-1);
        for (size_t recNode = 0; recNode < _state[layer].size(); recNode++) {
            _state[layer][recNode].resize(_innerNodes[i].size());
        }
    }
    return true;
}

void NeuralNet::enableAllNodes(const bool & t) {
    if (_state.size() > 0) {
        for (size_t i = 0; i < _state[0].size(); i++) {
            for (size_t j = 0; j < _state[0][i].size(); j++) {
                _state[0][i][j] = enableNodeBoolToValue(t);
            }
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

void NeuralNet::randomDropoutNodes(const real &mean=0, const real & sigma=1) {
    //std::normal_distribution<real> dist(mean, sigma);
    //std::uniform_real_distribution<real> dist(-1, 1);

    for (size_t i = 0; i < _state[0].size(); i++) {
        for (size_t j = 0; j < _state[0][i].size(); j++) {
            _state[0][i][j] = _randomEngine.normal(mean, sigma);
        }
    }
}

void NeuralNet::randomizeEnabledNodes() {
    for (size_t i = 0; i < _state[0].size(); i++) {
        for (size_t j = 0; j < _state[0][i].size(); j++) {
            _state[0][i][j] = _randomEngine.uniformReal(-1.0, 1.0);
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
  if (i < _inputNodes.size()-1) {
    _inputNodes[i] = in;
  }
}

void NeuralNet::loadInputs(const ExternalNodes & in) {
    for (size_t i = 0; i < in.size(); i++) {
        loadInput(in[i], i);
    }
}

NeuralNet::ExternalNodes NeuralNet::inputs() {
    std::vector<real> inputs;
    inputs.resize(static_cast<size_t>(_nParams.inputs));
    for (size_t i = 0; i < inputs.size(); i++) {
        inputs[i] = _inputNodes[i];
    }
    return inputs;
}

real NeuralNet::activation(const real & in, const real & k, const NeuralNet::Activation & act) {

    switch(act) {
    case NeuralNet::ReLU:
        return ActivationFunctions::ReLU(in);
    case Sin:
        return ActivationFunctions::Sin(in, k);
    case Sigmoid:
        return ActivationFunctions::Sigmoid(in, k);
    case HypTan:
        return ActivationFunctions::HypTan(in, k);
    case Gaussian:
        return ActivationFunctions::Gaussian(in, k);
    case Sinc:
        return ActivationFunctions::Sinc(in, k);
    case Step:
        return ActivationFunctions::Step(in, k);
    default:
        return in;
    }
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
/*
void NeuralNet::processRecurrentNodes() {
    if (_nParams.type == Recurrent) {
        // For each layer
        for (size_t layer = 0; layer < _innerNodes.size(); layer++) {
            // For each recurrent node
            for (size_t recNode = 0; recNode < _innerNodes[layer].size(); recNode++) {
                if (isSkipNode(layer, recNode)) {
                    continue;
                }
                // For each connected node;
                for (size_t conNode = 0; conNode < _innerNodes[layer].size(); conNode++) {
                    if (isSkipNode(layer, conNode)) {
                        continue;
                    }
                    _recurrentNodes[layer][recNode] +=
                            (*recurrentEdgeLayer(layer))[recNode][conNode] *
                            activation(_innerNodes[layer][conNode]);
                }
            }
        }
    } else if (_nParams.type == Feedforward){
        resetNodesForRerun();
    }
}
*/
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

        InnerNodes & kConstants = getConstantsFromState();

        for (size_t layer = 0; layer < _innerNodes.size()+1; layer++) {
            EdgeLayer * edges = forwardEdgeLayer(layer);

            if (layer == 0) {
                // Process Input to Hidden
                for (size_t left_node = 0; left_node < _inputNodes.size(); left_node++) {
                    for (size_t right_node = 0; right_node < _innerNodes[0].size()-1; right_node++) {
                        if (isSkipNode(layer+1, right_node)) {
                            continue;
                        }
                        _innerNodes[0][right_node] +=
                               (*edges)[left_node][right_node]*_inputNodes[left_node];
                    }
                }
            } else if (layer == _innerNodes.size()) {
                // Hidden to Output
                for (size_t left_node = 0; left_node < _innerNodes[layer-1].size(); left_node++) {
                    if (isSkipNode(layer, left_node)) {
                        continue;
                    }
                    for (size_t right_node = 0; right_node < _outputNodes.size(); right_node++) {
                        if (left_node == _innerNodes[layer-1].size()-1) {
                            _outputNodes[right_node] += (*edges)[left_node][right_node] * _innerNodes[layer-1][left_node];
                        } else {
                            real k_constant = kConstants[layer-1][left_node];
                            _outputNodes[right_node] +=
                                (*edges)[left_node][right_node] *
                                  activation(_innerNodes[layer-1][left_node], k_constant, _nParams.act);
                        }
                    }
                }
            } else {
                // Hidden Layers
                for (size_t left_node = 0; left_node < _innerNodes[layer-1].size(); left_node++) {
                    if (isSkipNode(layer, left_node)) {
                        continue;
                    }
                    for (size_t right_node = 0; right_node < _innerNodes[layer].size()-1; right_node++) {
                        if (isSkipNode(layer+1, right_node)) {
                            continue;
                        }
                        if (left_node == _innerNodes[layer-1].size()-1) {
                            _innerNodes[layer][right_node] += (*edges)[left_node][right_node] * _innerNodes[layer-1][left_node];
                        } else {
                            real k_constant = kConstants[layer-1][left_node];
                            _innerNodes[layer][right_node] +=
                                    (*edges)[left_node][right_node] *
                                    activation(_innerNodes[layer-1][left_node], k_constant, _nParams.act);
                        }
                    }
                }
            }
            // Apply recurrent nodes (handles check for you)
            processRecurrentNodes(layer);
        }
    }

    //normalizeOutputNodes();
}

/**
 * @brief NeuralNet::isSkipNode
 * @details Returns true if node is to be skipped
 * @details Returns default of false for all other situations
 * @param layer - size_t
 * @param node - size_t
 * @return
 */
bool NeuralNet::isSkipNode(const size_t & layer, const size_t & node) {
    if (_state.size() == 0) return false;
    if (layer >= _state[0].size()+1 || layer == 0) return false; // Inner Nodes Only
    if (node >= _state[0][layer-1].size()) return false;

    return !enableNodeValueToBool(_state[0][layer-1][node]);
}

/**
 * @brief NeuralNet::isSkipEdge
 * @details Returns true if edge contains skip node
 * @param leftLayer - size_t
 * @param leftNode - size_t
 * @param rightNode - size_t
 * @return
 */
bool NeuralNet::isSkipEdge(const size_t & leftLayer, const size_t & leftNode, const size_t & rightNode) {
    bool skipEdge = isSkipNode(leftLayer, leftNode);
    skipEdge |= isSkipNode(leftLayer+1, rightNode);
    return skipEdge;
}

/**
 * @brief NeuralNet::isSkipNode
 * @details static function for skip node detection.
 * @param state
 * @param layer
 * @param node
 * @return
 */
bool NeuralNet::isSkipNode(const State & state, const size_t & layer, const size_t & node) {
    if (state.size() == 0) return false;
    if (layer >= state[0].size()) return false;
    if (node >= state[0][layer].size()) return false;

    return !enableNodeValueToBool(state[0][layer][node]);
}

bool NeuralNet::isSkipEdge(const State & state, const size_t & leftLayer, const size_t & leftNode, const size_t & rightNode) {
    bool skipEdge = isSkipNode(state, leftLayer, leftNode);
    skipEdge |= isSkipNode(state, leftLayer+1, rightNode);
    return skipEdge;
}

/**
 * @brief NeuralNet::processRecurrentNodes
 * @details Process for each layer after updating a forward layers
 * @param layer
 */
void NeuralNet::processRecurrentNodes(const size_t &layer) {
    if (layer < _recurrentNodes.size() && _nParams.type == Recurrent) {
        EdgeLayer * recEdges = recurrentEdgeLayer(layer);
        std::vector<real> & kConstants = getConstantsFromState()[layer];

        // Update the recurrent nodes
        for (size_t recNode = 0; recNode < _recurrentNodes[layer].size(); recNode++) {
            if (isSkipNode(layer, recNode) || recNode == _innerNodes[layer].size()-1) {
                continue;
            }
            for (size_t conNode = 0; conNode < _recurrentNodes[layer].size(); conNode++) {
                if (isSkipNode(layer, conNode) || conNode == _innerNodes[layer].size()-1) {
                    continue;
                }
                real k_constant_connode = kConstants[conNode];
                _recurrentNodes[layer][recNode] +=
                        (*recEdges)[recNode][conNode] *
                          activation(_innerNodes[layer][conNode], k_constant_connode, _nParams.act);
            }
        }

        // Feedback the recurrent node to the assigned hidden node
        for (size_t node = 0; node < _recurrentNodes[layer].size(); node++) {
            size_t recNodeEdge = (*recEdges)[node].size()-1;
            //real k_constant_recnode = kConstants[node];
            _innerNodes[layer][node] +=
                    (*recEdges)[node][recNodeEdge] *
                    //activation(_recurrentNodes[layer][node], k_constant_recnode);
                    activation(_recurrentNodes[layer][node], 5.0, _nParams.act);
        }
    }
}

void NeuralNet::normalizeOutputNodes() {
    for (size_t i = 0; i < _outputNodes.size(); i++) {
        if (_outputNodes[i] > 1.0) {
            _outputNodes[i] = 1.0;
        } else if (_outputNodes[i] < -1.0) {
            _outputNodes[i] = -1.0;
        }
    }
}

void NeuralNet::resetNodesForRerun() {
    resetInnerNodes();
    resetOutputNodes();
}

bool NeuralNet::setState(const State & s) {
    int inputsFromState = 0, outputsFromState = 0;
    std::vector<int> innerNodesFromState;

    if (s.size() == 0) {
        std::cout << "Failed to set State: s.size() == 0" << std::endl;
        return false;
    }
    innerNodesFromState.resize(s[0].size());
    for (size_t i = 0; i < s[0].size(); i++) {
        innerNodesFromState[i] = s[0][i].size();
    }
    if (s.size() < 1) {
        std::cout << "Failed to set State: s.size() < 1" << std::endl;
        return false;
    }
    inputsFromState = s[1].size() - 1;
    if (s.size() < 1+innerNodesFromState.size()) {
        std::cout << "Failed to set State: s.size() < 1+innerNodesFromState.size()=" << 1+innerNodesFromState.size() << std::endl;
        return false;
    }
    if (s[innerNodesFromState.size()].size() == 0) {
        std::cout << "Failed to set State: s[innerNodesFromState.size()=" << innerNodesFromState.size() << "] == 0" << std::endl;
        return false;
    }
    outputsFromState = s[innerNodesFromState.size()+1][0].size();

    _nParams.inputs = inputsFromState;
    _nParams.innerNetNodes = innerNodesFromState;
    _nParams.outputs = outputsFromState;
    initialize(_nParams);
    // Validate safe to transfer
    if (_state.size() == s.size()) {
        for (size_t i = 0; i < s.size(); i++) {
            if (_state[i].size() == s[i].size()) {
                for (size_t j = 0; j < s[i].size(); j++) {
                    if (_state[i][j].size() != s[i][j].size()) {
                        std::cout << "Failed to set State: _state[" << i << "][" << j << "].size()=" << _state[i][j].size() << " != s[" << i << "][" << j << "].size()=" << s[i][j].size() << std::endl;
                        return false;
                    }
                }
            } else {
                std::cout << "Failed to set State: _state[" << i << "].size()=" << _state[i].size() << " != s[" << i << "].size()=" << s[i].size() << std::endl;
                return false;
            }
        }
    } else {
        std::cout << "Failed to set State: _state.size()=" << _state.size() << " != s.size()=" << s.size() << std::endl;
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

void NeuralNet::randomizeState() {
    real minVal = static_cast<real>(-1.0);
    real maxVal = static_cast<real>( 1.0);
    for (size_t i = 1; i < _state.size(); i++) {
        for (size_t j = 0; j < _state[i].size(); j++) {
            for (size_t k = 0; k < _state[i][j].size(); k++) {
                _state[i][j][k] = _randomEngine.uniformReal(minVal, maxVal);
            }
        }
    }
    enableAllNodes(true);
}

/**
 * @brief NeuralNet::validatePaths
 * @details Returns true if each output is connected to at least one input
 * @return bool
 * @todo We can get rid of BFS and just scan the enable nodes for an entire disabled layer
 */
bool NeuralNet::validatePaths() {
    return validateEnabledLayers();
    bool outputPath = true;

    for (size_t o = 0; o < totalOutputs(); o++) {
        for (size_t i = 0; i < totalInputs(); i++) {
            outputPath &= validatePath(i, o);
            if (!outputPath) return false;
        }
    }
    return outputPath;

}

bool NeuralNet::validateEnabledLayers() {
    std::vector<std::vector<real>> &enableLayers = _state[0];
    bool ok = true;
    for (size_t i = 0; i < enableLayers.size(); i++) {
        bool goodLayer = false;
        for (size_t j = 0; j < enableLayers[i].size(); j++) {
            goodLayer |= !isSkipNode(i+1, j);
            if (goodLayer) {
                break;
            }
        }
        ok &= goodLayer;
        if (!ok) {
            return false;
        }
    }
    return true;
}

bool NeuralNet::validatePath(const size_t & inputNode, const size_t & outputNode) {
    if (_state.size() == 0) return false;

    size_t totalNodeLayers = _innerNodes.size() + 2;

    struct Node {
        size_t layer;
        size_t vertex;
    };

    struct Edge {
        Node fromNode;
        Node toNode;
    };

    std::queue<Node> frontier;

    Node inNode;
    inNode.layer = 0;
    inNode.vertex = inputNode;
    frontier.push(inNode);

    while (frontier.size() > 0) {
        Node parent = frontier.front();
        frontier.pop();
        std::vector<real> & rightNodes = _state[parent.layer+1][parent.vertex];
        size_t nextLayer = parent.layer + 1;
        for (size_t rightNode = 0; rightNode < rightNodes.size(); rightNode++) {
            bool nodeEnabled = !isSkipNode(nextLayer, rightNode);
            if (nodeEnabled) {
                if (nextLayer == totalNodeLayers-1 && rightNode == outputNode) {
                    return true;
                } else {
                    Node newFrontier;
                    newFrontier.layer = nextLayer;
                    newFrontier.vertex = rightNode;
                    frontier.push(newFrontier);
                }
            }
        }
    }
    return false;

}

/**
 * @brief NeuralNet::proposedTopology
 * @details Call this after topo training to find hidden layers
 * @return - topology std::vector<std::vector<real>>
 */
std::vector<int> NeuralNet::proposedTopology() {
    std::vector<int> m_proposedTopology;
    if (_state.size() == 0) {
        std::cout << "Failed to find topology: _state empty." << std::endl;
        return m_proposedTopology;
    }
    std::vector<std::vector<real>> & topo = _state[0];
    for (size_t i = 0; i < topo.size(); i++) {
        //std::vector<real> layerBuffer;
        int layerCount = 0;
        for (size_t j = 0; j < topo[i].size(); j++) {
            if (topo[i][j] > 0) {
                layerCount++;
                //layerBuffer.push_back(j);
            }
        }
        m_proposedTopology.push_back(layerCount);
    }
    return m_proposedTopology;
}
