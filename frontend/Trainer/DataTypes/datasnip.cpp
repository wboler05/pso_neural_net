#include "datasnip.h"

unsigned long DataSnip::_totalObjects = 0;

DataSnip::DataSnip() :
    _lo(0),
    _avg(0),
    _hi(0)
{
    _totalObjects++;
}

DataSnip::~DataSnip() {
    _totalObjects--;
}

DataSnip::DataSnip(const real & lo, const real & avg, const real & hi) :
    _lo(lo),
    _avg(avg),
    _hi(hi)
{
    _totalObjects++;
}

DataSnip::DataSnip(const DataSnip &l) :
    _lo(l._lo),
    _avg(l._avg),
    _hi(l._hi)
{
    _totalObjects++;
}

DataSnip::DataSnip(DataSnip &&r) :
    _lo(r._lo),
    _avg(r._avg),
    _hi(r._hi)
{
    _totalObjects++;
}

DataSnip & DataSnip::operator=(const DataSnip & l) {
    _lo = l._lo;
    _avg = l._avg;
    _hi = l._hi;
    return *this;
}

DataSnip & DataSnip::operator=(DataSnip && r) {
    _lo = r._lo;
    _avg = r._avg;
    _hi = r._hi;
    return *this;
}
