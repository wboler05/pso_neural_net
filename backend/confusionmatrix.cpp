#include "confusionmatrix.h"
#include "util.h"

/**
 * @brief ConfusionMatrix::ConfusionMatrix
 */
ConfusionMatrix::ConfusionMatrix()
{

}

ConfusionMatrix::ConfusionMatrix(const ClassifierMatrix & results) {
    setResults(results);
}

/**
 * @brief ConfusionMatrix::setResults
 * @param predictions
 * @param actuals
 * @return
 */
bool ConfusionMatrix::setResults(const ClassifierMatrix & results) {

    // Validate the size of predictions and actuals
    if (!validate(results)) {
        return false;
    }

    // Copy the new data internally
    copyCounts(results);

    // Create the finalized results
    constructTestResults();

    return true;
}

/**
 * @brief ConfusionMatrix::validate
 * @details Ensure that the results are appropriate for construction
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

/**
 * @brief ConfusionMatrix::copyCounts
 * @details Deep copies results internally
 * @param results
 */
void ConfusionMatrix::copyCounts(const ClassifierMatrix & results) {
    setTotalClassifiers(results.size());
    for (size_t i = 0; i < results.size(); i++) {
        for (size_t j = 0; j < results[i].size(); j++) {
            _resultValues[i][j] = results[i][j];
        }
    }
}

/**
 * @brief ConfusionMatrix::setTotalClassifiers
 * @details Resets and sets the number of classifiers
 * @param totalClassifiers
 */
void ConfusionMatrix::setTotalClassifiers(const size_t & totalClassifiers) {
    reset();
    _numberOfClassifiers = totalClassifiers;

    _resultValues.resize(totalClassifiers);
    _resultRatios.resize(totalClassifiers);
    for (size_t i = 0; i < totalClassifiers; i++) {
        _resultValues[i].resize(totalClassifiers, 0);
        _resultRatios[i].resize(totalClassifiers, 0.0);
    }

    _truePositiveValues.resize(totalClassifiers, 0);
    _falsePositiveValues.resize(totalClassifiers, 0);
    _falseNegativeValues.resize(totalClassifiers, 0);

    _truePositiveRatios.resize(totalClassifiers, 0.0);
    _falsePositiveRatios.resize(totalClassifiers, 0.0);
    _falseNegativeRatios.resize(totalClassifiers, 0.0);
}

/**
 * @brief ConfusionMatrix::reset
 * @details Clears all objects
 */
void ConfusionMatrix::reset() {
    _numberOfClassifiers = 0;
    _totalPopulation = 0;

    _resultValues.clear();
    _resultRatios.clear();

    _truePositiveValues.clear();
    _falsePositiveValues.clear();
    _falseNegativeValues.clear();

    _truePositiveRatios.clear();
    _falsePositiveRatios.clear();
    _falseNegativeRatios.clear();

    _overallStats.clear();
    _overallError.clear();
    _classStats.clear();
    _classErrors.clear();
}

/**
 * @brief ConfusionMatrix::constructTestResults
 * @details Business work for calculating TP, FP, and FN.
 */
void ConfusionMatrix::constructTestResults() {

    // Count how many events there are
    setPopulationFromResults();

    // Track the true positives
    countTruePositives();
    countFalsePositives();
    countFalseNegatives();

    calcOverallAccuracy();

    ///TEST

//    qDebug() << _overallStats.outputString(_overallError).c_str();

}

void ConfusionMatrix::calcOverallAccuracy() {
    _overallStats.clear();
    real tp = CustomMath::total(_truePositiveValues);
    real fp = static_cast<real>(_totalPopulation) - tp;
    _overallStats.addTp(tp);
    _overallStats.addTn(tp);
    _overallStats.addFp(fp);
    _overallStats.addFn(fp);
    _overallStats.getClassError(_overallError);
}

void ConfusionMatrix::costlyComputeClassStats() {
    _classStats.clear();
    _classStats.resize(_numberOfClassifiers);
    _classErrors.clear();
    _classErrors.resize(_numberOfClassifiers);
    for (size_t c = 0; c < _numberOfClassifiers; c++) {

        // True Pos / True Neg
        for (size_t diag = 0; diag < _numberOfClassifiers; diag++) {
            if (c == diag) {
                _classStats[c].addTp(_resultValues[diag][diag]);
            } else {
                _classStats[c].addTn(_resultValues[diag][diag]);
           }
        }

        // False Neg
        for (size_t act = 0; act < _numberOfClassifiers; act++) {
            if (c != act) {
                _classStats[c].addFn(_resultValues[act][c]);
            }
        }

        // False Pos
        for (size_t pre = 0; pre < _numberOfClassifiers; pre++) {
            if (c != pre) {
                _classStats[c].addFp(_resultValues[c][pre]);
            }
        }

        _classStats[c].getClassError(_classErrors[c]);
    }
}

/**
 * @brief ConfusionMatrix::setPopulationFromResults
 * @details Counts the number of experiments
 */
void ConfusionMatrix::setPopulationFromResults() {
    _totalPopulation = 0;
    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        for (size_t j = 0; j < _numberOfClassifiers; j++) {
            _totalPopulation += _resultValues[i][j];
        }
    }
    constructResultRatios();
}

/**
 * @brief ConfusionMatrix::constructResultRatios
 */
void ConfusionMatrix::constructResultRatios() {
    _resultRatios.clear();
    _resultRatios.resize(_numberOfClassifiers);

    real pop = static_cast<real>(_totalPopulation);

    for (size_t i = 0; i < _numberOfClassifiers; i++) {
        _resultRatios[i].resize(_numberOfClassifiers, 0.0);
        for (size_t j = 0; j < _numberOfClassifiers; j++) {
            _resultRatios[i][j] = static_cast<real>(_resultValues[i][j]) / pop;
        }
    }
}

/**
 * @brief ConfusionMatrix::countTruePositives
 */
void ConfusionMatrix::countTruePositives() {
    _truePositiveValues.clear();
    _truePositiveValues.resize(numberOfClassifiers(), 0);
    for (size_t diag = 0; diag < _numberOfClassifiers; diag++) {
        _truePositiveValues[diag] = _resultValues[diag][diag];
    }
    constructTruePositiveRatios();
}



/**
 * @brief ConfusionMatrix::countFalsePositives
 */
void ConfusionMatrix::countFalsePositives() {
    _falsePositiveValues.clear();
    _falsePositiveRatios.clear();
    _falsePositiveValues.resize(_numberOfClassifiers, 0);
    for (size_t acts = 0; acts < _numberOfClassifiers; acts++) {
        for (size_t pres = 0; pres < _numberOfClassifiers; pres++) {
            if (acts != pres) {
                _falsePositiveValues[acts] += _resultValues[acts][pres];
            }
        }
    }
    constructFalsePositiveRatios();
}



void ConfusionMatrix::countFalseNegatives() {
    _falseNegativeValues.clear();
    _falseNegativeRatios.clear();
    _falseNegativeValues.resize(_numberOfClassifiers, 0);
    for (size_t pres = 0; pres < _numberOfClassifiers; pres++) {
        for (size_t acts = 0; acts < _numberOfClassifiers; acts++) {
            if (acts != pres) {
                _falseNegativeValues[pres] += _resultValues[acts][pres];
            }
        }
    }
    constructFalseNegativeRatios();
}

/**
 * @brief ConfusionMatrix::constructTruePositiveRatios
 */
void ConfusionMatrix::constructTruePositiveRatios() {
    constructRatioVector(_truePositiveValues, _truePositiveRatios);
}

/**
 * @brief ConfusionMatrix::constructFalsePositiveRatios
 */
void ConfusionMatrix::constructFalsePositiveRatios() {
    constructRatioVector(_falsePositiveValues, _falsePositiveRatios);
}

/**
 * @brief ConfusionMatrix::constructFalseNegativeRatios
 */
void ConfusionMatrix::constructFalseNegativeRatios() {
    constructRatioVector(_falseNegativeValues, _falseNegativeRatios);
}

/**
 * @brief ConfusionMatrix::constructRatioVector
 * @details Constructs ratios to "r" from values of "v" vector using total population of events
 * @param v - valued vector
 * @param r - ratio vector
 */
void ConfusionMatrix::constructRatioVector(ClassifierVector & v, ClassifierVectorRatios & r) {
    r.resize(_numberOfClassifiers, 0.0);
    if (_totalPopulation > 0) {
        for (size_t i = 0; i < _numberOfClassifiers; i++) {
            r[i] = static_cast<real>(v[i]) / static_cast<real>(_totalPopulation);
        }
    }
}

std::string ConfusionMatrix::toString() {
    std::string s("ConfusionMatrix\n");
    for (size_t pre = 0; pre < _numberOfClassifiers + 2; pre++) {
        for (size_t act = 0; act < _numberOfClassifiers + 2; act++) {
            if (act == 0 && pre == 0) {
                s.push_back('\t');
            } else if (pre == 0 && act < _numberOfClassifiers + 1) {
                s.push_back('A');
                s += stringPut(act+1);
                s.push_back('\t');
            } else if (pre == 0 && act == _numberOfClassifiers + 1) {
                s += "FN";
            } else if (act == 0 && pre < _numberOfClassifiers + 1) {
                s.push_back('P');
                s += stringPut(pre+1);
                s.push_back('\t');
            } else if (act == 0 && pre == _numberOfClassifiers + 1) {
                s += "FP\t";
            } else if (act == _numberOfClassifiers+1 && pre == _numberOfClassifiers+1) {
                s += stringPut(_overallError.accuracy);
            } else if (act == _numberOfClassifiers+1) {
                s += stringPut(_falseNegativeValues[pre-1]);
            } else if (pre == _numberOfClassifiers+1) {
                s += stringPut(_falsePositiveValues[act-1]);
                s.push_back('\t');
            } else if (act < _numberOfClassifiers + 1 && pre < _numberOfClassifiers + 1) {
                s += stringPut(_resultValues[act-1][pre-1]);
                s.push_back('\t');
            }
        }
        s.push_back('\n');
    }
    return s;
}

void ConfusionMatrix::print() {
    qDebug() << toString().c_str();
}

/**
 * @brief ConfusionMatrix::evaluateResults
 * @details Process a vector of results and return as a matrix;
 * @param predicted
 * @param actual
 * @return
 */
ConfusionMatrix::ClassifierMatrix ConfusionMatrix::evaluateResults(const std::vector<std::vector<real>> predicted, const std::vector<std::vector<real>> actual) {
    ClassifierMatrix result;
    if (predicted.size() != actual.size()) {
        qWarning() << "Error, unable to process results. Number of events mismatch. ConfusionMatrix::evaluateResults";
        return result;
    }

    if (predicted.size() == 0) {
        qWarning() << "Error, empty set of data to be processed. ConfusionMatrix::evaluateResults";
        return result;
    }

    size_t totalClassifiers = predicted[0].size();
    for (size_t i = 1; i < predicted.size(); i++) {
        if (predicted[i].size() != actual[i].size() || predicted[i].size() != totalClassifiers) {
            qWarning() << "Error, failed to validate results. ConfusionMatrix::evaluateResults";
            return result;
        }
    }

    result.resize(totalClassifiers);
    for (size_t i = 0; i < totalClassifiers; i++) {
        result[i].resize(totalClassifiers, 0);
    }

    for (size_t events = 0; events < predicted.size(); events++) {
        real maxPredictedValue = -std::numeric_limits<real>::max();
        real maxActualValue = -std::numeric_limits<real>::max();
        size_t maxPredictedIt = 0;
        size_t maxActualIt = 0;
        for (size_t c = 0; c < totalClassifiers; c++) {
            if (predicted[events][c] > maxPredictedValue) {
                maxPredictedValue = predicted[events][c];
                maxPredictedIt = c;
            }
            if (actual[events][c] > maxActualValue) {
                maxActualValue = actual[events][c];
                maxActualIt = c;
            }
        }
        result[maxActualIt][maxPredictedIt]++;
    }

    return result;
}






/**
 * @brief ConfusionMatrix::MSE
 * @details Find the MSE for a set of experiments
 * @param results
 * @param expecteds
 * @return
 */
real ConfusionMatrix::MSE(
        const std::vector<std::vector<real> > &results,
        const std::vector<std::vector<real> > &expecteds) {

    // Verify that their sizes match
    if (results.size() != expecteds.size()) {
        return std::numeric_limits<real>::infinity();
    }

    // Verify that they are both not empty
    if (results.size() == 0) {
        return std::numeric_limits<real>::infinity();
    }

    // Get us our error variable set to zero
    real error = 0;

    // Loop through each event
    //#pragma omp parallel for
    for (size_t i = 0; i < results.size(); i++) {

        // Get the error for each event seperately
        real tError = MSE(results[i], expecteds[i]);

        // If inifinity, return infinit (we broke it)
        if (tError == std::numeric_limits<real>::infinity()) {
            return std::numeric_limits<real>::infinity();
        }

        // Else add the error to the summation
        error += tError;
    }
    // Divide by the vector size
    error /= results.size();

    // Return our error
    return error;
}


/**
 * @brief ConfusionMatrix::MSE
 * @details Calculate the MSE for a single event
 * @param result
 * @param expected
 * @return
 */
real ConfusionMatrix::MSE(const std::vector<real> &result, const std::vector<real> &expected) {
    // Verify that their sizes match
    if (result.size() != expected.size()) {
        return std::numeric_limits<real>::infinity();
    }

    // Verify that they are both not empty
    if (result.size() == 0) {
        return std::numeric_limits<real>::infinity();
    }

    // Get us our error variable set to zero
    real error = 0;

    // Sum the squared errors
    // #pragma omp parallel for
    for (size_t i = 0; i < result.size(); i++) {
        error += CustomMath::pow((result[i] - expected[i]), 2);
    }
    // Divide by total error size
    error /= result.size();

    // Return our error
    return error;
}

std::vector<real> ConfusionMatrix::splitMSE(const std::vector<real> & result, const std::vector<real> & expected) {
    std::vector<real> mse;

    if (result.size() != expected.size()) {
        return mse;
    } else {
        mse.resize(result.size(),std::numeric_limits<real>::infinity());
    }
    for (size_t i = 0; i < mse.size(); i++) {
        mse[i] = CustomMath::pow(result[i]-expected[i], 2);
    }
    return mse;
}
