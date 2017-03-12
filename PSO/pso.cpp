#include "pso.h"

template <class T>
Pso<T>::Pso(PsoParams p) :
    _psoParams(p)
{
  _particles.resize(p.particles);

}

template <class T>
Pso<T>::~Pso() {
  while (_particles.size() > 0) {
    _particles.erase(_particles.begin());
  }
}

template <class T>
void Pso<T>::run() {
  double cost = _gb._fit_pb;
  double prevCost = 0;
  uint32_t iterations = 0;

  do {

    fly();
    getCost();

    prevCost = cost;
    cost = _gb._fit_pb;

  } while ((_psoParams.termIterationFlag && (++iterations < _psoParams.iterations)) ||
           (_psoParams.termDeltaFlag && (cost - prevCost > _psoParams.termDeltaFlag)));
}
