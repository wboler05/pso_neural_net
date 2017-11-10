#ifndef PSO_H
#define PSO_H

#include <iostream>
#include <cinttypes>
#include <cstdlib>
#include <vector>
#include <queue>
#include "particles.h"
#include <limits>
#include <thread>
#include <mutex>

#include "custommath.h"
#include "util.h"
#include "randomnumberengine.h"

struct PsoParams {
  size_t population=50;
  size_t neighbors=5;
  size_t minEpochs = 10;
  size_t maxEpochs=100;
  real delta=5E-3L;
  real vDelta=5E-20L;
  real vLimit = 0.75L;
  size_t windowSize = 50;
  real dt = 0.99;

  bool termIterationFlag;
  bool termDeltaFlag;

  ///TODO These NeuralPso parameters
  bool backPropagation=true;
  int iterationsPerLevel=100;

  // Points to assign to prevent disruption
  int startPoints = 50000;
  int pbPoints = 100;
  int lbPoints = 5000;
  int gbPoints = 10000;
  int weakPoints = 1; // Points to lose for being the weakest
  int decayPoints = 50;

};

template <class T>
class Pso {
public:
  Pso(PsoParams p);
  virtual ~Pso();

  void run();
  virtual void fly();           // Fly particles
  virtual void getCost();       // Calculate fitness
  virtual real evaluate();
  virtual void processEvents(); // Process events

  real getDelta();

  const std::shared_ptr<std::vector<Particle<T> > > & particles() { return _particles; }
  Particle<T> * gb() { return &_gb; }

  static void interruptProcess();
  static bool checkTermProcess();
  static void resetProcess();

  static void setToPrint();
  static bool checkForPrint();

  const size_t & epochs() const { return _epochs; }

  const PsoParams & psoParams() { return _psoParams; }

  const std::vector<real> & historyFromLastRun() { return _history; }

  static RandomNumberEngine _randomEngine;

protected:
  std::shared_ptr<std::vector<Particle<T> > > _particles;
  Particle<T> _gb;

  PsoParams _psoParams;
  size_t _epochs=0;

private:
  std::vector<real> _history;

  static bool _overideTermFlag;
  static std::mutex stopProcessMtx;
  static bool _printFlag;
  static std::mutex printMtx;

};


#endif // PSO_H
