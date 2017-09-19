#include "outagedataitem.h"

size_t OutageDataItem::_total_objects = 0;

OutageDataItem::OutageDataItem() :
    _precipitation(0),
    _outage(false),
    _affectedCustomers(0)
{
    _id = _total_objects++;
}

OutageDataItem::~OutageDataItem() {
    _total_objects--;
}

OutageDataItem::OutageDataItem(OutageDataItem &&r) {
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
    _reported_event = std::move(r._reported_event);
    _storm_event = std::move(r._storm_event);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);

    _id = _total_objects++;
}

OutageDataItem & OutageDataItem::operator= (OutageDataItem && r) {
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
    _reported_event = std::move(r._reported_event);
    _storm_event = std::move(r._storm_event);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);
    _id = _total_objects++;

    return *this;
}

