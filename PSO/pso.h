#ifndef PSO_H
#define PSO_H

#include <iostream>
#include <cinttypes>
#include <cstdlib>
#include <vector>
#include "particles.h"

template <class T>
class Pso {
public:
  Pso();
  ~Pso();

  void run();
  virtual void fly();
  virtual double getCost();

  std::vector<Particle<T> > * particles() { return &_particles; }

private:
  std::vector<Particle<T> > _particles;

  bool _terminationIterationFlag;
  bool _terminationDeltaFlag;

  uint32_t _terminationIterations;
  double _terminationDelta;

};


#endif // PSO_H
