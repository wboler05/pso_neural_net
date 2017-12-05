#include "pso.h"

#include <QTime>

template <class T>
bool Pso<T>::_overideTermFlag;
template <class T>
std::mutex Pso<T>::stopProcessMtx;
template <class T>
bool Pso<T>::_printFlag=false;
template <class T>
std::mutex Pso<T>::printMtx;
template <class T>
RandomNumberEngine Pso<T>::_randomEngine(
        std::chrono::system_clock::now().time_since_epoch().count());

template <class T>
Pso<T>::Pso(PsoParams p) :
    _psoParams(p)
{
    _particles = std::make_shared<std::vector<Particle<T>> > ();
    _particles->resize(p.population);
    // Initialize to true
    // Call "resetProcess()" from inheriting class.
    interruptProcess();
}

template <class T>
Pso<T>::~Pso() {
  while (_particles->size() > 0) {
    _particles->erase(_particles->begin());
  }
}

template <class T>
void Pso<T>::run() {
  size_t epochs = _epochs;

  resetProcess();
  _history.clear();

  do {

      //std::string msg;
      //msg.append("Epochs: " );
      //msg.append(stringPut(epochs));
      //msg.append("\n");
      //Logger::write(msg);

    _epochs = ++epochs;     // Count the iterations

 //   processEvents();        // Virtual function for GUI updates
    fly();                  // Fly the particles
    getCost();              // Establish particle cost
    real cost = evaluate(); // Evaluate pb, lb, gb

    // Push current cost to history
    try {
        _history.push_back(cost);
    } catch (const std::bad_alloc &e) {
        Logger::write("Error, bad allocation: Pso::run()\n");
        break;
    }

    // Terminate based on delta
    if (getDelta() < _psoParams.delta && _psoParams.termDeltaFlag) {
        if ((epochs >= _psoParams.minEpochs && _psoParams.termMinEpochsFlag) ||
                !_psoParams.termMinEpochsFlag)
        {
            break;
        }
    }

    // Terminate based on maxEpochs
    if ((_psoParams.termMaxEpochsFlag) && (epochs >= _psoParams.maxEpochs))
        break;

  } while (!checkTermProcess());  // Check for user interrupt
  interruptProcess();   // Reset the interrupt flag

  //Logger::write("Ending PSO Run.\n");
}

template <class T>
real Pso<T>::getDelta() {

    real lowCost = std::numeric_limits<real>::max();
    real highCost = -std::numeric_limits<real>::max();

    // Get the iteration for the beginning of the window
    size_t minWindowIt = _history.size() <= _psoParams.windowSize
            ? 0 : _history.size() - _psoParams.windowSize - 1;

    // Find the low and hi within the history
    for (size_t i = minWindowIt; i < _history.size() && _psoParams.termDeltaFlag; i++) {
        lowCost = std::min(lowCost, _history[i]);
        highCost = std::max(highCost, _history[i]);
    }
    return highCost - lowCost;
}

template <class T>
void Pso<T>::fly() {
}

template <class T>
void Pso<T>::getCost() {
}

template <class T>
real Pso<T>::evaluate() {
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
