#ifndef OUTAGETRAINER_H
#define OUTAGETRAINER_H

#include "neuralpso.h"
#include "outagedataitem.h"

#include "inputcache.h"
#include "util.h"

struct TrainingParameters {
    PsoParams pp;
    NeuralNet::NeuralNetParameters np;
    FitnessParameters fp;
    CacheParameters cp;
};

class OutageTrainer : public NeuralPso
{

public:

    OutageTrainer(const std::shared_ptr<TrainingParameters> & pe);

    void build();
    void randomlyDistributeData();

    void biasAgainstOutputs();
    void biasAgainstLOA();

    real trainingRun(real &correctRatio, uint &totalCount, real &confidence);
    void testGB();
    void validateGB();

    int randomizeTestInputs();
    void runTrainer();
    OutageDataWrapper & loadTestInput(const size_t & I);
    OutageDataWrapper &loadValidationInput(const size_t &I);

    void classError(TestStatistics::ClassificationError * ce);
    TestStatistics & testStats() { return _testStats; }

    static bool confirmOutage(const std::vector<real> output);

    void setFunctionMsg(std::string s) { _functionMsg = s; }
    std::string functionMsg() { return _functionMsg; }

private:
    std::shared_ptr<TrainingParameters> _params;
    TestStatistics _testStats;
    TestStatistics _validationStats;
    std::shared_ptr<InputCache> _inputCache;
    std::vector<size_t> _trainingInputs;
    std::vector<size_t> _testInputs;
    std::vector<size_t> _validationInputs;
    std::vector<real> _outputCount;
    std::vector<std::vector<size_t>> _outputIterators;

    std::string _functionMsg;

    bool validateOutput(
            const std::vector<real> & outputs,
            const std::vector<real> &expectedResult,
            std::vector<real> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);

};

#endif // OUTAGETRAINER_H
