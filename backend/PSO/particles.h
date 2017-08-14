#ifndef PARTICLES_H
#define PARTICLES_H

template <class T>
struct Particle {
  T _x;
  T _v;

  T _x_pb;
  T _x_lb;

  double _fit_pb;
  double _fit_lb;
  bool _worstFlag;
  int _points;
};


#endif // PARTICLES_H
