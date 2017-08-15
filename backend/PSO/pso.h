#ifndef PSO_H
#define PSO_H

#include <iostream>
#include <cinttypes>
#include <cstdlib>
#include <vector>
#include "particles.h"
#include <boost/thread.hpp>

struct PsoParams {
  uint32_t particles=50;
  uint32_t neighbors=5;
  uint32_t iterations=100;
  double delta=5E-3;
  double vDelta=5E-20;
  double vLimit = 0.5;

  bool termIterationFlag;
  bool termDeltaFlag;

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
  virtual void getCost();       // Calculate fitness
  virtual void processEvents(); // Process events

  std::vector<Particle<T> > * particles() { return &_particles; }
  Particle<T> * gb() { return &_gb; }

  static void interruptProcess();
  static bool checkTermProcess();
  static void resetProcess();

  static void setToPrint();
  static bool checkForPrint();

protected:
  std::vector<Particle<T> > _particles;
  Particle<T> _gb;

  PsoParams _psoParams;

private:
  static bool _overideTermFlag;
  static boost::mutex stopProcessMtx;
  static bool _printFlag;
  static boost::mutex printMtx;

};


#endif // PSO_H
