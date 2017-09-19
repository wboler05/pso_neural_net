#ifndef OUTAGETRAINER_H
#define OUTAGETRAINER_H

#include "neuralpso.h"
#include "outagedataitem.h"

#include "inputcache.h"

struct TrainingParameters {
    PsoParams pp;
    NeuralNet::NeuralNetParameters np;
    FitnessParameters fp;
};

class OutageTrainer : public NeuralPso
{

public:

    OutageTrainer(const TrainingParameters & pe);

    void build(std::vector<std::vector<real>> &input, std::vector<real> &output);

    real testRun(real &correctRatio, uint &totalCount, real &confidence);
    void testGB();

    int randomizeTestInputs();
    void runTrainer();
    void loadTestInput(uint32_t I);
    void loadValidationInput(size_t I);

    void classError(TestStatistics::ClassificationError * ce);
    TestStatistics & testStats() { return _testStats; }

    static bool convertOutput(const real & output);
    static real convertInput(const bool & b);

    void setFunctionMsg(std::string s) { _functionMsg = s; }
    std::string functionMsg() { return _functionMsg; }

private:
    TestStatistics _testStats;
    std::vector<std::vector<real> > *_input;
    std::vector<real> *_output;
    std::vector<real> *_outputCount;
    std::vector<std::vector<uint>> _outputIterators;

    std::string _functionMsg;

    bool validateOutput(
            const std::vector<real> & outputs,
            const std::vector<real> &expectedResult,
            std::vector<real> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);

};

#endif // OUTAGETRAINER_H
