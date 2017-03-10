#ifndef NEURALPSO_H
#define NEURALPSO_H

#include "PSO/pso.h"
#include "NeuralNet/NeuralNet.h"

template <class T>
class Pso;

template <class T>
struct Particle;

class NeuralPso : public Pso<Particle<vector<vector<vector<double>>>> > {
public:
  NeuralPso(NeuralNetParameters params);
  ~NeuralPso();

  void build(vector<vector<vector<byte> > > &images, vector<byte> &labels);
  void fly();
  double getCost();

  NeuralNet * neuralNet() { return _neuralNet; }

private:
  NeuralNet *_neuralNet;

};

#endif // NEURALPSO_H
