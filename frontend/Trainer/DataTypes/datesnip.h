#ifndef DATESNIP_H
#define DATESNIP_H

#include "custommath.h"

class DateSnip
{
public:
    DateSnip();
    ~DateSnip();

    DateSnip(const int & day, const int & month, const int & year);
    DateSnip(const DateSnip & l);
    DateSnip(DateSnip && r);

    DateSnip & operator=(const DateSnip & l);
    DateSnip & operator=(DateSnip && r);

    const int & day(void) { return _day; }
    const int & month(void) { return _month; }
    const int & year(void) { return _year; }

    void day(const int & day) { _day = day; }
    void month(const int & month) { _month = month; }
    void year(const int & year) { _year = year; }

    static const unsigned long & totalObjects() { return _totalObjects; }

private:
    int _day;
    int _month;
    int _year;

    static unsigned long _totalObjects;
};

#endif // DATESNIP_H
