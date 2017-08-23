#ifndef NEURALPSO_H
#define NEURALPSO_H

#include <QCoreApplication>

#include "PSO/pso.h"
#include "NeuralNet/NeuralNet.h"
#include "backend/teststatistics.h"

#include <ctime>
#include <limits>
#include <memory>

#include "CL/cl.hpp"
#include "util.h"

/*
template <class T>
class Pso;
*/
template class Pso<NeuralNet::EdgeType>;

/*
template <class T>
struct Particle;
*/
template struct Particle<NeuralNet::EdgeType>;

class NeuralNet;

struct FitnessParameters {
    double mse_weight;
    double mse_floor;
    TestStatistics::ClassificationError weights;
    TestStatistics::ClassificationError floors;
};

class NeuralPso : public Pso<NeuralNet::EdgeType> {
public:
  NeuralPso(PsoParams pp, NeuralNetParameters np, FitnessParameters fp);
  ~NeuralPso();

  void buildPso();
  void build(std::vector<std::vector<std::vector<byte> > > &images, std::vector<byte> &labels);
  void build(std::vector<std::vector<double>> &input, std::vector<double> &output);
  void fly();
  double getCost();
  void processEvents();

  double testRun(double &correctRatio, uint &totalCount, double &confidence);
  void testGB();

  NeuralNet * neuralNet() { return _neuralNet; }
  std::unique_ptr<NeuralNet> buildNeuralNetFromGb();

  int randomizeTestInputs();
  void runTrainer();
  void loadTestInput(uint32_t I);

  void printGB();
  void printParticle(uint i);
  void printParticlePBest(uint i);
  void printParticleLBest(uint i);

  static void setToPrintGBNet();

  void setFunctionMsg(std::string s) { _functionMsg = s; }
  std::string functionMsg() { return _functionMsg; }

  void classError(TestStatistics::ClassificationError * ce);

  FitnessParameters * fitnessParams() { return &_fParams; }

  NeuralNet::EdgeType & getGbEdges();

  //!TODO  convert to const, please
  TestStatistics & testStats() { return _testStats; }

  int convertOutput(const double & output);

protected:
    NeuralNet *_neuralNet;

private:
  std::vector<std::vector<std::vector<byte> > > *_images;
  std::vector<byte> *_labels;
  FitnessParameters _fParams;
  TestStatistics _testStats;

  // Test input
  std::vector<std::vector<double> > *_input;
  std::vector<double> *_output;
  std::vector<double> *_outputCount;
  std::vector<std::vector<uint>> _outputIterators;

  std::string _functionMsg;

  static bool printGBFlag;
  static boost::mutex printGBMtx;

  void flyIteration(size_t particle, size_t inner_net, size_t left_edge, size_t right_edge);
  virtual bool validateOutput(
          std::vector<double> & outputs,
          std::vector<double> & expectedOutputs,
          std::vector<double> & outputError,
          TestStatistics & testStats,
          bool & correctOutput);

//  vector<pair<bool, int>> _failureQueue;

};

#endif // NEURALPSO_H
