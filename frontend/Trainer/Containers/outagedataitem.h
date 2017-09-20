#ifndef OUTAGEDATAITEM_H
#define OUTAGEDATAITEM_H

#include "custommath.h"
#include <vector>
#include <string>
#include <locale>

#include "datasnip.h"
#include "datesnip.h"
#include "latlongobject.h"
#include "windsnip.h"

class OutageDataItem
{
public:
    OutageDataItem();
    ~OutageDataItem();
    OutageDataItem(OutageDataItem && r);
    OutageDataItem & operator=(OutageDataItem && r);
    OutageDataItem(const OutageDataItem &l);
    OutageDataItem & operator=(const OutageDataItem &l);

    // Inputs
    LatLongObject _latlong;
    DateSnip _date;
    DataSnip _temp;
    DataSnip _dew;
    DataSnip _humidity;
    DataSnip _pressure;
    DataSnip _visibility;
    WindSnip _wind;
    real _precipitation;
    std::string _city;
    std::string _county;
    std::string _reported_event;
    std::string _storm_event;

    // Labels
    bool _outage=false;
    int _affectedCustomers=0;

    size_t byteSize() { return sizeof(*this); }

    const size_t & id() { return _id; }
    static const size_t & totalObjects() { _total_objects; }

private:
    size_t _id;
    static size_t _total_objects;

};

#endif // OUTAGEDATAITEM_H
