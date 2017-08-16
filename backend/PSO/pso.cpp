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

  std::vector<double> history;

  do {

    _iterations = ++iterations; // On purpose.

    processEvents();
    fly();
    double cost = getCost();
    double prevCost = std::numeric_limits<double>::max();
    for (int i = 0; i < history.size(); i++) {
        prevCost = std::min(prevCost, history[i]);
    }
    history.push_back(cost);
    if (history.size() > _psoParams.window) {
        history.erase(history.begin());
    }

    double dif = (cost - prevCost);

    if ((dif < _psoParams.delta && _psoParams.termDeltaFlag)
            && (history.size() == _psoParams.window))
        break;

    if ((_psoParams.termIterationFlag) && (iterations > _psoParams.iterations))
        break;

  } while (!checkTermProcess());
  interruptProcess();
  cout << "Ending PSO Run.";
}

template <class T>
void Pso<T>::fly() {
}

template <class T>
double Pso<T>::getCost() {
    return 0;
}

template <class T>
void Pso<T>::processEvents() {
}

template <class T>
void Pso<T>::interruptProcess() {
//  boost::lock_guard<boost::mutex> guard(stopProcessMtx);
  _overideTermFlag = true;
}

template <class T>
bool Pso<T>::checkTermProcess() {
//  boost::lock_guard<boost::mutex> guard(stopProcessMtx);
  return _overideTermFlag;
}

template <class T>
void Pso<T>::resetProcess() {
//  boost::lock_guard<boost::mutex> guard(stopProcessMtx);
  _overideTermFlag = false;
}

template <class T>
bool Pso<T>::checkForPrint() {
//  boost::lock_guard<boost::mutex> guard(printMtx);
  if (_printFlag) {
    _printFlag = false;
    return true;
  } else {
    return false;
  }
}

template <class T>
void Pso<T>::setToPrint() {
//  boost::lock_guard<boost::mutex> guard(printMtx);
  _printFlag = true;
}
