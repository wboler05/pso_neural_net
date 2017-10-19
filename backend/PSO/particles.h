#ifndef PARTICLES_H
#define PARTICLES_H

#include "custommath.h"

template <class T>
struct Particle {
  T _x;
  T _v;

  T _minX;
  T _maxX;

  T _x_pb;
  T _x_lb;

  int totalEvents = 0;
  int noneCount = 0;
  int noneFlag = true;
  int pbCount = 0;
  int lbCount = 0;
  int gbCount = 0;

  real _fit;
  real _fit_pb;
  real _fit_lb;
  bool _worstFlag;
  int _points;
};


#endif // PARTICLES_H
