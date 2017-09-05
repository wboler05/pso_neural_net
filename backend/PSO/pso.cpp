#include "pso.h"

template <class T>
bool Pso<T>::_overideTermFlag;
template <class T>
std::mutex Pso<T>::stopProcessMtx;
template <class T>
bool Pso<T>::_printFlag=false;
template <class T>
std::mutex Pso<T>::printMtx;

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

  std::vector<real> history;

  resetProcess();

  do {

    _iterations = ++iterations; // On purpose.

    processEvents();
    fly();
    real cost = getCost();
    real lowCost = std::numeric_limits<real>::max();
    real highCost = cost;
    for (size_t i = 0; i < history.size() && _psoParams.termDeltaFlag; i++) {
        lowCost = std::min(lowCost, history[i]);
        highCost = std::max(highCost, history[i]);
    }
    history.push_back(cost);
    if (history.size() > _psoParams.window) {
        history.erase(history.begin());
    }

    real dif = (highCost - lowCost);

    if ((dif < _psoParams.delta && _psoParams.termDeltaFlag)
            && (history.size() == _psoParams.window))
        break;

    if ((_psoParams.termIterationFlag) && (iterations >= _psoParams.iterations))
        break;

  } while (!checkTermProcess());
  interruptProcess();
  cout << "Ending PSO Run.";
}

template <class T>
void Pso<T>::fly() {
}

template <class T>
real Pso<T>::getCost() {
    return 0;
}

template <class T>
void Pso<T>::processEvents() {
}

template <class T>
void Pso<T>::interruptProcess() {
    std::unique_lock<std::mutex> lock1(stopProcessMtx, std::defer_lock);
    lock1.lock();
    _overideTermFlag = true;
}

template <class T>
bool Pso<T>::checkTermProcess() {
    std::unique_lock<std::mutex> lock1(stopProcessMtx, std::defer_lock);
    lock1.lock();
    return _overideTermFlag;
}

template <class T>
void Pso<T>::resetProcess() {
    std::unique_lock<std::mutex> lock1(stopProcessMtx, std::defer_lock);
    lock1.lock();
    _overideTermFlag = false;
}

template <class T>
bool Pso<T>::checkForPrint() {
    std::unique_lock<std::mutex> lock1(printMtx, std::defer_lock);
    lock1.lock();
    if (_printFlag) {
        _printFlag = false;
        return true;
    } else {
        return false;
    }
}

template <class T>
void Pso<T>::setToPrint() {
    std::unique_lock<std::mutex> lock1(printMtx, std::defer_lock);
    lock1.lock();
    _printFlag = true;
}
