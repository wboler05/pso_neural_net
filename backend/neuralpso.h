#ifndef NEURALPSO_H
#define NEURALPSO_H

#include <QCoreApplication>

#include "PSO/pso.h"
#include "NeuralNet/NeuralNet.h"

#include <ctime>
#include <limits>

#include "CL/cl.hpp"
#include "util.h"

/*
template <class T>
class Pso;
*/
template class Pso<EdgeType>;

/*
template <class T>
struct Particle;
*/
template struct Particle<EdgeType>;

class NeuralNet;

class NeuralPso : public Pso<EdgeType> {
public:
  NeuralPso(PsoParams pp, NeuralNetParameters np);
  ~NeuralPso();

  void buildPso();
  void build(std::vector<std::vector<std::vector<byte> > > &images, std::vector<byte> &labels);
  void build(std::vector<std::vector<double>> &input, std::vector<double> &output);
  void fly();
  double getCost();
  void processEvents();

  double testRun(double &correctRatio, uint &totalCount, double &confidence);
  void testGB();

  NeuralNet * neuralNet() { return _neuralNet; }

  int randomizeTestInputs();
  void runTrainer();
  void loadTestInput(uint32_t I);

  void printGB();
  void printParticle(uint i);
  void printParticlePBest(uint i);
  void printParticleLBest(uint i);

  static void setToPrintGBNet();

  void setFunctionMsg(std::string s) { _functionMsg = s; }
  std::string functionMsg() { return _functionMsg; }

  void classError();

protected:
    NeuralNet *_neuralNet;

private:
  std::vector<std::vector<std::vector<byte> > > *_images;
  std::vector<byte> *_labels;

  // Test input
  std::vector<std::vector<double> > *_input;
  std::vector<double> *_output;
  std::vector<double> *_outputCount;
  std::vector<std::vector<uint>> _outputIterators;

  std::string _functionMsg;

  static bool printGBFlag;
  static boost::mutex printGBMtx;

  void flyIteration(size_t particle, size_t inner_net, size_t left_edge, size_t right_edge);

//  vector<pair<bool, int>> _failureQueue;

};

#endif // NEURALPSO_H
