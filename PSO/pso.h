#ifndef PSO_H
#define PSO_H

#include <iostream>
#include <cinttypes>
#include <cstdlib>

template <class T>
class Pso {
public:
  PSO();
  ~PSO();

  build();

  run();
  fly();
  getCost();

  vector<Particle<T> > * particles() { return &_particles; }

private:
  vector<Particle<T> > _particles;

};


#endif // PSO_H
