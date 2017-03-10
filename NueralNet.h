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

class NeuralNet {
public:
  NeuralNet();
  ~NeuralNet();

  void resetInputs();
  void resetInnerNodes();
  void resetWeights();
  void trainData(vector<vector<vector<byte> > > &images, vector<byte> &labels);

  void setTotalInputs(uint n);
  void setTotalInnerNets(uint n);
  void setInnerNetNodes(uint nodes, uint i);
  void setTotalOutputs(uint n);

  void loadInput(double in, uint i);
  void setOutputs(vector<double> out);
  vector<double> process();

  double activation(double in);
  bool buildNets();

private:
  // Weights and Data
  vector<double> _inputNodes;
  vector<vector<double>> _innerNodes;  /// columns / node
  vector<double> _outputNodes;

  vector<vector<vector<double>>> _edges;

/// Inner Edge index:           0           1            2  ... n-1            n
/// Nodes:               input -> inner (0) -> inner (1) -> ... -> inner (n-1) -> output
/// Dimension of edges:     dim(input, inner(0))   dim(inner(i), inner(i+1)    dim(inner(n), output)
};


#endif // NEURALNET_H