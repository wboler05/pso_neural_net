#include "neuralpso.h"

#include "PSO/pso.cpp"
//#include "NeuralNet/NeuralNet.cpp"

volatile bool NeuralPso::stopProcessing = false;
bool NeuralPso::printGBFlag = false;
std::mutex NeuralPso::printGBMtx;

NeuralPso::NeuralPso(PsoParams pp, NeuralNet::NeuralNetParameters np, FitnessParameters fp) :
  Pso(pp),
  _neuralNet(new NeuralNet(np)),
  _fParams(fp)
{
  resetProcess();
}

NeuralPso::~NeuralPso() {
  delete _neuralNet;
}

void NeuralPso::buildPso() {
    _gb._fit_pb = -numeric_limits<real>::max();

    std::vector<real> innerWeightRange = {-1, 1};
//    std::vector<real> infWeightRange = { _fParams.edgeWeightMin,_fParams.edgeWeightMax };

    std::vector<real> constantRange = { 0, 10 };

    _particles->empty();
    const NeuralNet::State &state = _neuralNet->state();
//    size_t outputIt = state.size() / 2;
//    size_t outputIt = NeuralNet::totalEdgeLayersFromState(state);

    // Create N particles
    _particles->resize(_psoParams.population);
    for (size_t i = 0; i < _psoParams.population; i++) {
        _neuralNet->randomizeState();
        _neuralNet->randomizeActivationConstants();
        const NeuralNet::State & randomState = _neuralNet->state();

        // Create the number of inner columns
        (*_particles)[i]._fit_pb = -numeric_limits<real>::max();
        (*_particles)[i]._fit_lb = -numeric_limits<real>::max();

        //(*_particles)[i]._x.resize(state.size());
        (*_particles)[i]._x = randomState;
        (*_particles)[i]._minX.resize(state.size());
        (*_particles)[i]._maxX.resize(state.size());
        (*_particles)[i]._v.resize(state.size());
        (*_particles)[i]._x_pb.resize(state.size());
        (*_particles)[i]._x_lb.resize(state.size());
        (*_particles)[i]._worstFlag = false;
        (*_particles)[i]._points = _psoParams.startPoints;

        (*_particles)[i].totalEvents = 0;
        (*_particles)[i].noneCount = 0;
        (*_particles)[i].pbCount = 0;
        (*_particles)[i].lbCount = 0;
        (*_particles)[i].gbCount = 0;
        (*_particles)[i].noneFlag = true;

        if (i == 0) {
            _gb._x.resize(state.size()+1);
        }
        // Create each inner column edge
        for (size_t j = 0; j < state.size(); j++) {
            // Create left edges
            //(*_particles)[i]._x[j].resize(state[j].size());
            (*_particles)[i]._minX[j].resize(state[j].size());
            (*_particles)[i]._maxX[j].resize(state[j].size());
            (*_particles)[i]._v[j].resize(state[j].size());
            (*_particles)[i]._x_pb[j].resize(state[j].size());
            (*_particles)[i]._x_lb[j].resize(state[j].size());
            if (i == 0) {
                _gb._x[j].resize(state[j].size());
            }
            for (size_t k = 0; k < state[j].size(); k++) {
                // Create right edges
                //(*_particles)[i]._x[j][k].resize(state[j][k].size());
                (*_particles)[i]._minX[j][k].resize(state[j][k].size());
                (*_particles)[i]._maxX[j][k].resize(state[j][k].size());
                (*_particles)[i]._v[j][k].resize(state[j][k].size());
                (*_particles)[i]._x_pb[j][k].resize(state[j][k].size());
                (*_particles)[i]._x_lb[j][k].resize(state[j][k].size());
                if (i == 0) {
                    _gb._x[j][k].resize(state[j][k].size());
                }
                for (size_t m = 0; m < state[j][k].size(); m++) {
                    (*_particles)[i]._v[j][k][m] = _randomEngine.uniformReal(
                                innerWeightRange[0], innerWeightRange[1]);
                    (*_particles)[i]._x_pb[j][k][m] = 0;
                    (*_particles)[i]._x_lb[j][k][m] = 0;

                    if (i == 0) {
                        _gb._x[j][k][m] = 0;
                    }

                    if (j == state.size() -1) {
                        (*_particles)[i]._minX[j][k][m] = constantRange[0];
                        (*_particles)[i]._maxX[j][k][m] = constantRange[1];
                    } else {
                        (*_particles)[i]._minX[j][k][m] = innerWeightRange[0];
                        (*_particles)[i]._maxX[j][k][m] = innerWeightRange[1];
                    }
                }
            }
        }
    }
}

void NeuralPso::fly() {
    flySerial();
    //flyParallel();

}

void NeuralPso::flySerial() {
    real velSum = 0;

    if (_particles->size() == 0) return;
    if ((*_particles)[0]._v.size() == 0) return;

    static int innerNetAccessCount = _psoParams.iterationsPerLevel;
    static size_t innerNetIt = (*_particles)[0]._v.size()-1;

    real choice=0;
    bool worstFlag;

    const real C1 = 2.495L, C2 = 2.495L, C3 = 0.05L;
    real dt = 1;
    real vLimit = _psoParams.vLimit;

    // Random, pb, lb, gb
    static std::vector<real> pdf = {.001L, 2.0L, 8.0L, 4.0L};
    static std::vector<real> cdf = cdfUniform(pdf);

    std::vector<real> probRange = { 0.0L, 1.0L};
    std::vector<real> negRange =  {-1.0L, 1.0L};
    std::vector<real> inertiaRange = {0.5L, 1.0L};

    size_t startLayer = 0;
    if (!_fParams.enableTopologyTraining) {
        startLayer = 1;
    }

    // For each particle
    for (size_t i = 0; i < _particles->size(); i++) {
        Particle<NeuralNet::State> *p = &(*_particles)[i];
        //p->_worstFlag = false;
        worstFlag = p->_worstFlag;

        if (worstFlag) {
            p->_worstFlag = false;
            if (p->_points <= 0) {
                //choice = (real) (rand() % 10000) / 10000.0;
                //choice = dist(_randomEngine.engine());
                choice = _randomEngine.uniformReal(
                            probRange[0], probRange[1]);
                p->_fit_pb = -std::numeric_limits<real>::max();
                p->_fit_lb = -std::numeric_limits<real>::max();
                p->_points = _psoParams.startPoints;
                /*
                std::string callWeak;
                callWeak += "Switching particle ";
                callWeak += stringPut(i);
                callWeak += " with option ";
                callWeak += stringPut(choice);
                callWeak += "\n";
                Logger::write(callWeak);
                */

            } else {
                worstFlag = false;
                p->_points -= _psoParams.weakPoints;
            }
        }

        // For each inner net
        for (size_t inner_net = startLayer; inner_net < p->_v.size(); inner_net++) {
            if (checkTermProcess())
                return;

            if (_psoParams.backPropagation && (inner_net != innerNetIt)) {
                innerNetIt = 0;
            }
            // For each edge (left side) of that inner net

            //#pragma omp parallel for
            for (size_t left_edge = 0; left_edge < p->_v[inner_net].size(); left_edge++) {
                // For each edge (right side) of that inner net
                for (size_t right_edge = 0; right_edge < p->_v[inner_net][left_edge].size(); right_edge++) {
                    velSum += flyIteration(i, inner_net, left_edge, right_edge, choice);
                }
            }
        }
    }

    if (innerNetAccessCount-- == 0) {
        innerNetAccessCount = _psoParams.iterationsPerLevel;
        if (innerNetIt-- == 0) {
            innerNetIt = _neuralNet->nParams()->innerNetNodes.size()-1;
        }
    }

    // Never happens
    // If stagnant, mix it up a bit
    if (abs(velSum) < _psoParams.vDelta) {

        for (size_t i = 0; i < _particles->size(); i++) {
            // Reset bests
            (*_particles)[i]._fit_pb = -numeric_limits<real>::max();
            (*_particles)[i]._fit_lb = -numeric_limits<real>::max();
            for (size_t j = 0; j < (*_particles)[i]._x.size(); j++) {
                for (size_t k = 0; k < (*_particles)[i]._x[j].size(); k++) {
                    for (size_t m = 0; m < (*_particles)[i]._x[j][k].size(); m++) {
                        (*_particles)[i]._x[j][k][m] = _randomEngine.uniformReal(negRange[0], negRange[1]);
                    }
                }
            }
        }
    }

    /// For termination through user control
    if (stopProcessing) {
        interruptProcess();
    }

}

void NeuralPso::flyParallel() {
    const size_t num_threads = std::thread::hardware_concurrency() * 2;

    real velSum = 0;

    if (_particles->size() == 0) return;
    if ((*_particles)[0]._v.size() == 0) return;

    static int innerNetAccessCount = _psoParams.iterationsPerLevel;
    static size_t innerNetIt = (*_particles)[0]._v.size()-1;

    real choice=0;
    bool worstFlag;

    const real C1 = 2.495L, C2 = 2.495L, C3 = 0.05L;
    real dt = 1;
    real vLimit = _psoParams.vLimit;

    // Random, pb, lb, gb
    static std::vector<real> pdf = {.001L, 2.0L, 8.0L, 4.0L};
    static std::vector<real> cdf = cdfUniform(pdf);

    std::vector<real> probRange = { 0.0L, 1.0L};
    std::vector<real> negRange =  {-1.0L, 1.0L};
    std::vector<real> inertiaRange = {0.5L, 1.0L};

    size_t startLayer = 0;
    if (!_fParams.enableTopologyTraining) {
        startLayer = 1;
    }

    std::vector<std::thread> particleThreads;

    // For each particle
    for (size_t i = 0; i < _particles->size(); i++) {
        Particle<NeuralNet::State> *p = &(*_particles)[i];
        //p->_worstFlag = false;
        worstFlag = p->_worstFlag;

        if (worstFlag) {
            p->_worstFlag = false;
            if (p->_points <= 0) {
                //choice = (real) (rand() % 10000) / 10000.0;
                //choice = dist(_randomEngine.engine());
                choice = _randomEngine.uniformReal(
                            probRange[0], probRange[1]);
                p->_fit_pb = -std::numeric_limits<real>::max();
                p->_fit_lb = -std::numeric_limits<real>::max();
                p->_points = _psoParams.startPoints;
                /*
                std::string callWeak;
                callWeak += "Switching particle ";
                callWeak += stringPut(i);
                callWeak += " with option ";
                callWeak += stringPut(choice);
                callWeak += "\n";
                Logger::write(callWeak);
                */

            } else {
                worstFlag = false;
                p->_points -= _psoParams.weakPoints;
            }
        }

        // For each inner net
        for (size_t inner_net = startLayer; inner_net < p->_v.size(); inner_net++) {
            if (checkTermProcess())
                return;

            if (_psoParams.backPropagation && (inner_net != innerNetIt)) {
                innerNetIt = 0;
            }
            // For each edge (left side) of that inner net

            //#pragma omp parallel for
            for (size_t left_edge = 0; left_edge < p->_v[inner_net].size(); left_edge++) {
                // For each edge (right side) of that inner net
                for (size_t right_edge = 0; right_edge < p->_v[inner_net][left_edge].size(); right_edge++) {
                    particleThreads.push_back(std::thread(&NeuralPso::flyIteration, this,
                                i, inner_net, left_edge, right_edge, choice));
                }
            }
        }
    }

    /*
    if (particleThreads.size() > 0) {
        for (size_t i = 0; i <= (particleThreads.size() / num_threads); i++) {
            for (size_t j = 0; j < num_threads; j++) {
                size_t threadId = i * num_threads + j;
                if (threadId > particleThreads.size()) {
                    break;
                } else {
                    particleThreads[threadId].join();
                }
            }
        }
    }*/
    for (size_t t = 0; t < particleThreads.size(); t++) {
        particleThreads[t].join();
    }
    particleThreads.clear();

    if (innerNetAccessCount-- == 0) {
        innerNetAccessCount = _psoParams.iterationsPerLevel;
        if (innerNetIt-- == 0) {
            innerNetIt = _neuralNet->nParams()->innerNetNodes.size()-1;
        }
    }

    // Never happens
    // If stagnant, mix it up a bit
    if (abs(velSum) < _psoParams.vDelta) {

        for (size_t i = 0; i < _particles->size(); i++) {
            // Reset bests
            (*_particles)[i]._fit_pb = -numeric_limits<real>::max();
            (*_particles)[i]._fit_lb = -numeric_limits<real>::max();
            for (size_t j = 0; j < (*_particles)[i]._x.size(); j++) {
                for (size_t k = 0; k < (*_particles)[i]._x[j].size(); k++) {
                    for (size_t m = 0; m < (*_particles)[i]._x[j][k].size(); m++) {
                        (*_particles)[i]._x[j][k][m] = _randomEngine.uniformReal(negRange[0], negRange[1]);
                    }
                }
            }
        }
    }

    /// For termination through user control
    if (stopProcessing) {
        interruptProcess();
    }
}

real NeuralPso::flyIteration(const size_t & particleId,
                             const size_t & inner_net,
                             const size_t & left_edge,
                             const size_t & right_edge,
                             const real & choice)
{
    NeuralParticle * p = &(*_particles)[particleId];
    const std::vector<real> probRange = { 0.0L, 1.0L};
    const std::vector<real> negRange =  {-1.0L, 1.0L};
    const std::vector<real> inertiaRange = {0.5L, 1.0L};
    const std::vector<real> pdf = {.001L, 2.0L, 8.0L, 4.0L};
    const std::vector<real> cdf = cdfUniform(pdf);
    const real C1 = 2.495L, C2 = 2.495L, C3 = 0.05L;

    if (p->_worstFlag) { // Reset the worst one
        if (choice < cdf[0]) {
            //p->_x[inner_net][left_edge][right_edge] = negPosRange(_randomEngine.engine());
            p->_x[inner_net][left_edge][right_edge] =
                    _randomEngine.uniformReal(negRange[0], negRange[1]);
        } else if (choice < cdf[1]) {
            p->_x[inner_net][left_edge][right_edge] = p->_x_pb[inner_net][left_edge][right_edge];
        } else if (choice < cdf[2]) {
            p->_x[inner_net][left_edge][right_edge] = p->_x_lb[inner_net][left_edge][right_edge];
        } else {
            p->_x[inner_net][left_edge][right_edge] = _gb._x[inner_net][left_edge][right_edge];
        }
        //p->_v[inner_net][left_edge][right_edge] = negPosRange(_randomEngine.engine());
        p->_v[inner_net][left_edge][right_edge] =
                _randomEngine.uniformReal(negRange[0], negRange[1]);
        return p->_v[inner_net][left_edge][right_edge];
    }

    real *w_v = &p->_v[inner_net][left_edge][right_edge];
    real *w_x = &p->_x[inner_net][left_edge][right_edge];
    real *w_pb = &p->_x_pb[inner_net][left_edge][right_edge];
    real *w_lb = &p->_x_lb[inner_net][left_edge][right_edge];
//    real *w_gb = &_gb._x[inner_net][left_edge][right_edge];
    real *w_x_min = &p->_minX[inner_net][left_edge][right_edge];
    real *w_x_max = &p->_maxX[inner_net][left_edge][right_edge];

    real inertia = _randomEngine.uniformReal(inertiaRange[0], inertiaRange[1]);
    real c1 = C1 * _randomEngine.uniformReal(probRange[0], probRange[1]);
    real c2 = C2 * _randomEngine.uniformReal(probRange[0], probRange[1]);
//    real c3 = C3 * _randomEngine.uniformReal(probRange[0], probRange[1]);

    *w_v = (
                inertia * (*w_v)
                + (c1*(*w_pb - *w_x))
                + (c2*(*w_lb - *w_x))
//                + (c3*(*w_gb - *w_x))
                );

    if (*w_v > _psoParams.vLimit) {
        *w_v = _psoParams.vLimit;
    } else if (*w_v < -_psoParams.vLimit) {
        *w_v = -_psoParams.vLimit;
    }

    *w_x += *w_v;

    *w_x = std::min(*w_x, *w_x_max);
    *w_x = std::max(*w_x, *w_x_min);

    return *w_v;
}


/**
 * @brief NeuralPso::getCost
 * @details Update each particle's current cost at current position
 */
void NeuralPso::getCost() {
    trainingRun();
} // end getCost()

/**
 * @brief NeuralPso::evaluate
 * @details Evaulate personal, local, and global bests
 */
real NeuralPso::evaluate() {
    real correctRatio=0;
//    int totalCount=0;

    std::vector<bool> printChange;
    printChange.resize(_particles->size(), false);

//    static real prevBest = 0;

    evaluatePoints(printChange);
    findPersonalBest(printChange);
    findLocalBest(printChange);
    findGlobalBest(printChange);

    // Print changes
    for (size_t i = 0; i < _particles->size(); i++) {
        NeuralParticle *p = &(*_particles)[i];

        // Handle logging
        if (printChange[i]) { // Disable this for now
            printChange[i] = false;
            std::string outputString;
            outputString += "Particle (";
            outputString += stringPut(i);
            outputString += "):: Fit: ";
            outputString += stringPut(p->_fit);
            outputString += "\tPB: ";
            outputString += stringPut(p->_fit_pb);
            outputString += "\tLB: ";
            outputString += stringPut(p->_fit_lb);
            outputString += "\tGB: ";
            outputString += stringPut(gb()->_fit_pb);
            outputString += "\tCor: ";
            outputString += stringPut(correctRatio);
            outputString += "\tPts: ";
            outputString += stringPut(p->_points);
            outputString += "\n";
            Logger::write(outputString);

        }
    } // end for each particle

    printGBMtx.lock();
    if (printGBFlag) {
        printGBFlag = false;
        printGB();
    }
    printGBMtx.unlock();

    testGB(); // This needs to be validateGB()
    return gb()->_fit_pb;
}

/**
 * @brief NeuralPso::evaluatePoints
 */
void NeuralPso::evaluatePoints(std::vector<bool> & printChange) {
    real worstFit = numeric_limits<real>::max();
    size_t worstFitIt = 0;

    for (size_t i = 0; i < _particles->size(); i++) {
        // Reset flag
        (*_particles)[i]._worstFlag = false;

        // Track worst fitness
        if (worstFit >= (*_particles)[i]._fit) {
          worstFit = (*_particles)[i]._fit;
          worstFitIt = i;
        }

        // Apply decay
        (*_particles)[i]._points -= _psoParams.decayPoints;

        // Negative points force particle updates
        if ((*_particles)[i]._points < 0) {
            (*_particles)[i]._worstFlag = true;
        }
    }
    // Set the worst fit flag for particle
    (*_particles)[worstFitIt]._worstFlag = true;

    for (size_t i = 0; i < _particles->size(); i++) {
        //printChange[i] = (*_particles)[i]._worstFlag;
    }
}

/**
 * @brief NeuralPso::findPersonalBest
 */
void NeuralPso::findPersonalBest(std::vector<bool> & printChange) {
    // Find personal best for each particle
    for (size_t i = 0; i < _particles->size(); i++) {
        _particles->at(i).noneFlag = true;
        _particles->at(i).totalEvents++;
        // Check if personal best has surpassed remembered fitness
        if ((*_particles)[i]._fit > (*_particles)[i]._fit_pb) {
            (*_particles)[i]._fit_pb = (*_particles)[i]._fit;
            (*_particles)[i]._points += _psoParams.pbPoints;
            //printChange[i] = true;
            updatePersonalBest((*_particles)[i]);
            _particles->at(i).pbCount++;
        }
    }
}

void NeuralPso::updatePersonalBest(NeuralParticle & p) {
    // Update personal best location to memory
    for (size_t i = 0; i < p._x_pb.size(); i++) {
        for (size_t j = 0; j < p._x_pb[i].size(); j++) {
            for (size_t k = 0; k < p._x_pb[i][j].size(); k++) {
                p._x_pb[i][j][k] = p._x[i][j][k];
            }
        }
    }
}

/**
 * @brief NeuralPso::findLocalBest
 * @details Update all the local bests for each particle
 * @return
 */
void NeuralPso::findLocalBest(std::vector<bool> & printChange) {
    for (size_t i = 0; i < _particles->size(); i++) {
        NeuralParticle &p = (*_particles)[i];
        bool goodNeighbor = false;

        int left_i = static_cast<int> (i - (_psoParams.neighbors / 2));
        if (left_i < 0) left_i += _particles->size();

        size_t bestNeighborIt = i;
        for (size_t j = 0; j < _psoParams.neighbors; j++) {
            int it = left_i + static_cast<int>(j);
            if (it < 0) {
                it += _particles->size();
            } else if (it >= static_cast<int>(_particles->size())) {
                it -= _particles->size();
            }

            NeuralParticle &p_n = (*_particles)[static_cast<size_t>(it)];
            if (p_n._fit_pb > p._fit_lb) {
                p._fit_lb = p_n._fit_pb;
                goodNeighbor = true;
                bestNeighborIt = static_cast<size_t>(it);
            }
        }

        NeuralParticle &p_n = (*_particles)[bestNeighborIt];
        if (goodNeighbor) {
            //printChange[i] = true;
            updateNeighborBest(p, p_n);
        }
    }
}

/**
 * @brief NeuralPso::updateNeighborBest
 * @details Update neighbor best for current particle
 * @param p     - Current Particle
 * @param p_n   - Best neighbor Particle
 */
void NeuralPso::updateNeighborBest(NeuralParticle & p, NeuralParticle & p_n) {
    //p._fit_lb = p_n._fit_pb;
    p._points += _psoParams.lbPoints;
    p.lbCount++;
    for (size_t i = 0; i < p_n._x_lb.size(); i++) {
        for (size_t j = 0; j < p_n._x_lb[i].size(); j++) {
            for (size_t k = 0; k < p_n._x_lb[i][j].size(); k++) {
                p._x_lb[i][j][k] = p_n._x_pb[i][j][k];
            } // for k
        } // for j
    } // for i
} // updateNeighborBest

void NeuralPso::findGlobalBest(std::vector<bool> & printChange) {
    int globalBestIt=-1;
    for (size_t i = 0; i < _particles->size(); i++) {
        NeuralParticle & p = (*_particles)[i];
        if (p._fit_pb > _gb._fit_pb) {
            _gb._fit_pb = p._fit_pb;
            globalBestIt = static_cast<int>(i);
        } // End global best
    }

    if (globalBestIt >= 0) {
        NeuralParticle & p = (*_particles)[static_cast<size_t>(globalBestIt)];
        printChange[static_cast<size_t>(globalBestIt)] = true;
        updateGlobalBest(p);
    }
}

void NeuralPso::updateGlobalBest(NeuralParticle &p) {
    //_gb._fit_pb = p._fit_pb;
    p._points += _psoParams.gbPoints;
    p.gbCount++;
    for (size_t i = 0; i < p._x.size(); i++) {
        for (size_t j = 0; j < p._x[i].size(); j++) {
            for (size_t k = 0; k < p._x[i][j].size(); k++) {
                _gb._x[i][j][k] = p._x[i][j][k];
            }
        }
    }
}

void NeuralPso::trainingRun() {
    std::cout << "Error, must be implemented via inherited training class." << std::endl;
    return;
}

void NeuralPso::testGB() {
    std::cout << "Error, must be implemented via inherited training class." << std::endl;
}

void NeuralPso::processEvents() {
    // Specific for Qt. Can be removed in other API.
    qApp->processEvents();
}

void NeuralPso::printGB() {
  std::string printString;
  printString += "Global Best: \n";
  for (size_t i = 0; i < _gb._x.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (size_t j = 0; j < gb()->_x[i].size(); j++) {
      for (size_t k = 0; k < gb()->_x[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut(gb()->_x[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

void NeuralPso::printParticle(size_t I) {
  if (I > _particles->size()) return;

  std::string printString;
  printString += "Particle (";
  printString += stringPut(I);
  printString += "): \n";
  for (size_t i = 0; i < (*_particles)[I]._x.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (size_t j = 0; j < (*_particles)[I]._x[i].size(); j++) {
      for (size_t k = 0; k < (*_particles)[I]._x[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut((*_particles)[I]._x[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

void NeuralPso::printParticlePBest(size_t I) {
  if (I > _particles->size()) return;

  std::string printString;
  printString += "Particle pBest (";
  printString += stringPut(I);
  printString += "): \n";
  for (size_t i = 0; i < (*_particles)[I]._x_pb.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (size_t j = 0; j < (*_particles)[I]._x_pb[i].size(); j++) {
      for (size_t k = 0; k < (*_particles)[I]._x_pb[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut((*_particles)[I]._x_pb[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

void NeuralPso::printParticleLBest(size_t I) {
  if (I > _particles->size()) return;

  std::string printString;
  printString += "Particle lBest (";
  printString += stringPut(I);
  printString += "): \n";
  for (size_t i = 0; i < (*_particles)[I]._x_lb.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (size_t j = 0; j < (*_particles)[I]._x_lb[i].size(); j++) {
      for (size_t k = 0; k < (*_particles)[I]._x_lb[i][j].size(); k++) {
        printString += "  -- ";
        printString += stringPut(j+1);
        printString += " : ";
        printString += stringPut(k+1);
        printString += " = ";
        printString += stringPut((*_particles)[I]._x_lb[i][j][k]);
        printString += "\n";
      }
    }
  }
  Logger::write(printString);
}

bool NeuralPso::injectGb(const NeuralNet::State &s) {
    if (_particles->size() < 1) {
        return false;
    }

    NeuralNet::State & px = (*_particles)[0]._x;

    if (s.size() != px.size()) {
        return false;
    }

    for (size_t inner_node = 0; inner_node < px.size(); inner_node++) {
        if (s.at(inner_node).size() != px[inner_node].size()) {
            return false;
        }
        for (size_t left_node = 0; left_node < px[inner_node].size(); left_node++) {
            if (s.at(inner_node).at(left_node).size() != px[inner_node][left_node].size()) {
                return false;
            }
            for (size_t right_node = 0; right_node < px[inner_node][left_node].size(); right_node++) {
                    px[inner_node][left_node][right_node] = s.at(inner_node).at(left_node).at(right_node);
            }
        }
    }
    return true;
}

void NeuralPso::setToPrintGBNet() {
  printGBMtx.lock();
  printGBFlag = true;
  printGBMtx.unlock();
}

std::unique_ptr<NeuralNet> NeuralPso::buildNeuralNetFromGb() {
    NeuralNet::State & gbState = getGbState();
    std::unique_ptr<NeuralNet> n = std::make_unique<NeuralNet>(*neuralNet()->nParams(), gbState);
    return n;
}

NeuralNet::State &NeuralPso::getGbState() {
    return gb()->_x;
}

std::string NeuralPso::stringifyState() {

    using namespace NeuralPsoStream;

    std::string stringState;

    stringState.append(stringifyPParams(psoParams()));
    stringState.append(stringifyNParams(*_neuralNet->nParams()));

    // Get Global Best
    stringState.append(openToken("_gb"));
    stringState.append("\n");
    stringState.append(stringifyParticle(_gb));
    stringState.append(closeToken("_gb"));
    stringState.append("\n");

    // Get Particles
    stringState.append(openToken("_particles"));
    stringState.append("\n");
    for (size_t i = 0; i < _particles->size(); i++) {
        stringState.append("\n");
        stringState.append(openToken(stringPut(i)));
        stringState.append("\n");

        stringState.append(stringifyParticle((*_particles)[i]));

        stringState.append("\n");
        stringState.append(closeToken(stringPut(i)));
        stringState.append("\n");
    }
    stringState.append(closeToken("_particles"));
    stringState.append("\n");

    return stringState;
}

bool NeuralPso::loadStatefromString(const string &psoState) {
    //!TEST!//
    using namespace NeuralPsoStream;

    std::string cleanString = psoState;
    cleanInputString(cleanString);

    int it = 0;

    if (!findNextToken(cleanString, it)) {
        return false;
    }

    std::string psoParamString = subStringByToken(cleanString, "PsoParams", it);
    if (psoParamString.size() == 0) return false;
    PsoParams newPParams = psoParametersFromString(psoParamString);
    _psoParams = newPParams;

    std::string nParamString = subStringByToken(cleanString, "NeuralNetParameters", it);
    if (nParamString.size() == 0) return false;
    NeuralNet::NeuralNetParameters newNParams = nParametersFromString(nParamString);
    _neuralNet->initialize(newNParams);

    std::string gbString = subStringByToken(cleanString, "_gb", it);
    if (gbString.size() == 0) return false;

    Particle<NeuralNet::State> gb =
            particleFromString(gbString);
    _gb = gb;

    std::string partString = subStringByToken(cleanString, "_particles", it);
    *_particles = readParticlesFromString(partString);

    return true;
}
