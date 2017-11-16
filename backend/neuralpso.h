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
#include <algorithm>
#include <thread>

#include "logger.h"
#include "neuralpsostream.h"
#include "confusionmatrix.h"

#ifdef OPENCL_DEFINED
#include "CL/cl.hpp"
#endif

class NeuralNet;
template struct Particle<NeuralNet::State>;

struct GlobalBestObject {

    GlobalBestObject() {}
    GlobalBestObject(const GlobalBestObject & l) : state(l.state), cm(l.cm) {}
    GlobalBestObject(GlobalBestObject && r) : state(std::move(r.state)), cm(std::move(r.cm)) {}

    GlobalBestObject & operator=(const GlobalBestObject & l) {
        state = l.state;
        cm = l.cm;
        return *this;
    }

    GlobalBestObject & operator=(GlobalBestObject && r) {
        state = std::move(r.state);
        cm = std::move(r.cm);
        return *this;
    }

    NeuralNet::State state;
    ConfusionMatrix cm;
};

struct FitnessParameters {
    real mse_weight;
    real mse_floor;
    TestStatistics::ClassificationError weights;
    TestStatistics::ClassificationError floors;

    real edgeWeightMax = 10E5L;
    real edgeWeightMin = -10E5L;

    bool enableTopologyTraining = false;
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

  void flySerial();
  void flyParallel();

  void evaluatePoints(std::vector<bool> & printChange);
  void findPersonalBest(std::vector<bool> & printChange);
  void findLocalBest(std::vector<bool> & printChange);
  void findGlobalBest(std::vector<bool> & printChange);

  void updatePersonalBest(NeuralParticle & p);
  void updateNeighborBest(NeuralParticle & p, NeuralParticle & p_n);
  void updateGlobalBest(NeuralParticle & p);

  virtual void trainingRun();
  virtual void testGb();
  virtual void validateGb();

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

  GlobalBestObject & getRecentGlobalBest() { return _recent_gb; }
  GlobalBestObject & getOverallBest() { return _best_overall_gb; }
  std::vector<GlobalBestObject> & validatedGbList() { return _validated_gb_list; }
  std::vector<GlobalBestObject> & selectedBestList() { return _selectedBestList; }

protected:
    NeuralNet *_neuralNet;
    FitnessParameters _fParams;

    GlobalBestObject _recent_gb;
    GlobalBestObject _best_overall_gb;
    std::vector<GlobalBestObject> _validated_gb_list;
    std::vector<GlobalBestObject> _selectedBestList;

private:
  static bool printGBFlag;
  static std::mutex printGBMtx;
  static volatile bool stopProcessing;

  real flyIteration(const size_t & particleId,
                    const size_t & inner_net,
                    const size_t & left_edge,
                    const size_t & right_edge,
                    const real & choice);

};

#endif // NEURALPSO_H
