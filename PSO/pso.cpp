#include "pso.h"

template <class T>
Pso<T>::Pso() {

}

template <class T>
Pso<T>::~Pso() {
  while (_particles.size() > 0) {
    _particles.erase(_particles.begin());
  }
}

template <class T>
void Pso<T>::run() {
  double cost = 0;
  double prevCost = 0;
  uint32_t iterations = 0;

  do {
    prevCost = cost;
    fly();
    cost = getCost();

  } while ((_terminationIterationFlag && (++iterations < _terminationIterations)) ||
           (_terminationDeltaFlag && (cost - prevCost > _terminationDelta)));
}
