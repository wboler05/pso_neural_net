#include "outagedatawrapper.h"

OutageDataWrapper::OutageDataWrapper() :
    OutageDataItem(),
    _empty(true)
{

}

OutageDataWrapper::OutageDataWrapper(OutageDataItem && r) :
    OutageDataItem(r)
{
    /*
    _latlong = std::move(r._latlong);
    _date = std::move(r._date);
    _temp = std::move(r._temp);
    _dew = std::move(r._dew);
    _humidity = std::move(r._humidity);
    _pressure = std::move(r._pressure);
    _visibility = std::move(r._visibility);
    _wind = std::move(r._wind);
    _precipitation = std::move(r._precipitation);
    _city = std::move(r._city);
    _county = std::move(r._county);
//    _reported_event = std::move(r._reported_event);
//    _storm_event = std::move(r._storm_event);
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);
    */
}

OutageDataWrapper::OutageDataWrapper(const OutageDataItem &r) :
    OutageDataItem(r)
{
    /*
    _latlong = r._latlong;
    _date = r._date;
    _temp = r._temp;
    _dew = r._dew;
    _humidity = r._humidity;
    _pressure = r._pressure;
    _visibility = r._visibility;
    _wind = r._wind;
    _precipitation = r._precipitation;
    _city = r._city;
    _county = r._county;
    _reported_event = r._reported_event;
    _storm_event = r._storm_event;
    _outage = r._outage;
    _affectedCustomers = r._affectedCustomers;
    */
}

OutageDataWrapper::OutageDataWrapper(OutageDataWrapper && r) {
    _latlong = std::move(r._latlong);
    _date = std::move(r._date);
    _temp = std::move(r._temp);
    _dew = std::move(r._dew);
    _humidity = std::move(r._humidity);
    _pressure = std::move(r._pressure);
    _visibility = std::move(r._visibility);
    _wind = std::move(r._wind);
    _precipitation = std::move(r._precipitation);
    _city = std::move(r._city);
    _county = std::move(r._county);
//    _reported_event = std::move(r._reported_event);
//    _storm_event = std::move(r._storm_event);
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);
}

OutageDataWrapper & OutageDataWrapper::operator = (OutageDataItem && r) {
    _latlong = std::move(r._latlong);
    _date = std::move(r._date);
    _temp = std::move(r._temp);
    _dew = std::move(r._dew);
    _humidity = std::move(r._humidity);
    _pressure = std::move(r._pressure);
    _visibility = std::move(r._visibility);
    _wind = std::move(r._wind);
    _precipitation = std::move(r._precipitation);
    _city = std::move(r._city);
    _county = std::move(r._county);
    //_reported_event = std::move(r._reported_event);
    //_storm_event = std::move(r._storm_event);
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);

    return *this;
}

OutageDataWrapper & OutageDataWrapper::operator = (OutageDataWrapper && r) {
    _latlong = std::move(r._latlong);
    _date = std::move(r._date);
    _temp = std::move(r._temp);
    _dew = std::move(r._dew);
    _humidity = std::move(r._humidity);
    _pressure = std::move(r._pressure);
    _visibility = std::move(r._visibility);
    _wind = std::move(r._wind);
    _precipitation = std::move(r._precipitation);
    _city = std::move(r._city);
    _county = std::move(r._county);
    //_reported_event = std::move(r._reported_event);
    //_storm_event = std::move(r._storm_event);
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);

    return *this;
}

OutageDataItem OutageDataWrapper::parseInputString(const QString & line) {

    // TODO: Return a pointer to a new object???
    OutageDataItem newItem;

    QStringList lineList = line.split(",");
    if (lineList.length() < 22) {
        return newItem;
    }
    bool ok;
    newItem._date.year(lineList[0].toInt(&ok));
    newItem._date.month(lineList[1].toInt(&ok));
    newItem._date.day(lineList[2].toInt(&ok));

    newItem._temp.hi(lineList[3].toDouble(&ok));
    newItem._temp.avg(lineList[4].toDouble(&ok));
    newItem._temp.lo(lineList[5].toDouble(&ok));

    newItem._dew.hi(lineList[6].toDouble(&ok));
    newItem._dew.avg(lineList[7].toDouble(&ok));
    newItem._dew.lo(lineList[8].toDouble(&ok));

    newItem._humidity.hi(lineList[9].toDouble(&ok));
    newItem._humidity.avg(lineList[10].toDouble(&ok));
    newItem._humidity.lo(lineList[11].toDouble(&ok));

    newItem._pressure.hi(lineList[12].toDouble(&ok));
    newItem._pressure.avg(lineList[13].toDouble(&ok));
    newItem._pressure.lo(lineList[14].toDouble(&ok));

    newItem._visibility.hi(lineList[15].toDouble(&ok));
    newItem._visibility.avg(lineList[16].toDouble(&ok));
    newItem._visibility.lo(lineList[17].toDouble(&ok));

    newItem._wind.hi(lineList[18].toDouble(&ok));
    newItem._wind.avg(lineList[19].toDouble(&ok));
    newItem._wind.gust(lineList[20].toDouble(&ok));

    newItem._precipitation = lineList[21].toDouble(&ok);

//    newItem._storm_event = ((QString)lineList[22]).toStdString();
    parseStormEvents(lineList, 22, newItem);

    return newItem;
}

void OutageDataWrapper::parseStormEvents(
        const QStringList & events, const size_t & bIt, OutageDataItem & item)
{
    QStringList parsedEvents;
    for (size_t i = bIt; i < events.size(); i++) {
        parsedEvents.append(events.at(i));
    }

    item._fog = parsedEvents.contains("fog", Qt::CaseInsensitive);
    item._rain = parsedEvents.contains("rain", Qt::CaseInsensitive);
    item._snow = parsedEvents.contains("snow", Qt::CaseInsensitive);
    item._thunderstorm = parsedEvents.contains("thunderstorm", Qt::CaseInsensitive);
}

std::vector<real> OutageDataWrapper::inputize() {
    std::vector<real> input;

    // Lat and Long
    input.push_back(_latlong.latitude());
    input.push_back(_latlong.longitude());

    // Date
    input.push_back((real) _date.year());
    input.push_back((real) _date.month());
    input.push_back((real) _date.day());

    // Temperature
    input.push_back(_temp.lo());
    input.push_back(_temp.avg());
    input.push_back(_temp.hi());

    // Dew Point
    input.push_back(_dew.lo());
    input.push_back(_dew.avg());
    input.push_back(_dew.hi());

    // Humidity
    input.push_back(_humidity.lo());
    input.push_back(_humidity.avg());
    input.push_back(_humidity.hi());

    // Sea Level Pressure
    input.push_back(_pressure.lo());
    input.push_back(_pressure.avg());
    input.push_back(_pressure.hi());

    // Visibility
    input.push_back(_visibility.lo());
    input.push_back(_visibility.avg());
    input.push_back(_visibility.hi());

    // Wind
    input.push_back(_wind.gust());
    input.push_back(_wind.avg());
    input.push_back(_wind.hi());

    // Precipitation
    input.push_back(_precipitation);

    // Strings
    //input.push_back(cityToNumber(_city));
    //input.push_back(countyToNumber(_county));
    //input.push_back(reportedEventToNumber(_reported_event));
    //input.push_back(stormTypeToNumber(_storm_event));
    input.push_back(bool2Double(_fog));
    input.push_back(bool2Double(_rain));
    input.push_back(bool2Double(_snow));
    input.push_back(bool2Double(_thunderstorm));

    return input;
}

std::vector<real> OutageDataWrapper::outputize() {
    std::vector<real> output;

    output.push_back(bool2Double(_outage));
    output.push_back((real) _affectedCustomers);

    return output;
}

/**
 * @brief OutageDataWrapper::MSE
 * @details Find the MSE for a set of experiments
 * @param results
 * @param expecteds
 * @return
 */
real OutageDataWrapper::MSE(
        const std::vector<std::vector<real> > &results,
        const std::vector<std::vector<real> > &expecteds) {

    // Verify that their sizes match
    if (results.size() != expecteds.size()) {
        return std::numeric_limits<real>::infinity();
    }

    // Verify that they are both not empty
    if (results.size() == 0) {
        return std::numeric_limits<real>::infinity();
    }

    // Get us our error variable set to zero
    real error = 0;

    // Loop through each event
    //#pragma omp parallel for
    for (size_t i = 0; i < results.size(); i++) {

        // Get the error for each event seperately
        real tError = MSE(results[i], expecteds[i]);

        // If inifinity, return infinit (we broke it)
        if (tError == std::numeric_limits<real>::infinity()) {
            return std::numeric_limits<real>::infinity();
        }

        // Else add the error to the summation
        error += tError;
    }
    // Divide by the vector size
    error /= results.size();

    // Return our error
    return error;
}


/**
 * @brief OutageDataWrapper::MSE
 * @details Calculate the MSE for a single event
 * @param result
 * @param expected
 * @return
 */
real OutageDataWrapper::MSE(const std::vector<real> &result, const std::vector<real> &expected) {
    // Verify that their sizes match
    if (result.size() != expected.size()) {
        return std::numeric_limits<real>::infinity();
    }

    // Verify that they are both not empty
    if (result.size() == 0) {
        return std::numeric_limits<real>::infinity();
    }

    // Get us our error variable set to zero
    real error = 0;

    // Sum the squared errors
    // #pragma omp parallel for
    for (size_t i = 0; i < result.size(); i++) {
        error += CustomMath::pow((result[i] - expected[i]), 2);
    }
    // Divide by total error size
    error /= result.size();

    // Return our error
    return error;
}

real OutageDataWrapper::bool2Double(const bool &b) {
    if (b) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}

bool OutageDataWrapper::double2Bool(const real &d) {
    return d < 0.5f;
}

real OutageDataWrapper::cityToNumber(const std::string &c) {
    std::locale loc;
    std::string newC = std::tolower(c.c_str(), loc); //!FIXME Cannot take const char!!

    if (newC == "monticello") {
        return (real) 100;
    } else if (newC == "valparaiso") {
        return (real) 50;
    } else if (newC == "crown point") {
        return (real) 30;
    } else if (newC == "plymouth") {
        return (real) 70;
    } else if (newC == "goshen") {
        return (real) 110;
    } else if (newC == "gary") {
        return (real) 150;
    } else if (newC == "hammond") {
        return (real) 10;
    } else if (newC == "la porte" || newC == "laporte") {
        return (real) 160;
    } else if (newC == "angola") {
        return (real) 90;
    } else {
        return (real) -1;
    }
}

real OutageDataWrapper::countyToNumber(const std::string &c) {
    std::locale loc;
    std::string newC = std::tolower(c.c_str(), loc);

    if (newC == "la porte" || newC == "laporte") {
        return (real) 0;
    } else if (newC == "lake") {
        return (real) 1;
    } else if (newC == "marshall") {
        return (real) 2;
    } else if (newC == "porter") {
        return (real) 3;
    } else if (newC == "steuben") {
        return (real) 4;
    } else if (newC == "white") {
        return (real) 5;
    } else {
        return (real) -1;
    }
}

real OutageDataWrapper::reportedEventToNumber(const std::string &e) {

    std::locale loc;
    std::string newC = std::tolower(e.c_str(), loc);

    if (newC == "no cause") {
        return (real) 0;
    } else if (newC == "210 - tree fell - naturally") {
        return (real) 210;
    } else if (newC == "211 - tree fell - storm") {
        return (real) 211;
    } else if (newC == "230 - tree grow - naturally") {
        return (real) 230;
    } else if (newC == "291 - weather") {
        return (real) 291;
    } else if (newC == "320 - snow") {
        return (real) 320;
    } else if (newC == "340 - tornado") {
        return (real) 340;
    } else if (newC == "310 - lightning") {
        return (real) 310;
    } else {
        return (real) -1;
    }

}

real OutageDataWrapper::stormTypeToNumber(const std::string &s) {
    size_t val = 0;
    for (size_t i = 0; i < s.size(); i++) {
        val += s[i];
    }
    return (real) val;
}

OutageDataItem OutageDataWrapper::copy(const OutageDataItem & l) {
    OutageDataItem r;

    r._latlong = l._latlong;
    r._date = l._date;
    r._temp = l._temp;
    r._dew = l._dew;
    r._humidity = l._humidity;
    r._pressure = l._pressure;
    r._visibility = l._visibility;
    r._wind = l._wind;
    r._precipitation = l._precipitation;
    r._city = l._city;
    r._county = l._county;
//    r._reported_event = l._reported_event;
//    r._storm_event = l._storm_event;
    r._precipitation = l._precipitation;
    r._fog = l._fog;
    r._rain = l._rain;
    r._snow = l._snow;
    r._thunderstorm = l._thunderstorm;

    r._outage = l._outage;
    r._affectedCustomers = l._affectedCustomers;

    return r;
}
