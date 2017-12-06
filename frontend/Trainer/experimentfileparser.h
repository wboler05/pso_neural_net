#ifndef EXPERIMENTFILEPARSER_H
#define EXPERIMENTFILEPARSER_H

#include <QMainWindow>
#include <QFileDialog>
#include <frontend/Trainer/OutageTrainer.h>

#include <memory>
#include <cctype>

#include "neuralpsostream.h"

struct ExperimentParams {

    int trials_per_experiment = 1;

    bool autoTrain = false;
    bool test_single_inputs = false;
    bool test_combination_inputs = false;
    bool test_activation_functions = false;
};

class ExperimentFileParser
{
public:
    ExperimentFileParser(QMainWindow * parent);

    bool readFile(const std::shared_ptr<TrainingParameters> & defaultParams);

    std::vector<TrainingParameters> & getParamsList() { return _paramsList; }
    const ExperimentParams & experimentParams() { return _experimentParams; }
    const QStringList & getExpList(){return _expList;}

    bool built() { return _builtFlag; }

private:
    QMainWindow * _parent = nullptr;
    ExperimentParams _experimentParams;
    TrainingParameters _defaultParams;
    QStringList _expList;
    std::vector<TrainingParameters> _paramsList;
    QString _fileLocation;
    bool _builtFlag = false;

    bool readOptions(const std::string & fullFile);
    bool readAutoTest(const std::string & fullFile);
    bool readExperiments(const std::string & fullFile);
    std::vector<TrainingParameters> parseExperimentLine(const QString & line, bool & valid);

    NeuralNet::Activation parseActivationFunction(const QString & s);
};

#endif // EXPERIMENTFILEPARSER_H
