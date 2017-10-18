#include "inputpartitioner.h"

void inputPartitioner::inputPartitioner(int numInputSamples, int trainingItters, RandomNumberEngine rand)
{
    _indicies = (int *)malloc(sizeof(int) * numInputSamples);
    int swpIdx;
    int temp;
    int numTrainSamples;
    int numValidationSamples;

    // Initialize the vector
    for (int i = 0; i < numInputSamples; i++){
        _indicies[i] = i;
    }
    // Shuffle it
    for (int i = 1; i < numInputSamples; i++){
        swpIdx = rand.uniformUnsignedInt(0,i);
        if (swpIdx == i){
            continue;
        }
        else{
            temp = _indicies[swpIdx];
            _indicies[swpIdx] = _indicies[i];
            _indicies[i] = temp;
        }
    }
    // Partition
    _testBound = numInputSamples * .90;
    _testIdx = _testBound;
    _trainingItters = trainingItters;
    _trainIdx = 0;
    _numInputSamples = numInputSamples;
    // Fill the vectors
    for (int i = 0; i < _testBound; i++){
        trainingInputs.push_back(_indicies[i]);
    }
    for (int i = _testBound; i < _numInputSamples; i++){
        testInputs.push_back(_indicies[i]);
    }
}

void inputPartitioner::~inputPartitioner(){
    free((void *)_indicies);
}
