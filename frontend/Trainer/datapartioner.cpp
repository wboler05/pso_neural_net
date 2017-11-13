#include "datapartioner.h"

dataPartioner::dataPartioner(size_t kFolds, size_t totalNumInputs, size_t numClasses, const std::shared_ptr<InputCache> & inputCache){

    //Init random engine
    time_t currentTime;
    time(&currentTime);
    _randomEngine = RandomNumberEngine(currentTime);

    _kFolds = kFolds;
    _totalNumInputs = totalNumInputs;
    _numClasses = numClasses;
    _inputCache = inputCache;

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

    // Fill the vectors
    for (size_t i = 0; i < _numElePerValidationRound; i++){
        _validationSet.push_back(indicies[i]);
    }
    for (size_t i = _numElePerValidationRound; i < _testBound; i++){
        _trainingSet.push_back(indicies[i]);
    }
    for (size_t i = _testBound; i < _totalNumInputs; i++){
        _testSet.push_back(indicies[i]);
    }

    // Split the training set for training vector creation
    splitTrainingClasses();

    // Calculate the implicit bias weights of each class
    calcImplicitBiasWeights();
}

void dataPartioner::reset(){

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

    // Fill the vectors
    for (size_t i = 0; i < _numElePerValidationRound; i++){
        _validationSet.push_back(indicies[i]);
    }
    for (size_t i = _numElePerValidationRound; i < _testBound; i++){
        _trainingSet.push_back(indicies[i]);
    }
    for (size_t i = _testBound; i < _totalNumInputs; i++){
        _testSet.push_back(indicies[i]);
    }

    // Split the training set for training vector creation
    splitTrainingClasses();
}

size_t dataPartioner::nextFold(){
    size_t temp;
    _foldIdx++;
    if(_foldIdx < _kFolds){
        for(size_t i = 0; i < _numElePerValidationRound; i++){
            temp = _validationSet[i];
            _validationSet[i] = _trainingSet[(_foldIdx-1)*_numElePerValidationRound + i];
            _trainingSet[(_foldIdx-1)*_numElePerValidationRound + i] = temp;
        }
        // Split the training set for training vector creation
        splitTrainingClasses();
        return 1;
    }
    return -1;
}

size_t dataPartioner::trainingSet(size_t i){
    return _trainingSet[i];
}

size_t dataPartioner::testSet(size_t i){
    return _testSet[i];
}

size_t dataPartioner::validationSet(size_t i){
    return _validationSet[i];
}

size_t dataPartioner::trainingSetSize(){
    return _trainingSet.size();
}

size_t dataPartioner::testSetSize(){
    return _testSet.size();
}

size_t dataPartioner::validationSetSize(){
    return _validationSet.size();
}

const std::vector<size_t> & dataPartioner::getTestSet() const{
    return _testSet;
}

const std::vector<size_t> & dataPartioner::getValidationSet() const{
    return _validationSet;
}

const real & dataPartioner::getImplicitBiasWeight(int classNum) const{
    return _implicitBiasWeights[classNum];
}

const real & dataPartioner::getFittnessNormFactor() const {
    return _fitnessNormalizationFactor;
}

const std::vector<size_t> & dataPartioner::getTrainingVector(std::vector<size_t> & tr, const size_t & iterations){
    tr.clear();
    tr.resize(iterations);
    for (size_t i = 0; i < iterations; i++) {
        size_t randClass = 0;
        size_t breakMe = 0;
        do {
            randClass = _randomEngine.uniformUnsignedInt(0, _numClasses-1);
            breakMe++;
        } while (_trainingSetClassBins[randClass].size() == 0 && breakMe < _totalNumInputs);
        size_t randTrainingIt = _randomEngine.uniformUnsignedInt(0, _trainingSetClassBins[randClass].size()-1);
        size_t realIterator = _trainingSetClassBins[randClass][randTrainingIt];
        tr[i] = realIterator;
    }
}

void dataPartioner::splitTrainingClasses() {

    // Clear equi-probable vector and resize to # of classes
    _trainingSetClassBins.clear();
    _trainingBinIndicies.clear();
    _trainingSetClassBins.resize(_numClasses);
    _trainingBinIndicies.resize(_numClasses);

    // Go through every data inside the file
    for (size_t i = 0; i < _trainingSet.size(); i++) {
        size_t realIterator = _trainingSet[i];
        // Get the output for each data line
        std::vector<real> v = (*_inputCache)[realIterator].outputize();
        for (size_t j = 0; j < v.size(); j++) {
            // If the output is classified '1', then
            if (v[j] == 1.0) {
                // Push that iterator to the correct bin
                _trainingSetClassBins[j].push_back(realIterator);
            }
        }
    }

    // initalize the index arrays
    for(size_t i = 0; i < _numClasses; i++){
        _trainingBinIndicies[i].resize(_trainingSetClassBins[i].size());
        for(size_t j = 0; j < _trainingSetClassBins[i].size(); j++){
            _trainingBinIndicies[i][j] = j;
        }
        shuffleVector(_trainingBinIndicies[i]);
    }
}

void dataPartioner::shuffleVector(std::vector<size_t> * toShuffle){
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

void dataPartioner::calculateClassFrequency() {

    _trueNumElesPerClass.resize(_numClasses,0);

    for (size_t i = 0; i < _totalNumInputs; i++) {
        OutageDataWrapper dataItem = (*_inputCache)[i];
        if (dataItem.empty()) {
            continue;
        }
        std::vector<real> outputClassVector = dataItem.outputize();
        if (outputClassVector.size() != _numClasses) {
            continue;
        }
        for (size_t j = 0; j < _numClasses; j++){
            if (outputClassVector[j] == 1.0){
                _trueNumElesPerClass[j]++;
                break;
            }
        }
    }
}

void dataPartioner::initializeBiasVectors() {
    _implicitBiasWeights.resize(_numClasses,0);
    _trueNumElesPerClass.resize(_numClasses,0);
    _equalizationFactors.resize(_numClasses,0);
    _fitnessNormalizationFactor = 0;
}

void dataPartioner::calcImplicitBiasWeights() {

    // Initialize vectors
    initializeBiasVectors();

    // Run through the data to get count of each class
    calculateClassFrequency();

    // Calculate Ratios
    for (size_t i = 0; i < _implicitBiasWeights.size(); i++){
        _implicitBiasWeights[i] = static_cast<real>(_trueNumElesPerClass[i]) / static_cast<real>(_totalNumInputs);
    }
    // Calculate Normalization Factor
    real a = 0;
    for (size_t i = 0; i < _numClasses; i++){
        a = ((static_cast<real>(_trueNumElesPerClass[i] - 1)/static_cast<real>(_totalNumInputs))/_implicitBiasWeights[i]);
        _equalizationFactors[i] = 1.0 / (1.0-(1.0/_numClasses)*(_numClasses - 1.0 + a));
        _fitnessNormalizationFactor += (1.0/_numClasses) * _equalizationFactors[i];
    }
}
