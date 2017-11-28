#ifndef DATAPARTIONER_H
#define DATAPARTIONER_H

#include <vector>
#include "util.h"
#include "randomnumberengine.h"
#include <ctime>
#include "inputcache.h"
#include "trainingparameters.h"
#include "outagedataitem.h"

class DataPartioner{
public:
    DataPartioner() {}
    DataPartioner(size_t kFolds, const std::shared_ptr<TrainingParameters> & params, size_t numClasses, const std::shared_ptr<InputCache> & inputCache);
    DataPartioner & operator=(DataPartioner && r);
    size_t trainingSetSize();
    size_t testSetSize();
    size_t validationSetSize();
    const std::vector<size_t> & getTestSet() const;
    const std::vector<size_t> & getValidationSet() const;
    void getTrainingVector(std::vector<size_t> & tr, const size_t & iterations);
    const real & getImplicitBiasWeight(int classNum) const;
    const real & getFittnessNormFactor() const;
    void reset();
    size_t nextFold();
    const size_t & foldIndex() { return _foldIdx; }
    const size_t & numFolds() { return _kFolds; }
    void updateMinMax();
    std::vector<real> normalizeInput(const size_t & id);
    std::vector<real> normalizeInput(std::vector<real> & input);
    OutageDataWrapper getDataWraper(size_t idxInCache);
private:
    RandomNumberEngine _randomEngine;
    void shuffleVector(std::vector<size_t> & toShuffle);
    void splitTrainingClasses();
    void calculateClassFrequency();
    void initializeBiasVectors();
    void calcImplicitBiasWeights();

    void initHistoryLookup();
    int findIdx(const std::vector<size_t> & toSearch, size_t value);

    void radixSort(std::vector<OutageDataWrapper> & toSort); // Needs to be modified if hourly data is used
    std::vector<OutageDataWrapper> radixMerge(std::vector<OutageDataWrapper> left, std::vector<OutageDataWrapper> right, int radix);
    std::vector<OutageDataWrapper> radixMergeSort(std::vector<OutageDataWrapper> & toSort, int radix);

    std::vector<std::vector<size_t>> _trainingSetClassBins;
    std::vector<std::vector<size_t>> _trainingBinIndicies;
    std::vector<size_t> _trainingBinCounters;
    std::vector<size_t> _trainingSet;
    std::vector<size_t> _testSet;
    std::vector<size_t> _validationSet;
    std::vector<std::vector<size_t>> _historyLookup; // First Dim is index in file, second Dim is historic neighbors
    size_t _historySize = 10; // How may historic samples to associate with each current sample (includes the current sample in the count)
    const size_t _numTimeScales = 3; // Number of time radix's to sort by. 3 => Year, Month, Day | 4 => Year, Month, Day, Hour | ect...

    std::vector<real> _implicitBiasWeights;
    std::vector<size_t> _trueNumElesPerClass;
    std::vector<real> _equalizationFactors;
    real _fitnessNormalizationFactor;

    std::shared_ptr<InputCache> _inputCache;
    std::shared_ptr<TrainingParameters> _params;
    std::vector<real> _minInputData;
    std::vector<real> _maxInputData;

    size_t _totalNumInputs = 0;
    const real _boundRatio = 0.90; // 90% for Training, 10% for Testing
    size_t _testBound; // Bounding index for test set in the file. Inclusive to the test set.
    size_t _kFolds = 0; // Number of validation rounds
    size_t _foldIdx = 0; // The current validation fold
    size_t _numElePerValidationRound = 0; // Number of elements in each validation round
    size_t _numClasses;
};

#endif // DATAPARTIONER_H
