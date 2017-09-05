#ifndef OUTAGEDATAITEM_H
#define OUTAGEDATAITEM_H

#include "custommath.h"

class OutageDataItem
{
public:
    OutageDataItem();

    std::vector<real> inputize();
    std::vector<real> outputize();

    static real bool2Double(const bool & b);
    static bool double2Bool(const real & d);

    static real cityToNumber(const std::string & c);
    static real countyToNumber(const std::string & c);
    static real reportedEventToNumber(const std::string & e);
    static real stormTypeToNumber(const std::string & s);

    // Inputs
    uint _year=0;
    uint _month=0;
    uint _day=0;
    std::string _city=0;
    std::string _county=0;
    std::string _reported_event=0;
    std::string _storm=0;
    real _precipitation=0;
    real _lo_temp=0;
    real _hi_temp=0;

    // Labels
    bool _outage=false;
    int _affectedCustomers=0;

};

#endif // OUTAGEDATAITEM_H
