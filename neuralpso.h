#ifndef NEURALPSO_H
#define NEURALPSO_H

#include "PSO/pso.h"
#include "NeuralNet/NeuralNet.h"

#include <ctime>
#include <limits>

#include "CL/opencl.h"

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

  void build(vector<vector<vector<byte> > > &images, vector<byte> &labels);
  void fly();
  void getCost();

  double testRun();

  NeuralNet * neuralNet() { return _neuralNet; }

  int randomizeTestInputs();
  void runTrainer();

private:
  NeuralNet *_neuralNet;
  vector<vector<vector<byte> > > *_images;
  vector<byte> *_labels;

};

#endif // NEURALPSO_H
