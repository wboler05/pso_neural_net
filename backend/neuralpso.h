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
template class Pso<std::vector<std::vector<std::vector<double>>>>;

/*
template <class T>
struct Particle;
*/
template struct Particle<std::vector<std::vector<std::vector<double>>>>;

class NeuralNet;

class NeuralPso : public Pso<std::vector<std::vector<std::vector<double>>>> {
public:
  NeuralPso(PsoParams pp, NeuralNetParameters np);
  ~NeuralPso();

  void buildPso();
  void build(std::vector<std::vector<std::vector<byte> > > &images, std::vector<byte> &labels);
  void build(std::vector<std::vector<double>> &input, std::vector<double> &output);
  void fly();
  void getCost();
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
  vector<vector<vector<byte> > > *_images;
  vector<byte> *_labels;

  // Test input
  vector<vector<double> > *_input;
  vector<double> *_output;
  vector<double> *_outputCount;
  vector<vector<uint>> _outputIterators;

  std::string _functionMsg;

  static bool printGBFlag;
  static boost::mutex printGBMtx;

//  vector<pair<bool, int>> _failureQueue;

};

#endif // NEURALPSO_H
