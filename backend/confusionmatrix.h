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

    TestStatistics & overallStats() { return _overallStats; }
    std::vector<TestStatistics> & classStats() { return _classStats; }
    TestStatistics::ClassificationError & overallError() { return _overallError; }
    std::vector<TestStatistics::ClassificationError> & classErrors() { return _classErrors; }

    void costlyComputeClassStats();

    static ClassifierMatrix evaluateResults(const std::vector<std::vector<real>> predicted, const std::vector<std::vector<real>> actual);

    std::string toString();
    void print();

    static real MSE(const std::vector<std::vector<real>> &results,
                    const std::vector<std::vector<real>> & expecteds);
    static real MSE(const std::vector<real> &result,
                    const std::vector<real> &expected);
    static std::vector<real> splitMSE(const std::vector<real> &result,
                    const std::vector<real> &expected);

private:
    // Data Analytics
    TestStatistics _overallStats;
    TestStatistics::ClassificationError _overallError;
    std::vector<TestStatistics> _classStats;
    std::vector<TestStatistics::ClassificationError> _classErrors;

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

    // Utility
    size_t _numberOfClassifiers = 0;
    size_t _totalPopulation = 0;

    void setTotalClassifiers(const size_t & totalClassifiers);
    void copyCounts(const ClassifierMatrix & results);
    void setPopulationFromResults();

    void calcOverallAccuracy();

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
