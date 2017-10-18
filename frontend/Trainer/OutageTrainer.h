#ifndef OUTAGETRAINER_H
#define OUTAGETRAINER_H

#include "neuralpso.h"
#include "outagedataitem.h"

#include "inputcache.h"
#include "util.h"

struct EnableParameters {
    bool year=false;
    bool month=false;
    bool day=false;
    bool temp_high=true;
    bool temp_avg=true;
    bool temp_low=true;
    bool dew_high=true;
    bool dew_avg=true;
    bool dew_low=true;
    bool humidity_high=false;
    bool humidity_avg=false;
    bool humidity_low=false;
    bool press_high=false;
    bool press_avg=false;
    bool press_low=false;
    bool visibility_high=false;
    bool visibility_avg=false;
    bool visibility_low=false;
    bool wind_high=true;
    bool wind_avg=false;
    bool wind_gust=true;
    bool precipitation=true;
    bool fog=false;
    bool rain=true;
    bool snow=false;
    bool thunderstorm=true;
    bool loa=false;
    bool latitude=false;
    bool longitude=false;
    bool outage=true;
    bool affected_people=true;

    std::vector<size_t> inputSkips();
    std::vector<size_t> outputSkips();
};

struct SelectedGlobalBest {
    NeuralNet::State state;
    TestStatistics testStats;
    TestStatistics::ClassificationError ce;
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
    bool showBestSelected = false;
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

    void trainingRun();
    real trainingStep(const std::vector<size_t> & trainingInputs);
    void testGB();
    void testSelectedGB();
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

    SelectedGlobalBest & getRecentGlobalBest() { return _recent_gb; }
    SelectedGlobalBest & getSelectedGlobalBest() { return _best_gb; }

private:
    SelectedGlobalBest _recent_gb;
    SelectedGlobalBest _best_gb;
    std::shared_ptr<TrainingParameters> _params;
    TestStatistics _testStats;
    TestStatistics _validationStats;
    TestStatistics _selectedTestStats;
    std::shared_ptr<InputCache> _inputCache;
    std::vector<size_t> _trainingInputs;
    std::vector<size_t> _testInputs;
    std::vector<size_t> _validationInputs;
    std::vector<real> _biasedTrainingInputsCounts;
    std::vector<std::vector<size_t>> _biasedTrainingInputs;
    std::vector<size_t> _inputSkips;
    std::vector<size_t> _outputSkips;

    std::string _functionMsg;

    std::vector<real> _minInputData;
    std::vector<real> _maxInputData;
    real _minOutputRegression;
    real _maxOutputRegression;

    void updateMinMax();
    std::vector<real> normalizeInput(const size_t & id);
    std::vector<real> normalizeInput(std::vector<real> & input);

    void postProcess(std::vector<real> & outputs);

    bool validateOutput(
            const std::vector<real> & outputs,
            const std::vector<real> &expectedResult,
            std::vector<real> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);
    bool networkPathValidation();

};

#endif // OUTAGETRAINER_H
