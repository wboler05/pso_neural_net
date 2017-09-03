#ifndef PETRAINER_H
#define PETRAINER_H

#include "neuralpso.h"

struct TrainingParameters {
    PsoParams pp;
    NeuralNetParameters np;
    FitnessParameters fp;
};

class ANDTrainer : public NeuralPso
{

public:
    struct InputCache {
        bool a;
        bool b;
    };

    ANDTrainer(const TrainingParameters & pe);

    void build(std::vector<std::vector<double>> &input, std::vector<double> &output);

    double testRun(double &correctRatio, uint &totalCount, double &confidence);
    void testGB();

    int randomizeTestInputs();
    void runTrainer();
    void loadTestInput(uint32_t I);
    void loadValidationInput(size_t I);

    void classError(TestStatistics::ClassificationError * ce);
    TestStatistics & testStats() { return _testStats; }

    static bool convertOutput(const double & output);
    static double convertInput(const bool & b);

    void setFunctionMsg(std::string s) { _functionMsg = s; }
    std::string functionMsg() { return _functionMsg; }

private:
    TestStatistics _testStats;
    std::vector<std::vector<double> > *_input;
    std::vector<double> *_output;
    std::vector<double> *_outputCount;
    std::vector<std::vector<uint>> _outputIterators;

    std::string _functionMsg;

    bool validateOutput(
            const std::vector<double> & outputs,
            const std::vector<double> &expectedResult,
            std::vector<double> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);

};

#endif // PETRAINER_H
