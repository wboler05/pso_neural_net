#ifndef OUTAGETRAINER_H
#define OUTAGETRAINER_H

#include "neuralpso.h"
#include "outagedataitem.h"
#include "confusionmatrix.h"
#include "datapartioner.h"
#include "trainingparameters.h"

#include "inputcache.h"
#include "statobject.h"
#include "util.h"

class OutageTrainer : public NeuralPso
{

public:

    OutageTrainer(const std::shared_ptr<TrainingParameters> & pe,
                  const std::shared_ptr<InputCache> & inputCache);

    void build();
    void partitionData(int kFolds, size_t numClasses);
    size_t getNextValidationSet();

    void trainingRun();
    real trainingStep(const vector<size_t> & trainingVector);
    real trainingStepBaseCase();
    void testGb();
    void testSelectedGB();
    void validateGb();
    TestStatistics::ClassificationError && validateCurrentNet();

    void runTrainer();

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

    void fullTestState();

    // Test
    std::vector<StatObject> _outputNodeStats;

    GlobalBestObject & sanityCheckGb() { return _sanityCheck_gb; }

    void stopValidation() { _stopValidation = true; }
    void resetFitnessScores();

    const size_t & currentFold() { return _dataSets.foldIndex(); }

    void setCustomNet(const GlobalBestObject & customNet) { _customNet = customNet;}
    GlobalBestObject customNet() { return _customNet; }
    void runFullTestOnCustomNet(const NeuralNet::NeuralNetParameters & params);

private:

    DataPartioner _dataSets;
    std::vector<GlobalBestObject> _validatedBests;
    GlobalBestObject _sanityCheck_gb;
    GlobalBestObject _customNet;
    std::shared_ptr<TrainingParameters> _params;
    ConfusionMatrix _testConfusionMatrix;
    ConfusionMatrix _validationConfusionMatrix;
    ConfusionMatrix _selectedConfusionMatrix;
    std::shared_ptr<InputCache> _inputCache;

    std::vector<size_t> _inputSkips;
    size_t _numClasses = 0;
    bool _stopValidation=false;

    std::string _functionMsg;

    bool validateOutput(
            const std::vector<real> & outputs,
            const std::vector<real> &expectedResult,
            std::vector<real> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);
    bool networkPathValidation();

};

#endif // OUTAGETRAINER_H
