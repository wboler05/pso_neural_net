#include "teststatistics.h"

TestStatistics::TestStatistics()
{
    clear();
}

void TestStatistics::clear() {
    _test.trueNegative = 0;
    _test.truePositive = 0;
    _test.falseNegative = 0;
    _test.falsePositive = 0;
    _population = 0;
    _mse = 0;
}

void TestStatistics::getClassError(ClassificationError & ce) {

    ce.accuracy = N_Accuracy(tp(), tn(), fp(), fn());
    ce.precision = N_Precision(tp(), fp());
    ce.sensitivity = N_Sensitivity(tp(), fn());
    ce.specificity = N_Specificity(tn(), fp());
    ce.f_score = N_F_Score(tp(), fp(), fn());
}

TestStatistics::TestStruct TestStatistics::testStruct_norm() {
    TestStruct ts;
    ts.falseNegative = fn_norm();
    ts.falsePositive = fp_norm();
    ts.truePositive = tp_norm();
    ts.trueNegative = tn_norm();
    return ts;
}

std::string TestStatistics::outputString(const ClassificationError &ce) {

    TestStatistics::TestStruct tsNorm = testStruct_norm();

    std::string outputString;
    outputString += "Out\\Act\tFalse\t\tTrue\n";
    outputString += "False\t";
    outputString += stringPut(tsNorm.trueNegative);
    outputString += "\t";
    outputString += stringPut(tsNorm.falsePositive);
    outputString += "\t";
    outputString += stringPut(tsNorm.trueNegative + tsNorm.falsePositive);
    outputString += "\nTrue\t";
    outputString += stringPut(tsNorm.falseNegative);
    outputString += "\t";
    outputString += stringPut(tsNorm.truePositive);
    outputString += "\t";
    outputString += stringPut(tsNorm.falseNegative + tsNorm.truePositive);
    outputString += "\n\t";
    outputString += stringPut(tsNorm.trueNegative + tsNorm.falseNegative);
    outputString += "\t";
    outputString += stringPut(tsNorm.falsePositive + tsNorm.truePositive);

    outputString += "\n\tAccuracy [ (tp + tn) / testSize ]: ";
    outputString += stringPut(ce.accuracy);

    outputString += "\n\tPrecision [ tp / (tp + fp) ]:      ";
    outputString += stringPut(ce.precision);

    outputString += "\n\tSensitivity [ tp / (tp + fn) ]:    ";
    outputString += stringPut(ce.sensitivity);

    outputString += "\n\tSpecificity [ tn / (tn + fp) ]:    ";
    outputString += stringPut(ce.specificity);

    outputString += "\n\tF-Score [ 2tp / (2tp + fp + fn) ]: ";
    outputString += stringPut(ce.f_score);

    outputString += "\n";
    return outputString;
}
