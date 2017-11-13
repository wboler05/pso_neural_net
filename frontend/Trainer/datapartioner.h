#ifndef DATAPARTIONER_H
#define DATAPARTIONER_H

#include <vector>
#include "util.h"
#include "randomnumberengine.h"
#include <ctime>
#include "inputcache.h"

class dataPartioner{
public:
    dataPartioner();
    dataPartioner(size_t kFolds, size_t totalNumInputs, size_t numClasses, const std::shared_ptr<InputCache> & inputCache);
    dataPartioner & operator=(dataPartioner && r);
    size_t trainingSet(size_t i);
    size_t trainingSetSize();
    size_t testSet(size_t i);
    size_t testSetSize();
    size_t validationSet(size_t i);
    size_t validationSetSize();
    const std::vector<size_t> & getTestSet() const;
    const std::vector<size_t> & getValidationSet() const;
    const std::vector<size_t> & getTrainingVector(std::vector<size_t> & tr, const size_t & iterations);
    const real & getImplicitBiasWeight(int classNum) const;
    const real & getFittnessNormFactor() const;
    void reset();
    size_t nextFold();
private:
    RandomNumberEngine _randomEngine;
    void shuffleVector(std::vector<size_t> * toShuffle);
    void splitTrainingClasses();
    void calculateClassFrequency();
    void initializeBiasVectors();
    void calcImplicitBiasWeights();

    std::vector<std::vector<size_t>> _trainingSetClassBins;
    std::vector<size_t> _trainingSet;
    std::vector<size_t> _testSet;
    std::vector<size_t> _validationSet;

    std::vector<real> _implicitBiasWeights;
    std::vector<size_t> _trueNumElesPerClass;
    std::vector<real> _equalizationFactors;
    real _fitnessNormalizationFactor;

    std::shared_ptr<InputCache> _inputCache;
    size_t _totalNumInputs = 0;
    const real _boundRatio = 0.90; // 90% for Training, 10% for Testing
    size_t _testBound; // Bounding index for test set in the file. Inclusive to the test set.
    size_t _kFolds = 0; // Number of validation rounds
    size_t _foldIdx = 0; // The current validation fold
    size_t _numElePerValidationRound = 0; // Number of elements in each validation round
    size_t _numClasses;
};

#endif // DATAPARTIONER_H
