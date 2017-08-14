#include "pso.h"

template <class T>
bool Pso<T>::_overideTermFlag;
template <class T>
boost::mutex Pso<T>::stopProcessMtx;
template <class T>
bool Pso<T>::_printFlag=false;
template <class T>
boost::mutex Pso<T>::printMtx;

template <class T>
Pso<T>::Pso(PsoParams p) :
    _psoParams(p)
{
  _particles.resize(p.particles);
  // Initialize to true
  // Call "resetProcess()" from inheriting class.
  interruptProcess();

}

template <class T>
Pso<T>::~Pso() {
  while (_particles.size() > 0) {
    _particles.erase(_particles.begin());
  }
}

template <class T>
void Pso<T>::run() {
  uint32_t iterations = 0;

  do {

    fly();
    getCost();
    //processEvents();

  } while ((_psoParams.termIterationFlag && (++iterations < _psoParams.iterations)) ||
           (!checkProcess()));
  interruptProcess();
}

template <class T>
void Pso<T>::fly() {
}

template <class T>
void Pso<T>::getCost() {
}

template <class T>
void Pso<T>::interruptProcess() {
  boost::lock_guard<boost::mutex> guard(stopProcessMtx);
  _overideTermFlag = true;
}

template <class T>
bool Pso<T>::checkProcess() {
  boost::lock_guard<boost::mutex> guard(stopProcessMtx);
  return _overideTermFlag;
}

template <class T>
void Pso<T>::resetProcess() {
  boost::lock_guard<boost::mutex> guard(stopProcessMtx);
  _overideTermFlag = false;
}

template <class T>
bool Pso<T>::checkForPrint() {
  boost::lock_guard<boost::mutex> guard(printMtx);
  if (_printFlag) {
    _printFlag = false;
    return true;
  } else {
    return false;
  }
}

template <class T>
void Pso<T>::setToPrint() {
  boost::lock_guard<boost::mutex> guard(printMtx);
  _printFlag = true;
}
