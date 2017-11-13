#include "datapartioner.h"

DataPartioner::DataPartioner(size_t kFolds, size_t totalNumInputs){

    //Init random engine
    time_t currentTime;
    time(&currentTime);
    _randomEngine = RandomNumberEngine(currentTime);

    _kFolds = kFolds;
    _totalNumInputs = totalNumInputs;

    std::vector<size_t> indicies;

    // Initialize the vector
    for (size_t i = 0; i < _totalNumInputs; i++){
        indicies.push_back(i);
    }

    // Shuffle it
    this->shuffleVector(&indicies);

    // Calculate the bounding index (inclusive)
    _testBound = _totalNumInputs * _boundRatio;
    _numElePerValidationRound = std::ceil(_testBound/(_kFolds+1));

    // Clear input lists
    _trainingSet.clear();
    _testSet.clear();
    _validationSet.clear();
    _trainingOrder.clear();

    // Fill the vectors
    for (size_t i = 0; i < _numElePerValidationRound; i++){
        _validationSet.push_back(indicies[i]);
    }
    for (size_t i = _numElePerValidationRound; i < _testBound; i++){
        _trainingSet.push_back(indicies[i]);
        _trainingOrder.push_back(i-_numElePerValidationRound);
    }
    for (size_t i = _testBound; i < _totalNumInputs; i++){
        _testSet.push_back(indicies[i]);
    }
}

void DataPartioner::reset(){

    std::vector<size_t> indicies;
    // Initialize the vector
    for (size_t i = 0; i < _totalNumInputs; i++){
        indicies.push_back(i);
    }

    // Shuffle it
    this->shuffleVector(&indicies);

    // Clear input lists
    _trainingSet.clear();
    _testSet.clear();
    _validationSet.clear();
    _trainingOrder.clear();

    // Fill the vectors
    for (size_t i = 0; i < _numElePerValidationRound; i++){
        _validationSet.push_back(indicies[i]);
    }
    for (size_t i = _numElePerValidationRound; i < _testBound; i++){
        _trainingSet.push_back(indicies[i]);
        _trainingOrder.push_back(i-_numElePerValidationRound);
    }
    for (size_t i = _testBound; i < _totalNumInputs; i++){
        _testSet.push_back(indicies[i]);
    }
}

size_t DataPartioner::nextFold(){
    size_t temp;
    _foldIdx++;
    if(_foldIdx < _kFolds){
        for(size_t i = 0; i < _numElePerValidationRound; i++){
            temp = _validationSet[i];
            _validationSet[i] = _trainingSet[(_foldIdx-1)*_numElePerValidationRound + i];
            _trainingSet[(_foldIdx-1)*_numElePerValidationRound + i] = temp;
        }
        this->shuffleVector(&_trainingOrder);
        return 1;
    }
    return -1;
}

size_t DataPartioner::trainingSet(size_t i){
    return _trainingSet[_trainingOrder[i]];
}

size_t DataPartioner::testSet(size_t i){
    return _testSet[i];
}

size_t DataPartioner::validationSet(size_t i){
    return _validationSet[i];
}

size_t DataPartioner::trainingSetSize(){
    return _trainingSet.size();
}

size_t DataPartioner::testSetSize(){
    return _testSet.size();
}

size_t DataPartioner::validationSetSize(){
    return _validationSet.size();
}

const std::vector<size_t> & DataPartioner::getTestSet() const{
    return _testSet;
}

const std::vector<size_t> & DataPartioner::getValidationSet() const{
    return _validationSet;
}

void DataPartioner::shuffleVector(std::vector<size_t> * toShuffle){
    size_t swpIdx, temp;
    for (size_t i = 1; i < toShuffle->size(); i++){
        swpIdx = _randomEngine.uniformUnsignedInt(0,i);
        if (swpIdx == i){
            continue;
        }
        else{
            temp = (*toShuffle)[swpIdx];
            (*toShuffle)[swpIdx] = (*toShuffle)[i];
            (*toShuffle)[i] = temp;
        }
    }
}

