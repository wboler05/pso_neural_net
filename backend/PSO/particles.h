#ifndef PARTICLES_H
#define PARTICLES_H

#include "custommath.h"

template <class T>
struct Particle {
  T _x;
  T _v;

  T _x_pb;
  T _x_lb;

  real _fit_pb;
  real _fit_lb;
  bool _worstFlag;
  int _points;
};


#endif // PARTICLES_H
