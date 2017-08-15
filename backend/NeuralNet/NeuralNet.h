#ifndef NEURALNET_H
#define NEURALNET_H

#include <iostream>
#include <cstdlib>
#include <cinttypes>
#include <vector>
#include <cmath>
#include <ctime>
using namespace std;

#define uint uint32_t
#define byte uint8_t

#define N_Accuracy(tp, tn, fp, fn) ((tp + tn) / (tp + tn + fp + fn))
#define N_Precision(tp, fp) (tp / (tp + fp + 1.0f))
#define N_Sensitivity(tp, fn) (tp / (tp + fn + 1.0))
#define N_Specificity(tn, fp) (tn / (tn + fp + 1.0))
#define N_F_Score(tp, fp, fn) (2.0f*tp / (2.0f*tp + fp + fn + 1.0))

#define EdgeType std::vector<std::vector<std::vector<double>>>

struct NeuralNetParameters {
  int inputs;                 // Input nodes
  int innerNets;              // Total inner layers
  vector<int> innerNetNodes;  // Number of nodes for each layer
  int outputs;                // Output nodes
  int testIterations=20;      // Number of inputs to check per training iteration
};

class NeuralNet {
public:
  NeuralNet(NeuralNetParameters params);
  ~NeuralNet();

  void resetInputs();
  void resetInnerNodes();
  void resetWeights();

  void setTotalInputs(uint n);
  void setTotalInnerNets(uint n);
  void setInnerNetNodes(uint nodes, uint i);
  void setTotalOutputs(uint n);

  bool setWeights(vector<vector<vector<double>>> * w);
  vector<vector<vector<double>>> & getWeights() {return _edges; }

  void loadInput(double in, uint i);
  void setOutputs(vector<double> out);
  const vector<double> & process();

  double activation(double in);
  bool buildNets();

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
