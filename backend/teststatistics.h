#ifndef TESTSTATISTICS_H
#define TESTSTATISTICS_H

#include <cinttypes>
#include <string>

#include "util.h"

#define N_Accuracy(tp, tn, fp, fn) ((tp + tn) / (tp + tn + fp + fn))
#define N_Precision(tp, fp) (tp / (tp + fp + 1.0f))
#define N_Sensitivity(tp, fn) (tp / (tp + fn + 1.0))
#define N_Specificity(tn, fp) (tn / (tn + fp + 1.0))
#define N_F_Score(tp, fp, fn) (2.0f*tp / (2.0f*tp + fp + fn + 1.0))

class TestStatistics
{
public:
    TestStatistics();

    struct TestStruct {
        double trueNegative=0;
        double truePositive=0;
        double falseNegative=0;
        double falsePositive=0;
    };

    struct ClassificationError {
        double accuracy;
        double precision;
        double sensitivity;
        double specificity;
        double f_score;
    };

    const TestStruct & testStruct() { return _test; }
    TestStruct testStruct_norm();

    const double & tn() { return _test.trueNegative; }
    const double & tp() { return _test.truePositive; }
    const double & fn() { return _test.falseNegative; }
    const double & fp() { return _test.falsePositive; }

    double tn_norm() { return _test.trueNegative / (double) _population; }
    double tp_norm() { return _test.truePositive / (double) _population; }
    double fn_norm() { return _test.falseNegative / (double) _population; }
    double fp_norm() { return _test.falsePositive / (double) _population; }

    void addTn() { _test.trueNegative++; _population++; }
    void addTp() { _test.truePositive++; _population++; }
    void addFn() { _test.falseNegative++; _population++; }
    void addFp() { _test.falsePositive++; _population++; }

    const uint64_t & population() { return _population; }
    void clear();

    void getClassError(ClassificationError *ce);
    std::string outputString(ClassificationError *ce);

private:
    TestStruct _test;
    uint64_t _population=0;
};

#endif // TESTSTATISTICS_H
