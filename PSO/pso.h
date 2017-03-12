#ifndef PSO_H
#define PSO_H

#include <iostream>
#include <cinttypes>
#include <cstdlib>
#include <vector>
#include "particles.h"

struct PsoParams {
  uint32_t particles;
  uint32_t neighbors;
  uint32_t iterations;
  double delta;

  bool termIterationFlag;
  bool termDeltaFlag;

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

protected:
  std::vector<Particle<T> > _particles;
  Particle<T> _gb;

  PsoParams _psoParams;

};


#endif // PSO_H
