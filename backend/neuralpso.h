#ifndef NEURALPSO_H
#define NEURALPSO_H

#include <QCoreApplication>

#include "PSO/pso.h"
#include "NeuralNet/NeuralNet.h"
#include "backend/teststatistics.h"

#include <ctime>
#include <limits>
#include <memory>

#include "CL/cl.hpp"
#include "util.h"

class NeuralNet;
template class Pso<NeuralNet::EdgeType>;
template struct Particle<NeuralNet::EdgeType>;

struct FitnessParameters {
    double mse_weight;
    double mse_floor;
    TestStatistics::ClassificationError weights;
    TestStatistics::ClassificationError floors;
};

class NeuralPso : public Pso<NeuralNet::EdgeType> {
public:
  NeuralPso(PsoParams pp, NeuralNetParameters np, FitnessParameters fp);
  ~NeuralPso();

  void buildPso();
  void build(std::vector<std::vector<std::vector<byte> > > &images, std::vector<byte> &labels);
  void build(std::vector<std::vector<double>> &input, std::vector<double> &output);
  void fly();
  double getCost();
  void processEvents();

  virtual double testRun(double &correctRatio, uint &totalCount, double &confidence);
  virtual void testGB();

  NeuralNet * neuralNet() { return _neuralNet; }
  std::unique_ptr<NeuralNet> buildNeuralNetFromGb();
  bool NeuralPso::injectGb(const NeuralNet::EdgeType &w);

  void printGB();
  void printParticle(uint i);
  void printParticlePBest(uint i);
  void printParticleLBest(uint i);

  static void setToPrintGBNet();

  FitnessParameters * fitnessParams() { return &_fParams; }

  NeuralNet::EdgeType & getGbEdges();

protected:
    NeuralNet *_neuralNet;
    FitnessParameters _fParams;

private:
  static bool printGBFlag;
  static std::mutex printGBMtx;

  void flyIteration(size_t particle, size_t inner_net, size_t left_edge, size_t right_edge);

};

#endif // NEURALPSO_H
