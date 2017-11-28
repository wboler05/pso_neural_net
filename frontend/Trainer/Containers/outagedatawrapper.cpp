#include "outagedatawrapper.h"

std::vector<size_t> OutageDataWrapper::_inputSkips;
bool OutageDataWrapper::_inputSkipsModified = true;
std::vector<real> OutageDataWrapper::_outputRanges = { 1 };

OutageDataWrapper::OutageDataWrapper() :
    OutageDataItem(),
    _empty(true)
{

}

OutageDataWrapper::OutageDataWrapper(OutageDataItem && r) :
    OutageDataItem(r)
{
}

OutageDataWrapper::OutageDataWrapper(const OutageDataItem &r) :
    OutageDataItem(r)
{
}

OutageDataWrapper::OutageDataWrapper(const OutageDataWrapper &r) :
    OutageDataItem(dynamic_cast<const OutageDataItem &>(r))
{
}

OutageDataWrapper::OutageDataWrapper(OutageDataWrapper && r) {
    _loa = std::move(r._loa);
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
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _population = std::move(r._population);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);
    _sourceLine = std::move(r._sourceLine);
}

OutageDataWrapper & OutageDataWrapper::operator = (OutageDataItem && r) {
    _loa = std::move(r._loa);
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
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _population = std::move(r._population);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);
    _sourceLine = std::move(r._sourceLine);

    return *this;
}

OutageDataWrapper & OutageDataWrapper::operator = (OutageDataWrapper && r) {
    _loa = std::move(r._loa);
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
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _population = std::move(r._population);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);
    _sourceLine = std::move(r._sourceLine);

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
    newItem._date.year(static_cast<int>(lineList[0].toDouble(&ok)));
    newItem._date.month(static_cast<int>(lineList[1].toDouble(&ok)));
    newItem._date.day(static_cast<int>(lineList[2].toDouble(&ok)));

    newItem._temp.hi(static_cast<real>(lineList[3].toDouble(&ok)));
    newItem._temp.avg(static_cast<real>(lineList[4].toDouble(&ok)));
    newItem._temp.lo(static_cast<real>(lineList[5].toDouble(&ok)));

    newItem._dew.hi(static_cast<real>(lineList[6].toDouble(&ok)));
    newItem._dew.avg(static_cast<real>(lineList[7].toDouble(&ok)));
    newItem._dew.lo(static_cast<real>(lineList[8].toDouble(&ok)));

    newItem._humidity.hi(static_cast<real>(lineList[9].toDouble(&ok)));
    newItem._humidity.avg(static_cast<real>(lineList[10].toDouble(&ok)));
    newItem._humidity.lo(static_cast<real>(lineList[11].toDouble(&ok)));

    newItem._pressure.hi(static_cast<real>(lineList[12].toDouble(&ok)));
    newItem._pressure.avg(static_cast<real>(lineList[13].toDouble(&ok)));
    newItem._pressure.lo(static_cast<real>(lineList[14].toDouble(&ok)));

    newItem._visibility.hi(static_cast<real>(lineList[15].toDouble(&ok)));
    newItem._visibility.avg(static_cast<real>(lineList[16].toDouble(&ok)));
    newItem._visibility.lo(static_cast<real>(lineList[17].toDouble(&ok)));

    newItem._wind.hi(static_cast<real>(lineList[18].toDouble(&ok)));
    newItem._wind.avg(static_cast<real>(lineList[19].toDouble(&ok)));
    newItem._wind.gust(static_cast<real>(lineList[20].toDouble(&ok)));

    newItem._precipitation = static_cast<real>(lineList[21].toDouble(&ok));

    newItem._fog = static_cast<int>(lineList[22].toDouble(&ok)) == 1 ? true : false;
    newItem._rain = static_cast<int>(lineList[23].toDouble(&ok)) == 1 ? true : false;
    newItem._snow = static_cast<int>(lineList[24].toDouble(&ok)) == 1 ? true : false;
    newItem._thunderstorm = static_cast<int>(lineList[25].toDouble(&ok)) == 1 ? true : false;

    newItem._loa = static_cast<int>(lineList[26].toDouble(&ok));

    newItem._latlong.latitude(static_cast<real>(lineList[27].toDouble(&ok)));
    newItem._latlong.longitude(static_cast<real>(lineList[28].toDouble(&ok)));

    newItem._affectedCustomers = static_cast<int>(lineList[29].toDouble(&ok));
    if (newItem._affectedCustomers > 0) {
        newItem._outage = true;
    } else {
        newItem._outage = false;
    }

    newItem._population = static_cast<unsigned int>(lineList[30].toDouble(&ok));

    return newItem;
}

/**
 * @brief OutageDataWrapper::parseStormEvents
 * @param events
 * @param bIt
 * @param item
 * @todo Obsolete
 */
void OutageDataWrapper::parseStormEvents(
        const QStringList & events, const size_t & bIt, OutageDataItem & item)
{
    QStringList parsedEvents;
    for (int i = static_cast<int>(bIt); i < events.size(); i++) {
        parsedEvents.append(events.at(i));
    }

    item._fog = parsedEvents.contains("fog", Qt::CaseInsensitive);
    item._rain = parsedEvents.contains("rain", Qt::CaseInsensitive);
    item._snow = parsedEvents.contains("snow", Qt::CaseInsensitive);
    item._thunderstorm = parsedEvents.contains("thunderstorm", Qt::CaseInsensitive);
}

/**
 * @brief OutageDataWrapper::inputize
 * @details Returns vector of inputs as reals for ANN training.
 * @param skips
 * @return
 */
std::vector<real> OutageDataWrapper::inputize() {
    //std::vector<real> input;
    std::list<real> inputs_l;

    inputs_l.push_back(_loa);

    // Lat and Long
    inputs_l.push_back(_latlong.latitude());
    inputs_l.push_back(_latlong.longitude());

    // Date
    inputs_l.push_back(static_cast<real>(_date.year()));
    inputs_l.push_back(static_cast<real>(_date.month()));
    inputs_l.push_back(static_cast<real>(_date.day()));

    // Temperature
    inputs_l.push_back(_temp.lo());
    inputs_l.push_back(_temp.avg());
    inputs_l.push_back(_temp.hi());

    // Dew Point
    inputs_l.push_back(_dew.lo());
    inputs_l.push_back(_dew.avg());
    inputs_l.push_back(_dew.hi());

    // Humidity
    inputs_l.push_back(_humidity.lo());
    inputs_l.push_back(_humidity.avg());
    inputs_l.push_back(_humidity.hi());

    // Sea Level Pressure
    inputs_l.push_back(_pressure.lo());
    inputs_l.push_back(_pressure.avg());
    inputs_l.push_back(_pressure.hi());

    // Visibility
    inputs_l.push_back(_visibility.lo());
    inputs_l.push_back(_visibility.avg());
    inputs_l.push_back(_visibility.hi());

    // Wind
    inputs_l.push_back(_wind.gust());
    inputs_l.push_back(_wind.avg());
    inputs_l.push_back(_wind.hi());

    // Precipitation
    inputs_l.push_back(_precipitation);

    // Strings
    //input.push_back(cityToNumber(_city));
    //input.push_back(countyToNumber(_county));
    //input.push_back(reportedEventToNumber(_reported_event));
    //input.push_back(stormTypeToNumber(_storm_event));
    inputs_l.push_back(bool2Double(_fog));
    inputs_l.push_back(bool2Double(_rain));
    inputs_l.push_back(bool2Double(_snow));
    inputs_l.push_back(bool2Double(_thunderstorm));

    inputs_l.push_back(static_cast<real>(_population));

    std::vector<real> inputs_v;
    size_t skipOffset = 0;
    size_t i = 0;
    while (inputs_l.size()) {
        if (_inputSkips.size() > 0 && skipOffset < _inputSkips.size()) {
            if (skipOffset < _inputSkips.size()) {
                if (i != _inputSkips[skipOffset]) {
                    inputs_v.push_back(inputs_l.front());
                } else {
                    skipOffset++;
                }
            }
        } else {
            inputs_v.push_back(inputs_l.front());
        }
        i++;
        inputs_l.pop_front();
    }

    /*
    size_t offset = 0;
    for (size_t i = 0; i < _inputSkips.size(); i++) {
        size_t skipElement = _inputSkips[i] - offset;
        inputs_l.erase(inputs_l.begin() + skipElement);
        ++offset;
    }


    for (size_t i = 0; i < input.size(); i++) {
        inputs_v[i] = input[i];
        input.pop_front();
    }
    */

    _inputSizeSet = true;
    _inputSkipsModified = false;
    _inputSize = inputs_v.size();

    return inputs_v;
}

/**
 * @brief OutageDataWrapper::outputize
 * @details Returns vector of ones-hot classifiers.
 * @return
 */
std::vector<real> OutageDataWrapper::outputize() {
    std::vector<real> output;

    //std::vector<real> ranges = {10, 100, 1000};
    //std::vector<real> ranges = { 1 };
    //std::vector<real> ranges = { 3, 42 };
    // 10 100 1000
    // 1 9 72
    // 3 42

    if (_outputRanges.size() > 1) {
        output.resize(_outputRanges.size()+2, -1);

        for (size_t i = 0; i < _outputRanges.size()+2; i++) {
            if (i == 0) {
                if (_affectedCustomers == 0) {
                    output[0] = 1;
                    break;
                }
            } else if (i == _outputRanges.size() + 1) {
                if (_affectedCustomers >= _outputRanges[i-2]) {
                    output[i] = 1;
                    break;
                }
            } else {
                if (_affectedCustomers <= _outputRanges[i-1]) {
                    output[i] = 1;
                    break;
                }
            }
        }
    } else if (_outputRanges.size() == 1) {
        output.resize(2, -1);
        if (_affectedCustomers < _outputRanges[0]) {
            output[0] = 1.0;
        } else {
            output[1] = 1.0;
        }
    } else {
        qDebug() << "What are you doing?";
        exit(1);
    }

    _outputSizeSet = true;
    _outputSize = output.size();

    return output;
}

real OutageDataWrapper::bool2Double(const bool &b) {
    if (b) {
        return static_cast<real>(1.0);
    } else {
        return static_cast<real>(-1.0);
    }
}

bool OutageDataWrapper::double2Bool(const real &d) {
    if (d < 0) {
        return false;
    } else {
        return true;
    }
}

real OutageDataWrapper::cityToNumber(const std::string &c) {
/*    std::locale loc;
    std::string newC = std::tolower(c.c_str(), loc); //!FIXME Cannot take const char!!

    if (newC == "monticello") {
        return static_cast<real>(100);
    } else if (newC == "valparaiso") {
        return static_cast<real>(50);
    } else if (newC == "crown point") {
        return static_cast<real>(30);
    } else if (newC == "plymouth") {
        return static_cast<real>(70);
    } else if (newC == "goshen") {
        return static_cast<real>(110);
    } else if (newC == "gary") {
        return static_cast<real>(150);
    } else if (newC == "hammond") {
        return static_cast<real>(10);
    } else if (newC == "la porte" || newC == "laporte") {
        return static_cast<real>(160);
    } else if (newC == "angola") {
        return static_cast<real>(90);
    } else {
        return static_cast<real>(-1);
    }
    */
    return 0;
}

real OutageDataWrapper::countyToNumber(const std::string &c) {
/*    std::locale loc;
    std::string newC = std::tolower(c.c_str(), loc);

    if (newC == "la porte" || newC == "laporte") {
        return static_cast<real>(0);
    } else if (newC == "lake") {
        return static_cast<real>(1);
    } else if (newC == "marshall") {
        return static_cast<real>(2);
    } else if (newC == "porter") {
        return static_cast<real>(3);
    } else if (newC == "steuben") {
        return static_cast<real>(4);
    } else if (newC == "white") {
        return static_cast<real>(5);
    } else {
        return static_cast<real>(-1);
    }
    */
    return 0;
}

real OutageDataWrapper::reportedEventToNumber(const std::string &e) {
/*
    std::locale loc;
    std::string newC = std::tolower(e.c_str(), loc);

    if (newC == "no cause") {
        return static_cast<real>(0);
    } else if (newC == "210 - tree fell - naturally") {
        return static_cast<real>(210);
    } else if (newC == "211 - tree fell - storm") {
        return static_cast<real>(211);
    } else if (newC == "230 - tree grow - naturally") {
        return static_cast<real>(230);
    } else if (newC == "291 - weather") {
        return static_cast<real>(291);
    } else if (newC == "320 - snow") {
        return static_cast<real>(320);
    } else if (newC == "340 - tornado") {
        return static_cast<real>(340);
    } else if (newC == "310 - lightning") {
        return static_cast<real>(310);
    } else {
        return static_cast<real>(-1);
    }
*/
    return 0;
}

real OutageDataWrapper::stormTypeToNumber(const std::string &s) {
    size_t val = 0;
    for (size_t i = 0; i < s.size(); i++) {
        val += static_cast<size_t>(s[i]);
    }
    return static_cast<real>(val);
}

OutageDataItem OutageDataWrapper::copy(const OutageDataItem & l) {
    OutageDataItem r;

    r._loa = l._loa;
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
    r._precipitation = l._precipitation;
    r._fog = l._fog;
    r._rain = l._rain;
    r._snow = l._snow;
    r._thunderstorm = l._thunderstorm;
    r._population = l._population;
    r._sourceLine = l._sourceLine;

    r._outage = l._outage;
    r._affectedCustomers = l._affectedCustomers;

    return r;
}

const size_t & OutageDataWrapper::inputSize() {
    if (!_inputSizeSet || _inputSkipsModified) {
        inputize();
    }
    return _inputSize;
}

const size_t & OutageDataWrapper::outputSize() {
    if (!_outputSizeSet) {
        outputize();
    }
    return _outputSize;
}

void OutageDataWrapper::setInputSkips(const std::vector<size_t> & skips) {
    _inputSkips = skips;
    if (_inputSkips.size() > 0) {
        CustomSort::quickSort(_inputSkips);
    }
    _inputSkipsModified = true;
}

void OutageDataWrapper::setOutputRanges(const std::vector<real> & ranges) {
    if (ranges.size() > 0) {
        _outputRanges = ranges;
    } else {
        std::cerr << "Error: cannot set empty ranges for OutputRanges. OutageDataWrapper::setOutputRanges" << std::endl;
    }
}
