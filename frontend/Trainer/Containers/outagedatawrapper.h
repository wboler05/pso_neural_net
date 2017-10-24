#ifndef OUTAGEDATAWRAPPER_H
#define OUTAGEDATAWRAPPER_H

#include "outagedataitem.h"

class OutageDataWrapper : public OutageDataItem
{
public:
    OutageDataWrapper();
    OutageDataWrapper(OutageDataItem && r);
    OutageDataWrapper(const OutageDataItem & l);
    OutageDataWrapper(OutageDataWrapper && r);
    OutageDataWrapper & operator = (OutageDataItem && r);
    OutageDataWrapper & operator = (OutageDataWrapper && r);

    static OutageDataItem parseInputString(const QString & line);
    static void parseStormEvents(const QStringList & events,
                                 const size_t & bIt, OutageDataItem & item);

    std::vector<real> inputize(const std::vector<size_t> &skips);
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

    static real MSE(const std::vector<std::vector<real>> &results,
                    const std::vector<std::vector<real>> & expecteds);
    static real MSE(const std::vector<real> &result,
                    const std::vector<real> &expected);
    static std::vector<real> splitMSE(const std::vector<real> &result,
                    const std::vector<real> &expected);

    bool empty() { return _empty; }

private:
    bool _empty=false;

};

#endif // OUTAGEDATAWRAPPER_H
