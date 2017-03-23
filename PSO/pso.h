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

  bool termIterationFlag;
  bool termDeltaFlag;

  bool backPropagation=true;
  int iterationsPerLevel=100;

  // Points to assign to prevent disruption
  int startPoints = 5;
  int pbPoints = 1;
  int lbPoints = 3;
  int gbPoints = 5;
  int weakPoints = 1; // Points to lose for being the weakest

};

template <class T>
class Pso {
public:
  Pso(PsoParams p);
  ~Pso();

  void run();
  virtual void fly();       // Fly particles
  virtual void getCost();   // Calculate fitness

  std::vector<Particle<T> > * particles() { return &_particles; }
  Particle<T> * gb() { return &_gb; }

  static void interruptProcess();
  static bool checkProcess();
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
