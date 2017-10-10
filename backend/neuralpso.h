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
template struct Particle<NeuralNet::State>;

struct FitnessParameters {
    real mse_weight;
    real mse_floor;
    TestStatistics::ClassificationError weights;
    TestStatistics::ClassificationError floors;

    real edgeWeightMax = 10E5L;
    real edgeWeightMin = -10E5L;
};

class NeuralPso : public Pso<NeuralNet::State> {
public:
  NeuralPso(PsoParams pp, NeuralNet::NeuralNetParameters np, FitnessParameters fp);
  virtual ~NeuralPso();

  typedef Particle<NeuralNet::State> NeuralParticle;

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
  bool injectGb(const NeuralNet::State &s);

  std::string stringifyState();
  bool loadStatefromString(const std::string & psoState);

  void printGB();
  void printParticle(size_t i);
  void printParticlePBest(size_t i);
  void printParticleLBest(size_t i);

  static void setToPrintGBNet();

  FitnessParameters * fitnessParams() { return &_fParams; }

  NeuralNet::State & getGbState();

protected:
    NeuralNet *_neuralNet;
    FitnessParameters _fParams;

private:
  static bool printGBFlag;
  static std::mutex printGBMtx;
  static volatile bool stopProcessing;

  void flyIteration(size_t particle, size_t inner_net, size_t left_edge, size_t right_edge);

};

#endif // NEURALPSO_H
