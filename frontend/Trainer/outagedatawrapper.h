#ifndef OUTAGEDATAWRAPPER_H
#define OUTAGEDATAWRAPPER_H

#include "outagedataitem.h"

class OutageDataWrapper : public OutageDataItem
{
public:
    OutageDataWrapper(OutageDataItem && r);
    OutageDataWrapper(OutageDataWrapper && r);
    OutageDataWrapper & operator = (OutageDataItem && r);
    OutageDataWrapper & operator = (OutageDataWrapper && r);

    std::vector<real> inputize();
    std::vector<real> outputize();

    static real bool2Double(const bool & b);
    static bool double2Bool(const real & d);

    static real cityToNumber(const std::string & c);
    static real countyToNumber(const std::string & c);
    static real reportedEventToNumber(const std::string & e);
    static real stormTypeToNumber(const std::string & s);

    static std::string numberToCity(const int & c);
    static std::string numberToCounty(const int & c);
    static std::string numberToReportedEvent(const int & c);

    static OutageDataItem copy(const OutageDataItem & l);

private:
    OutageDataWrapper() = delete;
};

#endif // OUTAGEDATAWRAPPER_H
