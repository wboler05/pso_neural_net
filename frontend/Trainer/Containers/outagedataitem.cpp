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

OutageDataItem::OutageDataItem(const OutageDataItem &l) {
    _latlong = l._latlong;
    _date = l._date;
    _temp = l._temp;
    _dew = l._dew;
    _humidity = l._humidity;
    _pressure = l._pressure;
    _visibility = l._visibility;
    _wind = l._wind;
    _precipitation = l._precipitation;
    _city = l._city;
    _county = l._county;
//    _reported_event = l._reported_event;
//    _storm_event = l._storm_event;
    _fog = l._fog;
    _rain = l._rain;
    _snow = l._snow;
    _thunderstorm = l._thunderstorm;
    _outage = l._outage;
    _affectedCustomers = l._affectedCustomers;

    _id = _total_objects++;
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
//    _reported_event = std::move(r._reported_event);
//    _storm_event = std::move(r._storm_event);
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
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
//    _reported_event = std::move(r._reported_event);
//    _storm_event = std::move(r._storm_event);
    _fog = std::move(r._fog);
    _rain = std::move(r._rain);
    _snow = std::move(r._snow);
    _thunderstorm = std::move(r._thunderstorm);
    _outage = std::move(r._outage);
    _affectedCustomers = std::move(r._affectedCustomers);
    _id = _total_objects++;

    return *this;
}

OutageDataItem & OutageDataItem::operator=(const OutageDataItem &l) {
    _latlong = l._latlong;
    _date = l._date;
    _temp = l._temp;
    _dew = l._dew;
    _humidity = l._humidity;
    _pressure = l._pressure;
    _visibility = l._visibility;
    _wind = l._wind;
    _precipitation = l._precipitation;
    _city = l._city;
    _county = l._county;
//    _reported_event = l._reported_event;
//    _storm_event = l._storm_event;
    _fog = l._fog;
    _rain = l._rain;
    _snow = l._snow;
    _thunderstorm = l._thunderstorm;
    _outage = l._outage;
    _affectedCustomers = l._affectedCustomers;

    return *this;
}

