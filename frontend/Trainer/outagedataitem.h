#ifndef OUTAGEDATAITEM_H
#define OUTAGEDATAITEM_H

#include "custommath.h"
#include <vector>
#include <string>
#include <locale>

class OutageDataItem
{
public:
    OutageDataItem();
    ~OutageDataItem();
    OutageDataItem(OutageDataItem && r);
    OutageDataItem & operator=(OutageDataItem && r);

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
    real _latitude=0;
    real _longitude=0;

    // Labels
    bool _outage=false;
    int _affectedCustomers=0;

    const size_t & id() { return _id; }
    static const size_t & totalObjects() { _total_objects; }

private:
    size_t _id;
    static size_t _total_objects;

    OutageDataItem(const OutageDataItem &l) = delete;
    OutageDataItem & operator=(const OutageDataItem &l) = delete;

};

#endif // OUTAGEDATAITEM_H
