#ifndef OUTAGEDATAWRAPPER_H
#define OUTAGEDATAWRAPPER_H

#include "outagedataitem.h"
#include "utils/sort.hpp"

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

    bool empty() { return _empty; }

    const size_t & inputSize();
    const size_t & outputSize();

    static const std::vector<size_t> inputSkips() { return _inputSkips; }
    static void setInputSkips(const std::vector<size_t> & skips);

private:
    bool _empty=false;
    bool _inputSizeSet=false;
    bool _outputSizeSet=false;
    size_t _inputSize = 0;
    size_t _outputSize = 0;

    static std::vector<size_t> _inputSkips;
    static bool _inputSkipsModified;

};

#endif // OUTAGEDATAWRAPPER_H
