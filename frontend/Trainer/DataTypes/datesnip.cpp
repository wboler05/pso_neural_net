#include "datesnip.h"

unsigned long DateSnip::_totalObjects = 0;

DateSnip::DateSnip() :
    _day(0),
    _month(0),
    _year(0)
{
    _totalObjects++;
}

DateSnip::~DateSnip() {
    _totalObjects--;
}

DateSnip::DateSnip(const int & day, const int & month, const int & year) :
    _day(day),
    _month(month),
    _year(year)
{
    _totalObjects++;
}

DateSnip::DateSnip(const DateSnip & l) :
    _day(l._day),
    _month(l._month),
    _year(l._year)
{
    _totalObjects++;
}

DateSnip::DateSnip(DateSnip && r) :
    _day(r._day),
    _month(r._month),
    _year(r._year)
{
    _totalObjects++;
}

DateSnip & DateSnip::operator=(const DateSnip & l) {
    _day = l._day;
    _month = l._month;
    _year = l._year;
    return *this;
}

DateSnip & DateSnip::operator=(DateSnip && r) {
    _day = r._day;
    _month = r._month;
    _year = r._year;
    return *this;
}
