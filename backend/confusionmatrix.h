#ifndef CONFUSIONMATRIX_H
#define CONFUSIONMATRIX_H

#include "backend/teststatistics.h"

class ConfusionMatrix
{
public:

    typedef std::vector<std::vector<size_t>> ClassifierCounts;
    typedef std::vector<std::vector<real>> ClassifierRatios;

    ConfusionMatrix();

    bool setValues(const ClassifierCounts & predictions, const ClassifierCounts & actuals);
    bool validate(const ClassifierCounts & predictions, const ClassifierCounts & actuals);

    TestStatistics getTestStatistics() { return _ts; }
    size_t numberOfClassifiers() const { return _predictions.size(); }

    void reset();

    const ClassifierCounts & getPredictions() { return _predictions; }
    const ClassifierRatios & getPredictionRatios() { return _predictionRatios; }
    const ClassifierCounts & getActuals() { return _actuals; }
    const ClassifierRatios & getActualRatios() { return _actualRatios; }

private:
    ClassifierCounts _predictions;
    ClassifierRatios _predictionRatios;
    ClassifierCounts _actuals;
    ClassifierRatios _actualRatios;
    TestStatistics _ts;

    void setTotalClassifiers(const size_t & totalClassifiers);
    void copyCounts(const ClassifierCounts & predictions, const ClassifierCounts & actuals);
    void buildRatiosFromCounts();
};

#endif // CONFUSIONMATRIX_H
