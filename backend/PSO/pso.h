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

struct PsoParams {
  uint32_t particles=50;
  uint32_t neighbors=5;
  uint32_t iterations=100;
  real delta=5E-3;
  real vDelta=5E-20;
  real vLimit = 0.5;
  uint32_t window = 50;

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
  int decayPoints = 1;

};

template <class T>
class Pso {
public:
  Pso(PsoParams p);
  ~Pso();

  void run();
  virtual void fly();           // Fly particles
  virtual real getCost();       // Calculate fitness
  virtual void processEvents(); // Process events

  std::vector<Particle<T> > * particles() { return &_particles; }
  Particle<T> * gb() { return &_gb; }

  static void interruptProcess();
  static bool checkTermProcess();
  static void resetProcess();

  static void setToPrint();
  static bool checkForPrint();

  const uint32_t & iterations() const { return _iterations; }

  const PsoParams & psoParams() { return _psoParams; }

  const std::vector<real> & historyFromLastRun() { return _history; }

protected:
  std::vector<Particle<T> > _particles;
  Particle<T> _gb;

  PsoParams _psoParams;

private:
  uint32_t _iterations=0;
  std::vector<real> _history;

  static bool _overideTermFlag;
  static std::mutex stopProcessMtx;
  static bool _printFlag;
  static std::mutex printMtx;

};


#endif // PSO_H
