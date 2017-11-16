#ifndef NEURALNET_H
#define NEURALNET_H

#include <iostream>
#include <cstdlib>
#include <memory>
#include <vector>
#include <queue>
#include <cmath>
#include <ctime>
#include <chrono>

//#ifdef USE_MAC
//#include <tr1/cinttypes>
//#include <tr1/random>
//#include
//#else
#include <cinttypes>
#include <random>
//#endif

using namespace std;

#include "custommath.h"
#include "randomnumberengine.h"

#ifndef M_PI
// Qt redifines M_PI, so it disables <cmath>
// But we need it here and not <QtMath>
// to maintain portability.
#define M_PI 3.1415926535897
#endif

//#define uint uint32_t
#define byte uint8_t

class NeuralNet {
public:

    typedef std::vector<std::vector<real>> EdgeLayer;
    typedef std::vector<std::vector<std::vector<real>>> State;
    typedef std::vector<std::vector<real>> InnerNodes;
    typedef std::vector<real> ExternalNodes;

    enum Type { Feedforward = 0, Recurrent };

    struct NeuralNetParameters {
        int inputs=0;               // Input nodes
        vector<int> innerNetNodes;  // Number of nodes for each layer
        int outputs=0;              // Output nodes
        int trainingIterations=20;  // Total iterations of training
        int validationIterations=20;// Total iterations of validation
        int testIterations=20;      // Total iterations of testing
        NeuralNet::Type type = Feedforward;
    };

    NeuralNet();
    NeuralNet(const NeuralNetParameters &params);
    NeuralNet(const NeuralNetParameters &p, const State &n);
    NeuralNet(const NeuralNet & n);
    NeuralNet(NeuralNet && n);
    NeuralNet & operator= (const NeuralNet & n);
    NeuralNet & operator= (NeuralNet && n);
    ~NeuralNet();

    void initialize(const NeuralNetParameters & p);

    void resetAllNodes();
    void resetInputNodes();
    void resetInnerNodes();
    void resetRecurrentNodes();
    void resetOutputNodes();
    void resetWeights();

    void resetNodesForRerun();

    void setTotalInputs(const size_t & n);
    void setTotalInnerNets(const size_t & n);
    void setInnerNetNodes(const size_t & nodes, const size_t & i);
    void setTotalOutputs(const size_t & n);

    size_t totalInputs() { return _inputNodes.size(); }
    size_t totalOutputs() { return _outputNodes.size(); }

    bool setState(const State & s);
    State & state() { return _state; }
    void randomizeState();
    void randomizeActivationConstants();

    void loadInput(const real &in, const size_t & i);
    void loadInputs(const ExternalNodes & in);
    void setOutputs(const std::vector<real>& out);
    const ExternalNodes &process();

    ExternalNodes inputs();
    const ExternalNodes & outputs() { return _outputNodes; }

    InnerNodes & getConstantsFromState();
    static InnerNodes & getConstantsFromState(State & state);

    //TODO
    void enableAllNodes(const bool & t);
    void randomDropoutNodes(const real & mean, const real & sigma);
    void randomizeEnabledNodes();

    static real enableNodeBoolToValue(const bool &t);
    static bool enableNodeValueToBool(const real &val);

    EdgeLayer * forwardEdgeLayer(const size_t & i);
    EdgeLayer * recurrentEdgeLayer(const size_t & i);

    static real activation(const real & in, const real & k);
    static real getSign(const real & in);

    NeuralNetParameters * nParams() { return &_nParams; }

    //void printEdges();
    void printState();

    static size_t totalStateElementsFromInnerNodes(const size_t & innerNodes);
    static size_t totalEdgeLayersFromState(const State & state);
    static size_t totalInnerNodeLayersFromState(const State & state);
    static size_t totalInputsFromState(const State & state);
    static size_t totalOutputsFromState(const State & state);

    bool isSkipNode(const size_t & layer, const size_t & node);
    bool isSkipEdge(const size_t & leftLayer, const size_t & leftNode, const size_t & rightNode);

    static bool isSkipNode(const State & state, const size_t & layer, const size_t & node);
    bool isSkipEdge(const State & state, const size_t & leftLayer, const size_t & leftNode, const size_t & rightNode);

    bool validatePaths();

    bool buildANN();

private:
    // Weights and Data
    ExternalNodes _inputNodes;  // Nodes
    InnerNodes _innerNodes;     // Columns : Nodes
    InnerNodes _recurrentNodes; // Columns : Nodes
    ExternalNodes _outputNodes; // Nodes
    bool _modifiedFlag = false;
    bool _built = false;

    State _state;   // [ T[][] : Edges[][] : Recurrent Edges[][] ]
    bool _localEdgesFlag;

    NeuralNetParameters _nParams;

    RandomNumberEngine _randomEngine = std::chrono::system_clock::now().time_since_epoch().count();

/// Inner Edge index:           0           1            2  ... n-1            n
/// Nodes:               input -> inner (0) -> inner (1) -> ... -> inner (n-1) -> output
/// Dimension of edges:     dim(input, inner(0))   dim(inner(i), inner(i+1)    dim(inner(n), output)

    bool validateParams();
    void prepareState();
    void buildTopology();
    void buildActivationConstants();
    bool buildEdges();
    bool buildForwardEdges();
    bool buildRecurrentEdges();

    void buildWithoutHiddenLayer();
    bool buildWithHiddenLayer(const size_t & hiddenLayers);

    void processRecurrentNodes();
    void processRecurrentNodes(const size_t &layer);
    void processForwardPropagation();
    void normalizeOutputNodes();

    bool validateEnabledLayers();
    bool validatePath(const size_t & inputNode, const size_t & outputNode);
};


#endif // NEURALNET_H
