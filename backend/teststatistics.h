#ifndef TESTSTATISTICS_H
#define TESTSTATISTICS_H

#include <cinttypes>
#include <string>

#include "util.h"
#include "custommath.h"

#define N_Accuracy(tp, tn, fp, fn) ((tp + tn) / (tp + tn + fp + fn))
#define N_Precision(tp, fp) (tp / (tp + fp + 1.0L))
#define N_Sensitivity(tp, fn) (tp / (tp + fn + 1.0L))
#define N_Specificity(tn, fp) (tn / (tn + fp + 1.0L))
#define N_F_Score(tp, fp, fn) (2.0L*tp / (2.0L*tp + fp + fn + 1.0L))

class TestStatistics
{
public:
    TestStatistics();

    struct TestStruct {
        real trueNegative=0;
        real truePositive=0;
        real falseNegative=0;
        real falsePositive=0;
    };

    struct ClassificationError {
        real accuracy = 0;
        real precision = 0;
        real sensitivity = 0;
        real specificity = 0;
        real f_score = 0;
        real mse = 0;
    };

    const TestStruct & testStruct() { return _test; }
    TestStruct testStruct_norm();

    const real & tn() { return _test.trueNegative; }
    const real & tp() { return _test.truePositive; }
    const real & fn() { return _test.falseNegative; }
    const real & fp() { return _test.falsePositive; }

    real tn_norm() { return _test.trueNegative / static_cast<real>(_population); }
    real tp_norm() { return _test.truePositive / static_cast<real>(_population); }
    real fn_norm() { return _test.falseNegative / static_cast<real>(_population); }
    real fp_norm() { return _test.falsePositive / static_cast<real>(_population); }

    void addTn() { _test.trueNegative++; _population++; }
    void addTp() { _test.truePositive++; _population++; }
    void addFn() { _test.falseNegative++; _population++; }
    void addFp() { _test.falsePositive++; _population++; }

    const uint64_t & population() { return _population; }
    void clear();

    void getClassError(ClassificationError & ce);
    std::string outputString(const ClassificationError &ce);

private:
    TestStruct _test;
    uint64_t _population=0;
};

#endif // TESTSTATISTICS_H
