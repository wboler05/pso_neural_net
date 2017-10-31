#include "confusionmatrix.h"

/**
 * @brief ConfusionMatrix::ConfusionMatrix
 */
ConfusionMatrix::ConfusionMatrix()
{

}

/**
 * @brief ConfusionMatrix::setValues
 * @param predictions
 * @param actuals
 * @return
 */
bool ConfusionMatrix::setValues(const ClassifierMatrix & results) {

    // Validate the size of predictions and actuals
    if (!validate(results)) {
        return false;
    }

    // Set the size of the classifier
    size_t totalClassifiers = results.size();
    setTotalClassifiers(totalClassifiers);

    copyCounts(results);
    buildRatiosFromCounts();

    return true;
}

/**
 * @brief ConfusionMatrix::validate
 * @param predictions
 * @param actuals
 * @return
 */
bool ConfusionMatrix::validate(const ClassifierMatrix & results) {
    if (results.size() == 0) {
        return false;
    }

    size_t validSize = results.size();

    for (size_t i = 0; i < results.size(); i++) {
        if (results[i].size() != validSize) {
            return false;
        }
    }
    return true;
}

void ConfusionMatrix::setTotalClassifiers(const size_t & totalClassifiers) {
    reset();
    _results.resize(totalClassifiers);
    _resultRatios.resize(totalClassifiers);

    for (size_t i = 0; i < totalClassifiers; i++) {
        _results[i].resize(totalClassifiers, 0);
        _resultRatios[i].resize(totalClassifiers, 0.0);
    }
}

/**
 * @brief ConfusionMatrix::reset
 */
void ConfusionMatrix::reset() {
    _results.clear();
    _resultRatios.clear();
    _ts.clear();
}

void ConfusionMatrix::copyCounts(const ClassifierMatrix & results) {
    for (size_t i = 0; i < results.size(); i++) {
        for (size_t j = 0; j < results[i].size(); j++) {
            _results[i][j] = results[i][j];
        }
    }
}

/**
 * @brief ConfusionMatrix::buildRatiosFromCounts
 */
void ConfusionMatrix::buildRatiosFromCounts() {

    real population = 0;

    for (size_t i = 0; i < _results.size(); i++) {
        for (size_t j = 0; j < _results[i].size(); j++) {
            population += _results[i][j];
        }
    }

    for (size_t i = 0; i < _results.size(); i++) {
        for (size_t j = 0; j < _results[i].size(); j++) {

            _resultRatios[i][j] = static_cast<real>(_results[i][j]) / population;

        }
    }
}

void ConfusionMatrix::constructTestResults() {
    // Test This out

    _ts.clear();

    size_t totalClassifiers = _results.size();

    _falsePositives.resize(totalClassifiers, 0);
    _falseNegatives.resize(totalClassifiers, 0);

    size_t population = 0;

    // Calc Population
    for (size_t i = 0; i < totalClassifiers; i++) {
        for (size_t j = 0; j < totalClassifiers; j++) {
            population += _results[i][j];
        }
    }

    // Do Tp
    for (size_t i = 0; i < totalClassifiers; i++) {
        _ts.addTp(_results[i][i]);
    }

    // Do Fp
    for (size_t i = 0; i < totalClassifiers; i++) {
        for (size_t j = 0; j < totalClassifiers; j++) {
            if (i != j) {
                _falseNegatives[i] += _results[i][j];
                _falsePositives[j] += _results[i][j];
            }
        }
    }

    real avgFN = 0;
    real avgFP = 0;
    for (size_t i = 0; i < totalClassifiers; i++) {
        avgFN += static_cast<real>(_falseNegatives[i]);
        avgFP += static_cast<real>(_falsePositives[i]);
    }
    avgFN /= static_cast<real>(totalClassifiers);
    avgFP /= static_cast<real>(totalClassifiers);

    size_t newFN = static_cast<size_t>(round(avgFN));
    size_t newFP = static_cast<size_t>(round(avgFP));

    _ts.addFn(newFN);
    _ts.addFp(newFP);
    _ts.addTn(population - (newFN + newFP + static_cast<size_t>(round(_ts.tp()))));

}
