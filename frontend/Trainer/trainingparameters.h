#ifndef TRAININGPARAMETERS_H
#define TRAININGPARAMETERS_H

#include "neuralpso.h"
#include "inputcache.h"

struct EnableParameters {

    EnableParameters & operator= (const EnableParameters & e) {
        year = e.year;
        month = e.month;
        day = e.day;
        temp_high = e.temp_high;
        temp_avg = e.temp_avg;
        temp_low = e.temp_low;
        dew_high = e.dew_high;
        dew_avg = e.dew_avg ;
        dew_low = e.dew_low;
        humidity_high = e.humidity_high;
        humidity_avg = e.humidity_avg;
        humidity_low = e.humidity_low;
        press_high = e.press_high;
        press_avg = e.press_avg;
        press_low = e.press_low;
        visibility_high = e.visibility_high;
        visibility_avg = e.visibility_avg;
        visibility_low = e.visibility_low;
        wind_high = e.wind_high;
        wind_avg = e.wind_avg;
        wind_gust = e.wind_gust;
        precipitation = e.precipitation;
        fog = e.fog;
        rain = e.rain;
        snow = e.snow;
        thunderstorm = e.thunderstorm;
        loa = e.loa;
        latitude = e.latitude;
        longitude = e.longitude;
        population = e.population;

        return *this;
    }

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

    const size_t totalElements = 30;

    std::vector<size_t> inputSkips();
    void setSkipsFromVector(const std::vector<size_t> & skips);
    void setElementEnabled(const size_t & index, bool b);
    bool elementLookup(const size_t & index);

};

struct TrainingParameters {
    TrainingParameters() {}

    TrainingParameters(const TrainingParameters & t) :
        pp(t.pp),
        np(t.np),
        fp(t.fp),
        cp(t.cp),
        ep(t.ep),
        alpha(t.alpha),
        beta(t.beta),
        gamma(t.gamma),
        showBestSelected(t.showBestSelected),
        enableBaseCase(t.enableBaseCase),
        kFolds(t.kFolds),
        inputHistorySize(t.inputHistorySize)
    {

    }

    TrainingParameters & operator=(const TrainingParameters & t) {
        pp = (t.pp);
        np = (t.np);
        fp = (t.fp);
        cp = (t.cp);
        ep = (t.ep);
        alpha = (t.alpha);
        beta = (t.beta);
        gamma = (t.gamma);
        showBestSelected = (t.showBestSelected);
        enableBaseCase = (t.enableBaseCase);
        kFolds = (t.kFolds);
        inputHistorySize = (t.inputHistorySize);
        return *this;
    }

    enum ShowBestSelected { Recent_Global_Best, Selected_Global_Best, Sanity_Check_Best, Current_Net };
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
    size_t inputHistorySize = 1;
};


#endif // TRAININGPARAMETERS_H
