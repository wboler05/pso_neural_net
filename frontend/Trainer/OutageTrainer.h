#ifndef OUTAGETRAINER_H
#define OUTAGETRAINER_H

#include "neuralpso.h"
#include "outagedataitem.h"
#include "confusionmatrix.h"
#include "datapartioner.h"

#include "inputcache.h"
#include "statobject.h"
#include "util.h"

struct EnableParameters {
    bool year=false;
    bool month=false;
    bool day=true;
    bool temp_high=false;
    bool temp_avg=true;
    bool temp_low=false;
    bool dew_high=false;
    bool dew_avg=true;
    bool dew_low=false;
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

    std::vector<size_t> inputSkips();
};

struct GlobalBestObject {
    NeuralNet::State state;
    ConfusionMatrix cm;
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
    bool enableBaseCase = false;
};

class OutageTrainer : public NeuralPso
{

public:

    OutageTrainer(const std::shared_ptr<TrainingParameters> & pe,
                  const std::shared_ptr<InputCache> & inputCache);

    void build();
    void partitionData(int kFolds);
    size_t getNextValidationSet();

    void calcImplicitBiasWeights();
    void biasAgainstLOA();

    void trainingRun();
    real trainingStep(const dataPartioner & dataSets);
    real trainingStepBaseCase();
    void testGB();
    void testSelectedGB();
    void validateGB();
    TestStatistics::ClassificationError && validateCurrentNet();

    void runTrainer();
    OutageDataWrapper && loadTestInput(const size_t & I);
    OutageDataWrapper && loadValidationInput(const size_t &I);

    void classError(const std::vector<size_t> &testInputs,
                    ConfusionMatrix & cm,
                    const size_t &testIterations);
    //TestStatistics & testStats() { return _testStats; }
    const ConfusionMatrix & testConfusionMatrix() { return _testConfusionMatrix; }
    const ConfusionMatrix & validationConfusionMatrix() { return _validationConfusionMatrix; }
    const ConfusionMatrix & selectedConfusionMatrix() { return _selectedConfusionMatrix; }

    static bool confirmOutage(const std::vector<real> & output);

    void setFunctionMsg(std::string s) { _functionMsg = s; }
    std::string functionMsg() { return _functionMsg; }

    const std::shared_ptr<InputCache> & inputCache() { return _inputCache; }
    void setInputCache(std::shared_ptr<InputCache> & inputCache) { _inputCache = inputCache; }

    void updateEnableParameters();
    const EnableParameters & enableParameters() { return _params->ep; }

    GlobalBestObject & getRecentGlobalBest() { return _recent_gb; }
    GlobalBestObject & getSelectedGlobalBest() { return _best_gb; }

    // Test
    std::vector<StatObject> _outputNodeStats;

private:

    dataPartioner _dataSets;
    GlobalBestObject _recent_gb;
    GlobalBestObject _best_gb;
    std::shared_ptr<TrainingParameters> _params;
    ConfusionMatrix _testConfusionMatrix;
    ConfusionMatrix _validationConfusionMatrix;
    ConfusionMatrix _selectedConfusionMatrix;
    std::shared_ptr<InputCache> _inputCache;
    std::vector<real> _implicitBiasWeights;
    std::vector<size_t> _trueNumElesPerClass;
    std::vector<real> _equalizationFactors;
    real _fitnessNormalizationFactor;
    std::vector<size_t> _inputSkips;
    size_t _numClasses;

    std::string _functionMsg;
    std::vector<real> _minInputData;
    std::vector<real> _maxInputData;

    void updateMinMax();
    std::vector<real> normalizeInput(const size_t & id);
    std::vector<real> normalizeInput(std::vector<real> & input);

    bool validateOutput(
            const std::vector<real> & outputs,
            const std::vector<real> &expectedResult,
            std::vector<real> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);
    bool networkPathValidation();

};

#endif // OUTAGETRAINER_H
