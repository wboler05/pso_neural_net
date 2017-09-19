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
  size_t iterations = 0;
  real lowCost = 0;
  real highCost = 0;

  resetProcess();
  _history.clear();

  do {

    _iterations = ++iterations;                         // Count the iterations

    processEvents();                                    // Virtual function for GUI updates
    fly();                                              // Fly the particles
    real cost = getCost();                              // Get the cost of GB

    // Init low and hi vals
    lowCost = std::numeric_limits<real>::max();
    highCost = cost;

    // Get the iteration for the beginning of the window
    size_t minWindowIt = _history.size() <= _psoParams.window
            ? 0 : _history.size() - _psoParams.window - 1;

    // Find the low and hi within the history
    for (size_t i = minWindowIt; i < _history.size() && _psoParams.termDeltaFlag; i++) {
        lowCost = std::min(lowCost, _history[i]);
        highCost = std::max(highCost, _history[i]);
    }

    // Push current cost to history
    try {
        _history.push_back(cost);
    } catch (const std::bad_alloc &e) {
        Logger::write("Error, bad allocation: Pso::run()\n");
        break;
    }

    // Get the delta
    real dif = (highCost - lowCost);

    // If delta is less than the setting and flag is set, break
    if ((dif < _psoParams.delta && _psoParams.termDeltaFlag)
            && (_history.size() >= _psoParams.window))
        break;

    // If iterations surpassed and flag is set, break
    if ((_psoParams.termIterationFlag) && (iterations >= _psoParams.iterations))
        break;

  } while (!checkTermProcess());  // Check for user interrupt
  interruptProcess();   // Reset the interrupt flag

  Logger::write("Ending PSO Run.\n");
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
