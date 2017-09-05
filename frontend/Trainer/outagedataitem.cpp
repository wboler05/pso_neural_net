#include "outagedataitem.h"

OutageDataItem::OutageDataItem()
{
    _id = _total_objects++;
}

OutageDataItem::~OutageDataItem() {
    _total_objects--;
}

OutageDataItem::OutageDataItem(OutageDataItem &&r) {
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
    _id = _total_objects++;
}

OutageDataItem & OutageDataItem::operator= (OutageDataItem && r) {
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
    _id = _total_objects++;

    return *this;
}

