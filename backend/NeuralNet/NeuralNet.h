#ifndef NEURALNET_H
#define NEURALNET_H

#include <iostream>
#include <cstdlib>
#include <cinttypes>
#include <vector>
#include <cmath>
#include <ctime>
using namespace std;

#include "custommath.h"

#ifndef M_PI
// Qt redifines M_PI, so it disables <cmath>
// But we need it here and not <QtMath>
// to maintain portability.
#define M_PI 3.1415926535897
#endif

#define uint uint32_t
#define byte uint8_t

//#define EdgeType std::vector<std::vector<std::vector<double>>>

struct NeuralNetParameters {
    int inputs=0;               // Input nodes
    int innerNets=0;            // Total inner layers
    vector<int> innerNetNodes;  // Number of nodes for each layer
    int outputs=0;              // Output nodes
    int testIterations=20;      // Number of inputs to check per training iteration
};

class NeuralNet {
public:
    typedef std::vector<std::vector<std::vector<double>>> EdgeType;

    NeuralNet(const NeuralNetParameters &params);
    NeuralNet(const NeuralNetParameters &p, const EdgeType & n);
    NeuralNet(const NeuralNet & n);
    NeuralNet(NeuralNet && n);
    NeuralNet & operator= (const NeuralNet & n);
    NeuralNet & operator= (NeuralNet && n);
    ~NeuralNet();

    void initialize(const NeuralNetParameters & p);

    void resetInputs();
    void resetInnerNodes();
    void resetWeights();

    void setTotalInputs(uint n);
    void setTotalInnerNets(uint n);
    void setInnerNetNodes(uint nodes, uint i);
    void setTotalOutputs(uint n);

    size_t totalInputs() { return _inputNodes.size(); }
    size_t totalOutputs() { return _outputNodes.size(); }

    bool setWeights(const EdgeType &w);
    EdgeType & getWeights() {return _edges; }

    void loadInput(double in, uint i);
    void setOutputs(vector<double> out);
    const vector<double> & process();

    bool buildNets();

    static double activation(double in);
    static double getSign(const double & in);

    NeuralNetParameters * nParams() { return &_nParams; }

    void printEdges();

private:
    // Weights and Data
    vector<double> _inputNodes;
    vector<vector<double>> _innerNodes;  /// columns / node
    vector<double> _outputNodes;

    EdgeType _edges;  ///TODO: Make _edges a pointer and pass edge pointers from PSO.
    bool _localEdgesFlag;

    NeuralNetParameters _nParams;

/// Inner Edge index:           0           1            2  ... n-1            n
/// Nodes:               input -> inner (0) -> inner (1) -> ... -> inner (n-1) -> output
/// Dimension of edges:     dim(input, inner(0))   dim(inner(i), inner(i+1)    dim(inner(n), output)
};


#endif // NEURALNET_H
