#ifndef PETRAINER_H
#define PETRAINER_H

#include "neuralpso.h"

struct PEParameters {
    PsoParams pp;
    NeuralNetParameters np;
    FitnessParameters fp;
};

class PETrainer : public NeuralPso
{
public:
    PETrainer(const PEParameters & pe);

    void build(std::vector<std::vector<std::vector<byte> > > &images, std::vector<byte> &labels);
    void build(std::vector<std::vector<double>> &input, std::vector<double> &output);

    double testRun(double &correctRatio, uint &totalCount, double &confidence);
    void testGB();

    int randomizeTestInputs();
    void runTrainer();
    void loadTestInput(uint32_t I);

    void classError(TestStatistics::ClassificationError * ce);
    TestStatistics & testStats() { return _testStats; }

    static bool convertOutput(const double & output);
    static double convertInput(const bool & input);

    void setFunctionMsg(std::string s) { _functionMsg = s; }
    std::string functionMsg() { return _functionMsg; }

private:
    std::vector<std::vector<std::vector<byte> > > *_images;
    std::vector<byte> *_labels;
    TestStatistics _testStats;
    std::vector<std::vector<double> > *_input;
    std::vector<double> *_output;
    std::vector<double> *_outputCount;
    std::vector<std::vector<uint>> _outputIterators;

    std::string _functionMsg;

    bool validateOutput(const std::vector<double> &outputs,
            const std::vector<double> &expectedOutputs,
            std::vector<double> & outputError,
            TestStatistics & testStats,
            bool & correctOutput);

};

#endif // PETRAINER_H
