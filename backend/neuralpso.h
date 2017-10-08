#ifndef NEURALPSO_H
#define NEURALPSO_H

#include <QCoreApplication>

#include "PSO/pso.h"
#include "NeuralNet/NeuralNet.h"
#include "backend/teststatistics.h"

#include <ctime>
#include <limits>
#include <memory>
#include <random>
#include <string>

#include "logger.h"
#include "neuralpsostream.h"

#ifdef OPENCL_DEFINED
#include "CL/cl.hpp"
#endif

class NeuralNet;
//template class Pso<NeuralNet::CombEdgeType>;
template struct Particle<NeuralNet::CombEdgeType>;

struct FitnessParameters {
    real mse_weight;
    real mse_floor;
    TestStatistics::ClassificationError weights;
    TestStatistics::ClassificationError floors;

    real edgeWeightMax = 10E5L;
    real edgeWeightMin = -10E5L;
};

class NeuralPso : public Pso<NeuralNet::CombEdgeType> {
public:
  NeuralPso(PsoParams pp, NeuralNet::NeuralNetParameters np, FitnessParameters fp);
  virtual ~NeuralPso();

  typedef Particle<NeuralNet::CombEdgeType> NeuralParticle;

  void buildPso();
  void build(std::vector<std::vector<std::vector<byte> > > &images, std::vector<byte> &labels);
  void build(std::vector<std::vector<real>> &input, std::vector<real> &output);
  void fly();
  void getCost();
  real evaluate();
  void processEvents();

  void evaluatePoints(std::vector<bool> & printChange);
  void findPersonalBest(std::vector<bool> & printChange);
  void findLocalBest(std::vector<bool> & printChange);
  void findGlobalBest(std::vector<bool> & printChange);

  void updatePersonalBest(NeuralParticle & p);
  void updateNeighborBest(NeuralParticle & p, NeuralParticle & p_n);
  void updateGlobalBest(NeuralParticle & p);

  virtual real trainingRun();
  virtual void testGB();

  NeuralNet * neuralNet() { return _neuralNet; }
  std::unique_ptr<NeuralNet> buildNeuralNetFromGb();
  bool injectGb(const NeuralNet::CombEdgeType &w);

  std::string stringifyState();
  bool loadStatefromString(const std::string & psoState);

  void printGB();
  void printParticle(uint i);
  void printParticlePBest(uint i);
  void printParticleLBest(uint i);

  static void setToPrintGBNet();

  FitnessParameters * fitnessParams() { return &_fParams; }

  NeuralNet::CombEdgeType & getGbEdges();

protected:
    NeuralNet *_neuralNet;
    FitnessParameters _fParams;

private:
  static bool printGBFlag;
  static std::mutex printGBMtx;

  void flyIteration(size_t particle, size_t inner_net, size_t left_edge, size_t right_edge);

};

#endif // NEURALPSO_H
