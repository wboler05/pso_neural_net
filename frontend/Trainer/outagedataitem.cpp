#include "outagedataitem.h"

OutageDataItem::OutageDataItem()
{

}

std::vector<real> OutageDataItem::inputize() {
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

    return input;
}

std::vector<real> OutageDataItem::outputize() {
    std::vector<real> output;

    output.push_back(bool2Double(_outage));
    output.push_back((real) _affectedCustomers);

    return output;
}

real OutageDataItem::bool2Double(const bool &b) {
    if (b) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}

bool OutageDataItem::double2Bool(const real &d) {
    return d < 0.5f;
}

real OutageDataItem::cityToNumber(const std::string &c) {
    size_t val = 0;
    for (size_t i = 0; i < c.size(); i++) {
        val += c[i];
    }
    return (real) val;
}

real OutageDataItem::countyToNumber(const std::string &c) {
    size_t val = 0;
    for (size_t i = 0; i < c.size(); i++) {
        val += c[i];
    }
    return (real) val;
}

real OutageDataItem::reportedEventToNumber(const std::string &e) {
    size_t val = 0;
    for (size_t i = 0; i < e.size(); i++) {
        val += e[i];
    }
    return (real) val;
}

real OutageDataItem::stormTypeToNumber(const std::string &s) {
    size_t val = 0;
    for (size_t i = 0; i < s.size(); i++) {
        val += s[i];
    }
    return (real) val;
}
