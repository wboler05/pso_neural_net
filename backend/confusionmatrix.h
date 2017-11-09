#ifndef CONFUSIONMATRIX_H
#define CONFUSIONMATRIX_H

#include "backend/teststatistics.h"

class ConfusionMatrix
{
public:

    // First Dim: Actual, Second Dim: Predicted
    typedef std::vector<std::vector<size_t>> ClassifierMatrix;
    typedef std::vector<std::vector<real>> ClassifierMatrixRatios;
    typedef std::vector<size_t> ClassifierVector;
    typedef std::vector<real> ClassifierVectorRatios;

    ConfusionMatrix();
    ConfusionMatrix(const ClassifierMatrix & results);

    bool setResults(const ClassifierMatrix & results);
    bool validate(const ClassifierMatrix & results);

    TestStatistics getTestStatistics() { return _ts; }
    size_t numberOfClassifiers() const { return _numberOfClassifiers; }
    size_t getPopulation() { return _totalPopulation; }

    void reset();

    const ClassifierMatrix & getResultValues() { return _resultValues; }
    const ClassifierVector & getTruePositiveValues() { return _truePositiveValues; }
    const ClassifierVector & getFalsePositiveValues() { return _falsePositiveValues; }
    const ClassifierVector & getFalseNegativeValues() { return _falseNegativeValues; }

    const ClassifierMatrixRatios & getResultRatios() { return _resultRatios; }
    const ClassifierVectorRatios & getTruePositiveRatios() { return _truePositiveRatios; }
    const ClassifierVectorRatios & getFalsePositiveRatios() { return _falsePositiveRatios; }
    const ClassifierVectorRatios & getFalseNegativeRatios() { return _falseNegativeRatios; }

    static ClassifierMatrix evaluateResults(const std::vector<std::vector<real>> predicted, const std::vector<std::vector<real>> actual);

    std::string toString();
    void print();

private:
    // Result Matrix
    ClassifierMatrix _resultValues;
    ClassifierMatrixRatios _resultRatios;

    // Integer Values
    ClassifierVector _truePositiveValues;
    ClassifierVector _falsePositiveValues;
    ClassifierVector _falseNegativeValues;

    // Double Values
    ClassifierVectorRatios _truePositiveRatios;
    ClassifierVectorRatios _falsePositiveRatios;
    ClassifierVectorRatios _falseNegativeRatios;

    TestStatistics _ts;
    size_t _numberOfClassifiers = 0;
    size_t _totalPopulation = 0;

    void setTotalClassifiers(const size_t & totalClassifiers);
    void copyCounts(const ClassifierMatrix & results);
    void setPopulationFromResults();

    void constructTestResults();
    void countTruePositives();
    void countFalsePositives();
    void countFalseNegatives();

    void constructResultRatios();
    void constructTruePositiveRatios();
    void constructFalsePositiveRatios();
    void constructFalseNegativeRatios();

    void constructRatioVector(ClassifierVector & v, ClassifierVectorRatios & r);
};

#endif // CONFUSIONMATRIX_H
