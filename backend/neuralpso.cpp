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

    //std::uniform_real_distribution<real> dist(-1, 1);
    //std::uniform_real_distribution<real> infDist(-std::numeric_limits<real>::max(),
    //                                             std::numeric_limits<real>::max());
    std::vector<real> innerWeightRange = {-1, 1};
    std::vector<real> infWeightRange = {-std::numeric_limits<real>::max()*.1,
                                        std::numeric_limits<real>::max()*.1};

    _particles->empty();
    const NeuralNet::EdgeType &edges = _neuralNet->getWeights();
    const NeuralNet::RecEdgeType & rEdges = _neuralNet->getRecWeights();

    // Create N particles
    _particles->resize(_psoParams.population);
    for (uint i = 0; i < _psoParams.population; i++) {
        // Create the number of inner columns
        (*_particles)[i]._fit_pb = -numeric_limits<real>::max();
        (*_particles)[i]._fit_lb = -numeric_limits<real>::max();

        (*_particles)[i]._x.resize(edges.size()+1);
        (*_particles)[i]._minX.resize(edges.size()+1);
        (*_particles)[i]._maxX.resize(edges.size()+1);
        (*_particles)[i]._v.resize(edges.size()+1);
        (*_particles)[i]._x_pb.resize(edges.size()+1);
        (*_particles)[i]._x_lb.resize(edges.size()+1);
        (*_particles)[i]._worstFlag = false;
        (*_particles)[i]._points = _psoParams.startPoints;

        if (i == 0) {
            _gb._x.resize(edges.size()+1);
        }
        // Create each inner column edge
        for (uint j = 0; j < edges.size(); j++) {
            // Create left edges
            (*_particles)[i]._x[j].resize(edges[j].size());
            (*_particles)[i]._minX[j].resize(edges[j].size());
            (*_particles)[i]._maxX[j].resize(edges[j].size());
            (*_particles)[i]._v[j].resize(edges[j].size());
            (*_particles)[i]._x_pb[j].resize(edges[j].size());
            (*_particles)[i]._x_lb[j].resize(edges[j].size());
            if (i == 0) {
                _gb._x[j].resize(edges[j].size());
            }
            for (uint k = 0; k < edges[j].size(); k++) {
                // Create right edges
                (*_particles)[i]._x[j][k].resize(edges[j][k].size());
                (*_particles)[i]._minX[j][k].resize(edges[j][k].size());
                (*_particles)[i]._maxX[j][k].resize(edges[j][k].size());
                (*_particles)[i]._v[j][k].resize(edges[j][k].size());
                (*_particles)[i]._x_pb[j][k].resize(edges[j][k].size());
                (*_particles)[i]._x_lb[j][k].resize(edges[j][k].size());
                if (i == 0) {
                    _gb._x[j][k].resize(edges[j][k].size());
                }
                for (uint m = 0; m < edges[j][k].size(); m++) {
                    (*_particles)[i]._v[j][k][m] = 0;
                    (*_particles)[i]._x_pb[j][k][m] = 0;
                    (*_particles)[i]._x_lb[j][k][m] = 0;

                    if (i == 0) {
                        _gb._x[j][k][m] = 0;
                    }
                    if (j == 0 || j == edges.size()-1) {
                        //(*_particles)[i]._x[j][k][m] = infDist(_randomEngine.engine());
//                        (*_particles)[i]._x[j][k][m] = _randomEngine.uniformReal(
//                                    infWeightRange[0], infWeightRange[1]);
                        (*_particles)[i]._x[j][k][m] = _randomEngine.uniformReal(
                                    innerWeightRange[0], innerWeightRange[1]);
                        (*_particles)[i]._minX[j][k][m] = infWeightRange[0];
                        (*_particles)[i]._maxX[j][k][m] = infWeightRange[1];
                    } else {
                        //(*_particles)[i]._x[j][k][m] = dist(_randomEngine.engine());
                        (*_particles)[i]._x[j][k][m] = _randomEngine.uniformReal(
                                    innerWeightRange[0], innerWeightRange[1]);
                        (*_particles)[i]._minX[j][k][m] = innerWeightRange[0];
                        (*_particles)[i]._maxX[j][k][m] = innerWeightRange[1];
                    }
                }
            }
        }

        const size_t recEdgeIt = edges.size();

        (*_particles)[i]._x[recEdgeIt].resize(rEdges.size());
        (*_particles)[i]._minX[recEdgeIt].resize(rEdges.size());
        (*_particles)[i]._maxX[recEdgeIt].resize(rEdges.size());
        (*_particles)[i]._v[recEdgeIt].resize(rEdges.size());
        (*_particles)[i]._x_pb[recEdgeIt].resize(rEdges.size());
        (*_particles)[i]._x_lb[recEdgeIt].resize(rEdges.size());
        if (i == 0) {
            _gb._x[recEdgeIt].resize(rEdges.size());
        }
        for (uint j = 0; j < rEdges.size(); j++) {
            (*_particles)[i]._x[recEdgeIt][j].resize(rEdges[j].size());
            (*_particles)[i]._minX[recEdgeIt][j].resize(rEdges[j].size());
            (*_particles)[i]._maxX[recEdgeIt][j].resize(rEdges[j].size());
            (*_particles)[i]._v[recEdgeIt][j].resize(rEdges[j].size());
            (*_particles)[i]._x_pb[recEdgeIt][j].resize(rEdges[j].size());
            (*_particles)[i]._x_lb[recEdgeIt][j].resize(rEdges[j].size());
            if (i == 0) {
                _gb._x[recEdgeIt][j].resize(rEdges[j].size());
            }
            for (uint k = 0; k < rEdges[j].size(); k++) {
                //(*_particles)[i]._x[recEdgeIt][j][k] = dist(_randomEngine.engine());
                (*_particles)[i]._x[recEdgeIt][j][k] = _randomEngine.uniformReal(
                            innerWeightRange[0], innerWeightRange[1]);
                (*_particles)[i]._minX[recEdgeIt][j][k] = innerWeightRange[0];
                (*_particles)[i]._maxX[recEdgeIt][j][k] = innerWeightRange[1];
                (*_particles)[i]._v[recEdgeIt][j][k] = 0;
                (*_particles)[i]._x_pb[recEdgeIt][j][k] = 0;
                (*_particles)[i]._x_lb[recEdgeIt][j][k] = 0;

                if (i == 0) {
                    _gb._x[recEdgeIt][j][k] = 0;
                }
            }
        }
    }
}

void NeuralPso::fly() {

    real velSum = 0;

    if (_particles->size() == 0) return;
    if ((*_particles)[0]._v.size() == 0) return;

    static int innerNetAccessCount = _psoParams.iterationsPerLevel;
    static uint innerNetIt = (*_particles)[0]._v.size()-1;

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
    //std::uniform_real_distribution<real> dist(0, 1);
    //std::uniform_real_distribution<real> negPosRange(-1, 1);

    // For each particle
    for (size_t i = 0; i < _particles->size(); i++) {
        Particle<NeuralNet::CombEdgeType> *p = &(*_particles)[i];
        //p->_worstFlag = false;
        worstFlag = p->_worstFlag;

        if (worstFlag) {
            p->_worstFlag = false;
            if (p->_points <= 0) {
                //choice = (real) (rand() % 10000) / 10000.0;
                //choice = dist(_randomEngine.engine());
                choice = _randomEngine.uniformReal(
                            probRange[0], probRange[1]);
                p->_fit_pb = 0;
                p->_fit_lb = 0;
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
        for (uint inner_net = 0; inner_net < p->_v.size(); inner_net++) {
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
                    /// Concept based on Genetic Algorithms, idea based on Alex
                    /// Find the worst of the particles and reset it.
                    if (worstFlag) { // Reset the worst one
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
                        continue;
                    }

                    //flyIteration(i, inner_net, left_edge, right_edge);
                    real *w_v = &p->_v[inner_net][left_edge][right_edge];
                    real *w_x = &p->_x[inner_net][left_edge][right_edge];
                    real *w_pb = &p->_x_pb[inner_net][left_edge][right_edge];
                    real *w_lb = &p->_x_lb[inner_net][left_edge][right_edge];
                    real *w_gb = &_gb._x[inner_net][left_edge][right_edge];
                    real *w_x_min = &p->_minX[inner_net][left_edge][right_edge];
                    real *w_x_max = &p->_maxX[inner_net][left_edge][right_edge];

                    //real inertia = (dist(_randomEngine.engine())*0.5) + 0.5;
                    //real c1 = C1 * dist(_randomEngine.engine());
                    //real c2 = C2 * dist(_randomEngine.engine());
                    //real c3 = C3 * dist(_randomEngine.engine());
                    real inertia = _randomEngine.uniformReal(inertiaRange[0], inertiaRange[1]);
                    real c1 = C1 * _randomEngine.uniformReal(probRange[0], probRange[1]);
                    real c2 = C2 * _randomEngine.uniformReal(probRange[0], probRange[1]);
                    real c3 = C3 * _randomEngine.uniformReal(probRange[0], probRange[1]);

                    *w_v = (
                                inertia * (*w_v)
                                + (c1*(*w_pb - *w_x))
                                + (c2*(*w_lb - *w_x))
                                + (c3*(*w_gb - *w_x))
                                ) * dt;

                    velSum += *w_v;

                    if (*w_v > vLimit) {
                        *w_v = vLimit;
                    } else if (*w_v < -vLimit) {
                        *w_v = -vLimit;
                    }

                    *w_x += *w_v;

                    *w_x = std::min(*w_x, *w_x_max);
                    *w_x = std::max(*w_x, *w_x_min);

//                    if (*w_x > fitnessParams()->edgeWeightMax) {
//                        *w_x = fitnessParams()->edgeWeightMax;
//                        //*w_v *= -0.01;
//                    } else if (*w_x < fitnessParams()->edgeWeightMin) {
//                        *w_x = fitnessParams()->edgeWeightMin;
//                        //*w_v *= -0.01;
//                    }
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

    // If stagnant, mix it up a bit
    if (abs(velSum) < _psoParams.vDelta) {

        for (size_t i = 0; i < _particles->size(); i++) {
            // Reset bests
            (*_particles)[i]._fit_pb = -numeric_limits<real>::max();
            (*_particles)[i]._fit_lb = -numeric_limits<real>::max();
            for (size_t j = 0; j < (*_particles)[i]._x.size(); j++) {
                for (size_t k = 0; k < (*_particles)[i]._x[j].size(); k++) {
                    for (size_t m = 0; m < (*_particles)[i]._x[j][k].size(); m++) {
                        //(*_particles)[i]._x[j][k][m] = ((real)(rand() % 20000)/10000.0) - 1.0;
                        //(*_particles)[i]._x[j][k][m] = negPosRange(_randomEngine.engine());
                        (*_particles)[i]._x[j][k][m] = _randomEngine.uniformReal(negRange[0], negRange[1]);
                    }
                }
            }
        }
    }
    //cout << velSum << endl;

    /// For termination through user control
    if (stopProcessing) {
        interruptProcess();
    }
}

/*
void NeuralPso::flyIteration(
        size_t particle,
        size_t inner_net,
        size_t left_edge,
        size_t right_edge) {
//    const real C1 = 2.495, C2 = 2.495, C3 = 0.5;
//    real dt = 1;
//    Particle<NeuralNet::CombEdgeType> *p = &(*_particles)[particle];
//    real *w_v = &p->_v[inner_net][left_edge][right_edge];
//    real *w_x = &p->_x[inner_net][left_edge][right_edge];
//    real *w_pb = &p->_x_pb[inner_net][left_edge][right_edge];
//    real *w_lb = &p->_x_lb[inner_net][left_edge][right_edge];
//    real *w_gb = &_gb._x[inner_net][left_edge][right_edge];

//    real inertia = ((real) ((rand() % 50000) + 50000)) / 100000.0;
//    real c1 = C1 * ((real) (rand() % 10000)) / 10000.0;
//    real c2 = C2 * ((real) (rand() % 10000)) / 10000.0;
//    real c3 = C3 * ((real) (rand() % 10000)) / 10000.0;

//    *w_v = (
//                inertia * (*w_v)
//            + (c1*(*w_pb - *w_x))
//            + (c2*(*w_lb - *w_x))
//            + (c3*(*w_gb - *w_x))
//            ) * dt;

//    velSum += *w_v;
//    term = term && (*w_pb == *w_x) && (*w_lb == *w_x);

//    if (*w_v > vLimit) {
//      *w_v = vLimit;
//    } else if (*w_v < -vLimit) {
//      *w_v = -vLimit;
//    }

//    *w_x += *w_v;

//    if (*w_x > 1.0) {
//      *w_x = -1.0;
//      // *w_v *= -0.01;
//    } else if (*w_x < -1.0) {
//      *w_x = 1.0;
//      // *w_v *= -0.01;
//    }
}
*/

/**
 * @brief NeuralPso::getCost
 * @details Update each particle's current cost at current position
 */
void NeuralPso::getCost() {
    // Get the cost for each particle's current position
    for (size_t i = 0; i < _particles->size(); i++) {
        if (checkTermProcess()) {
            return;
        }

        NeuralParticle *p = &(*_particles)[i];

        if (!_neuralNet->setCombinedWeights(p->_x)) {
            std::cout<< "Failure to set weights." << endl;
        }

        // Get fitness
        real fit = trainingRun();
        p->_fit = fit;
    }
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
        if (printChange[i]) {
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

/*
  // Print out global change made
  if (prevBest != gb()->_fit_pb) {
      prevBest = gb()->_fit_pb;

      std::string outputString;
      outputString += "Global: ";
      outputString += stringPut(gb()->_fit_pb);
      outputString += " - Correct: ";
      outputString += stringPut(correctRatio * 100.0);
      outputString += "% of ";
      outputString += stringPut(totalCount);
      outputString += " tests\n";
      Logger::write(outputString);

      printGB();
      testGB();
  }
*/

    if (checkForPrint()) {
        testGB();
    }

    printGBMtx.lock();
    if (printGBFlag) {
        printGBFlag = false;
        printGB();
    }
    printGBMtx.unlock();

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
        // Check if personal best has surpassed remembered fitness
        if ((*_particles)[i]._fit > (*_particles)[i]._fit_pb) {
            (*_particles)[i]._fit_pb = (*_particles)[i]._fit;
            (*_particles)[i]._points += _psoParams.pbPoints;
            //printChange[i] = true;
            updatePersonalBest((*_particles)[i]);
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
            if (p_n._fit_pb >= p._fit_lb) {
                p._fit_lb = p_n._fit_pb;
                bestNeighborIt = static_cast<size_t>(it);
            }
        }

        NeuralParticle &p_n = (*_particles)[bestNeighborIt];
        //printChange[i] = true;
        updateNeighborBest(p, p_n);
    }
}

/**
 * @brief NeuralPso::updateNeighborBest
 * @details Update neighbor best for current particle
 * @param p     - Current Particle
 * @param p_n   - Best neighbor Particle
 */
void NeuralPso::updateNeighborBest(NeuralParticle & p, NeuralParticle & p_n) {
    p_n._fit_lb = p._fit_pb;
    p_n._points += _psoParams.lbPoints;
    for (size_t i = 0; i < p_n._x_lb.size(); i++) {
        for (size_t j = 0; j < p_n._x_lb[i].size(); j++) {
            for (size_t k = 0; k < p_n._x_lb[i][j].size(); k++) {
                p_n._x_lb[i][j][k] = p._x_pb[i][j][k];
            } // for k
        } // for j
    } // for i
} // updateNeighborBest

void NeuralPso::findGlobalBest(std::vector<bool> & printChange) {
    size_t globalBestIt=0;
    for (size_t i = 0; i < _particles->size(); i++) {
        NeuralParticle & p = (*_particles)[i];
        if (p._fit_pb >= _gb._fit_pb) {
            _gb._fit_pb = p._fit_pb;
            globalBestIt = i;
        } // End global best
    }

    NeuralParticle & p = (*_particles)[globalBestIt];
    printChange[globalBestIt] = true;
    updateGlobalBest(p);
}

void NeuralPso::updateGlobalBest(NeuralParticle &p) {
    _gb._fit_pb = p._fit_pb;
    p._points += _psoParams.gbPoints;
    for (size_t i = 0; i < p._x.size(); i++) {
        for (size_t j = 0; j < p._x[i].size(); j++) {
            for (size_t k = 0; k < p._x[i][j].size(); k++) {
                _gb._x[i][j][k] = p._x[i][j][k];
            }
        }
    }
}

real NeuralPso::trainingRun() {
    std::cout << "Error, must be implemented via inherited training class." << std::endl;
    return 0;
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
  for (uint i = 0; i < _gb._x.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < gb()->_x[i].size(); j++) {
      for (uint k = 0; k < gb()->_x[i][j].size(); k++) {
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

void NeuralPso::printParticle(uint I) {
  if (I > _particles->size()) return;

  std::string printString;
  printString += "Particle (";
  printString += stringPut(I);
  printString += "): \n";
  for (uint i = 0; i < (*_particles)[I]._x.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < (*_particles)[I]._x[i].size(); j++) {
      for (uint k = 0; k < (*_particles)[I]._x[i][j].size(); k++) {
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

void NeuralPso::printParticlePBest(uint I) {
  if (I > _particles->size()) return;

  std::string printString;
  printString += "Particle pBest (";
  printString += stringPut(I);
  printString += "): \n";
  for (uint i = 0; i < (*_particles)[I]._x_pb.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < (*_particles)[I]._x_pb[i].size(); j++) {
      for (uint k = 0; k < (*_particles)[I]._x_pb[i][j].size(); k++) {
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

void NeuralPso::printParticleLBest(uint I) {
  if (I > _particles->size()) return;

  std::string printString;
  printString += "Particle lBest (";
  printString += stringPut(I);
  printString += "): \n";
  for (uint i = 0; i < (*_particles)[I]._x_lb.size(); i++) {
    printString += "  Inner Net ";
    printString += stringPut(i+1);
    printString += "\n";
    for (uint j = 0; j < (*_particles)[I]._x_lb[i].size(); j++) {
      for (uint k = 0; k < (*_particles)[I]._x_lb[i][j].size(); k++) {
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

bool NeuralPso::injectGb(const NeuralNet::CombEdgeType &w) {
    if (_particles->size() < 1) {
        return false;
    }

    NeuralNet::EdgeType & px = (*_particles)[0]._x;

    if (w.size() != px.size()) {
        return false;
    }

    for (size_t inner_node = 0; inner_node < px.size(); inner_node++) {
        if (w.at(inner_node).size() != px[inner_node].size()) {
            return false;
        }
        for (size_t left_node = 0; left_node < px[inner_node].size(); left_node++) {
            if (w.at(inner_node).at(left_node).size() != px[inner_node][left_node].size()) {
                return false;
            }
            for (size_t right_node = 0; right_node < px[inner_node][left_node].size(); right_node++) {
                    px[inner_node][left_node][right_node] = w.at(inner_node).at(left_node).at(right_node);
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
    NeuralNet::CombEdgeType & gbEdges = getGbEdges();
    std::unique_ptr<NeuralNet> n = std::make_unique<NeuralNet>(*neuralNet()->nParams(), gbEdges);
    return n;
}

NeuralNet::CombEdgeType &NeuralPso::getGbEdges() {
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

    Particle<NeuralNet::CombEdgeType> gb =
            particleFromString(gbString);
    _gb = gb;

    std::string partString = subStringByToken(cleanString, "_particles", it);
    *_particles = readParticlesFromString(partString);

    return true;
}
