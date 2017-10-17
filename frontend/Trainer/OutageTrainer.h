#ifndef OUTAGETRAINER_H
#define OUTAGETRAINER_H

#include "neuralpso.h"
#include "outagedataitem.h"

#include "inputcache.h"
#include "util.h"

struct EnableParameters {
    bool year=true;
    bool month=true;
    bool day=true;
    bool temp_high=true;
    bool temp_avg=true;
    bool temp_low=true;
    bool dew_high=true;
    bool dew_avg=true;
    bool dew_low=true;
    bool humidity_high=true;
    bool humidity_avg=true;
    bool humidity_low=true;
    bool press_high=true;
    bool press_avg=true;
    bool press_low=true;
    bool visibility_high=true;
    bool visibility_avg=true;
    bool visibility_low=true;
    bool wind_high=true;
    bool wind_avg=true;
    bool wind_gust=true;
    bool precipitation=true;
    bool fog=true;
    bool rain=true;
    bool snow=true;
    bool thunderstorm=true;
    bool loa=true;
    bool latitude=true;
    bool longitude=true;
    bool outage=true;
    bool affected_people=true;

    std::vector<size_t> inputSkips();
    std::vector<size_t> outputSkips();
};

struct TrainingParameters {
    PsoParams pp;
    NeuralNet::NeuralNetParameters np;
    FitnessParameters fp;
    CacheParameters cp;
    EnableParameters ep;
    real alpha = 1.0L;
    real beta = 1.0L;
    real gamma = 1.0L;
};

class OutageTrainer : public NeuralPso
{

public:

    OutageTrainer(const std::shared_ptr<TrainingParameters> & pe,
                  const std::shared_ptr<InputCache> & inputCache);

    void build();
    void randomlyDistributeData();

    void biasAgainstOutputs();
    void biasAgainstLOA();

    real trainingRun();
    void testGB();
    void validateGB();
    TestStatistics::ClassificationError && validateCurrentNet();

    size_t randomizeTrainingInputs();
    void runTrainer();
    OutageDataWrapper && loadTestInput(const size_t & I);
    OutageDataWrapper && loadValidationInput(const size_t &I);

    void classError(const std::vector<size_t> &testInputs,
                    TestStatistics &testStats,
                    TestStatistics::ClassificationError &ce,
                    const size_t &testIterations);
    TestStatistics & testStats() { return _testStats; }

    static bool confirmOutage(const std::vector<real> & output);

    void setFunctionMsg(std::string s) { _functionMsg = s; }
    std::string functionMsg() { return _functionMsg; }

    const std::shared_ptr<InputCache> & inputCache() { return _inputCache; }
    void setInputCache(std::shared_ptr<InputCache> & inputCache) { _inputCache = inputCache; }

    void updateEnableParameters();
    const EnableParameters & enableParameters() { return _params->ep; }

private:
    std::shared_ptr<TrainingParameters> _params;
    TestStatistics _testStats;
    TestStatistics _validationStats;
    std::shared_ptr<InputCache> _inputCache;
    std::vector<size_t> _trainingInputs;
    std::vector<size_t> _testInputs;
    std::vector<size_t> _validationInputs;
    std::vector<real> _biasedTrainingInputsCounts;
    std::vector<std::vector<size_t>> _biasedTrainingInputs;
    std::vector<size_t> _inputSkips;
    std::vector<size_t> _outputSkips;

    std::string _functionMsg;

    std::vector<real> _minData;
    std::vector<real> _maxData;

    void updateMinMax();
    std::vector<real> normalizeInput(const size_t & id);
    std::vector<real> normalizeInput(std::vector<real> & input);

    bool validateOutput(
            const std::vector<real> & outputs,
            const std::vector<real> &expectedResult,
            std::vector<real> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);

};

#endif // OUTAGETRAINER_H
