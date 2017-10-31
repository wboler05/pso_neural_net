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
bool ConfusionMatrix::setValues(const ClassifierCounts & predictions, const ClassifierCounts & actuals) {

    // Validate the size of predictions and actuals
    if (!validate(predictions, actuals)) {
        return false;
    }

    // Set the size of the classifier
    size_t totalClassifiers = predictions.size();
    setTotalClassifiers(totalClassifiers);

    copyCounts(predictions, actuals);
    buildRatiosFromCounts();

    return true;
}

/**
 * @brief ConfusionMatrix::validate
 * @param predictions
 * @param actuals
 * @return
 */
bool ConfusionMatrix::validate(const ClassifierCounts & predictions, const ClassifierCounts & actuals) {
    if (predictions.size() != actuals.size()) {
        return false;
    }

    for (size_t i = 0; i < predictions.size(); i++) {
        if (predictions[i].size() != actuals[i].size()) {
            return false;
        }
    }
    return true;
}

void ConfusionMatrix::setTotalClassifiers(const size_t & totalClassifiers) {
    reset();
    _predictions.resize(totalClassifiers);
    _predictionRatios.resize(totalClassifiers);
    _actuals.resize(totalClassifiers);
    _actualRatios.resize(totalClassifiers);

    for (size_t i = 0; i < totalClassifiers; i++) {
        _predictions[i].resize(totalClassifiers, 0);
        _predictionRatios[i].resize(totalClassifiers, 0.0);
        _actuals[i].resize(totalClassifiers, 0);
        _actualRatios[i].resize(totalClassifiers, 0.0);
    }
}

/**
 * @brief ConfusionMatrix::reset
 */
void ConfusionMatrix::reset() {
    _predictions.clear();
    _predictionRatios.clear();
    _actuals.clear();
    _actualRatios.clear();
    _ts.clear();
}

void ConfusionMatrix::copyCounts(const ClassifierCounts & predictions, const ClassifierCounts & actuals) {
    for (size_t i = 0; i < predictions.size(); i++) {
        for (size_t j = 0; j < predictions[i].size(); j++) {

            _predictions[i][j] = predictions[i][j];
            _actuals[i][j] = actuals[i][j];

        }
    }
}

/**
 * @brief ConfusionMatrix::buildRatiosFromCounts
 */
void ConfusionMatrix::buildRatiosFromCounts() {

    size_t predictionMax = 0;
    size_t actualMax = 0;

    for (size_t i = 0; i < _predictions.size(); i++) {
        for (size_t j = 0; j < _predictions[i].size(); j++) {

            predictionMax = std::max(predictionMax, _predictions[i][j]);
            actualMax = std::max(actualMax, _actuals[i][j]);

        }
    }

    real realPredictionMax = static_cast<real>(predictionMax);
    real realActualMax = static_cast<real>(actualMax);

    for (size_t i = 0; i < _predictions.size(); i++) {
        for (size_t j = 0; j < _predictions[i].size(); j++) {

            _predictionRatios[i][j] = static_cast<real>(_predictions[i][j]) / realPredictionMax;
            _actualRatios[i][j] = static_cast<real>(_actuals[i][j]) / realActualMax;

        }
    }
}
