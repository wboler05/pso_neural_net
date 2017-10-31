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

    bool setValues(const ClassifierMatrix & results);
    bool validate(const ClassifierMatrix & results);

    TestStatistics getTestStatistics() { return _ts; }
    size_t numberOfClassifiers() const { return _results.size(); }

    void reset();

    const ClassifierMatrix & getResults() { return _results; }
    const ClassifierMatrixRatios & getResultRatios() { return _resultRatios; }

private:
    ClassifierMatrix _results;
    ClassifierMatrixRatios _resultRatios;
    ClassifierVector _falsePositives;
    ClassifierVector _falseNegatives;
    TestStatistics _ts;

    void setTotalClassifiers(const size_t & totalClassifiers);
    void copyCounts(const ClassifierMatrix & results);
    void buildRatiosFromCounts();
    void constructTestResults();
};

#endif // CONFUSIONMATRIX_H
