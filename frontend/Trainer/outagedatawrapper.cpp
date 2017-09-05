#include "outagedatawrapper.h"

OutageDataWrapper::OutageDataWrapper(OutageDataItem && r) :
    OutageDataItem()
{
    _year = r._year;
    _month = r._month;
    _day = r._day;
    _city = r._city;
    _county = r._county;
    _reported_event = r._reported_event;
    _storm = r._storm;
    _precipitation = r._precipitation;
    _lo_temp = r._lo_temp;
    _hi_temp = r._hi_temp;
    _latitude = r._latitude;
    _longitude = r._longitude;
    _outage = r._outage;
    _affectedCustomers = r._affectedCustomers;
}

OutageDataWrapper::OutageDataWrapper(OutageDataWrapper && r) {
    _year = r._year;
    _month = r._month;
    _day = r._day;
    _city = r._city;
    _county = r._county;
    _reported_event = r._reported_event;
    _storm = r._storm;
    _precipitation = r._precipitation;
    _lo_temp = r._lo_temp;
    _hi_temp = r._hi_temp;
    _latitude = r._latitude;
    _longitude = r._longitude;
    _outage = r._outage;
    _affectedCustomers = r._affectedCustomers;
}

OutageDataWrapper & OutageDataWrapper::operator = (OutageDataItem && r) {
    _year = r._year;
    _month = r._month;
    _day = r._day;
    _city = r._city;
    _county = r._county;
    _reported_event = r._reported_event;
    _storm = r._storm;
    _precipitation = r._precipitation;
    _lo_temp = r._lo_temp;
    _hi_temp = r._hi_temp;
    _latitude = r._latitude;
    _longitude = r._longitude;
    _outage = r._outage;
    _affectedCustomers = r._affectedCustomers;

    return *this;
}

OutageDataWrapper & OutageDataWrapper::operator = (OutageDataWrapper && r) {
    _year = r._year;
    _month = r._month;
    _day = r._day;
    _city = r._city;
    _county = r._county;
    _reported_event = r._reported_event;
    _storm = r._storm;
    _precipitation = r._precipitation;
    _lo_temp = r._lo_temp;
    _hi_temp = r._hi_temp;
    _latitude = r._latitude;
    _longitude = r._longitude;
    _outage = r._outage;
    _affectedCustomers = r._affectedCustomers;

    return *this;
}

std::vector<real> OutageDataWrapper::inputize() {
    std::vector<real> input;

    input.push_back((real) _year);
    input.push_back((real) _month);
    input.push_back((real) _day);
    input.push_back(cityToNumber(_city));
    input.push_back(countyToNumber(_county));
    input.push_back(reportedEventToNumber(_reported_event));
    input.push_back(stormTypeToNumber(_storm));
    input.push_back(_precipitation);
    input.push_back(_lo_temp);
    input.push_back(_hi_temp);
    input.push_back(_latitude);
    input.push_back(_longitude);

    return input;
}

std::vector<real> OutageDataWrapper::outputize() {
    std::vector<real> output;

    output.push_back(bool2Double(_outage));
    output.push_back((real) _affectedCustomers);

    return output;
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

    r._year = l._year;
    r._month = l._month;
    r._day = l._day;
    r._city = l._city;
    r._county = l._county;
    r._reported_event = l._reported_event;
    r._storm = l._storm;
    r._precipitation = l._precipitation;
    r._lo_temp = l._lo_temp;
    r._hi_temp = l._hi_temp;
    r._latitude = l._latitude;
    r._longitude = l._longitude;
    r._outage = l._outage;
    r._affectedCustomers = l._affectedCustomers;

    return r;
}
