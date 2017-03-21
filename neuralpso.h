#ifndef NEURALPSO_H
#define NEURALPSO_H

#include "PSO/pso.h"
#include "NeuralNet/NeuralNet.h"

#include <ctime>
#include <limits>

#include "CL/cl.hpp"

/*
template <class T>
class Pso;
*/
template class Pso<vector<vector<vector<double>>>>;

/*
template <class T>
struct Particle;
*/
template struct Particle<vector<vector<vector<double>>>>;

class NeuralNet;

class NeuralPso : public Pso<vector<vector<vector<double>>>> {
public:
  NeuralPso(PsoParams pp, NeuralNetParameters np);
  ~NeuralPso();

  void buildPso();
  void build(vector<vector<vector<byte> > > &images, vector<byte> &labels);
  void build(vector<vector<double>> &input, vector<double> &output);
  void fly();
  void getCost();

  double testRun(double &, uint &);
  void testGB();

  NeuralNet * neuralNet() { return _neuralNet; }

  int randomizeTestInputs();
  void runTrainer();
  void loadTestInput(uint32_t I);

  void printGB();
  void printParticle(uint i);
  void printParticlePBest(uint i);
  void printParticleLBest(uint i);

private:
  NeuralNet *_neuralNet;
  vector<vector<vector<byte> > > *_images;
  vector<byte> *_labels;

  // Test input
  vector<vector<double> > *_input;
  vector<double> *_output;

//  vector<pair<bool, int>> _failureQueue;

};

#endif // NEURALPSO_H
