#ifndef DATAPARTIONER_H
#define DATAPARTIONER_H

#include <vector>
#include "util.h"
#include "randomnumberengine.h"
#include <ctime>

class dataPartioner{
public:
    dataPartioner();
    dataPartioner(size_t kFolds, size_t totalNumInputs);
    dataPartioner & operator=(dataPartioner && r);
    size_t trainingSet(size_t i);
    size_t trainingSetSize();
    size_t testSet(size_t i);
    size_t testSetSize();
    size_t validationSet(size_t i);
    size_t validationSetSize();
    const std::vector<size_t> & getTestSet() const;
    const std::vector<size_t> & getValidationSet() const;
    void reset();
    size_t nextFold();
private:
    RandomNumberEngine _randomEngine;
    void shuffleVector(std::vector<size_t> * toShuffle);
    std::vector<size_t> _trainingSet;
    std::vector<size_t> _testSet;
    std::vector<size_t> _validationSet;
    std::vector<size_t> _trainingOrder;
    size_t _totalNumInputs = 0;
    const real _boundRatio = 0.90; // 90% for Training, 10% for Testing
    size_t _testBound; // Bounding index for test set in the file. Inclusive to the test set.
    size_t _kFolds = 0; // Number of validation rounds
    size_t _foldIdx = 0; // The current validation fold
    size_t _numElePerValidationRound = 0; // Number of elements in each validation round
};

#endif // DATAPARTIONER_H
