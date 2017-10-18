#ifndef INPUTPARTITIONER_H
#define INPUTPARTITIONER_H

#include "randomnumberengine.h"

class inputPartitioner
{
public:
    void inputPartitioner(int numInputSamples, int trainingItters, RandomNumberEngine rand);
    void ~inputPartitioner();
    std::vector<size_t> testInputs;
    std::vector<size_t> trainingInputs;
private:
    int * _indicies;
    int _testBound; // inclusive
    int _testIdx; // next input to test
    int _trainIdx; // next input for training
    int _trainingItters;
    int _numInputSamples;
    void reshuffle(); // Only reshuffels segment for training / validation
};

#endif // INPUTPARTITIONER_H
