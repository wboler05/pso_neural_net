#ifndef TRAININGPARAMETERS_H
#define TRAININGPARAMETERS_H

#include "neuralpso.h"
#include "inputcache.h"

struct EnableParameters {
    bool year=false;
    bool month=true;
    bool day=false;
    bool temp_high=false;
    bool temp_avg=true;
    bool temp_low=false;
    bool dew_high=false;
    bool dew_avg=true;
    bool dew_low=false;
    bool humidity_high=true;
    bool humidity_avg=false;
    bool humidity_low=false;
    bool press_high=false;
    bool press_avg=false;
    bool press_low=false;
    bool visibility_high=false;
    bool visibility_avg=false;
    bool visibility_low=true;
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
    bool population = true;

    std::vector<size_t> inputSkips();
};

struct TrainingParameters {
    enum ShowBestSelected { Recent_Global_Best, Selected_Global_Best, Sanity_Check_Best };
    PsoParams pp;
    NeuralNet::NeuralNetParameters np;
    FitnessParameters fp;
    CacheParameters cp;
    EnableParameters ep;
    real alpha = 1.0L;
    real beta = 1.0L;
    real gamma = 1.0L;
    ShowBestSelected showBestSelected = Recent_Global_Best;
    bool enableBaseCase = false;
    size_t kFolds = 10;
};


#endif // TRAININGPARAMETERS_H
