#include "windsnip.h"

unsigned long WindSnip::_totalObjects = 0;

WindSnip::WindSnip() :
    _gust(0),
    _avg(0),
    _hi(0)
{
    _totalObjects++;
}

WindSnip::~WindSnip() {
    _totalObjects--;
}

WindSnip::WindSnip(const real & gust, const real & avg, const real & hi) :
    _gust(gust),
    _avg(avg),
    _hi(hi)
{
    _totalObjects++;
}

WindSnip::WindSnip(const WindSnip & l) :
    _gust(l._gust),
    _avg(l._avg),
    _hi(l._hi)
{
    _totalObjects++;
}

WindSnip::WindSnip(WindSnip && r) :
    _gust(r._gust),
    _avg(r._avg),
    _hi(r._hi)
{
    _totalObjects++;
}

WindSnip & WindSnip::operator=(const WindSnip & l) {
    _gust = l._gust;
    _avg = l._avg;
    _hi = l._hi;
    return *this;
}

WindSnip & WindSnip::operator=(WindSnip && r) {
    _gust = r._gust;
    _avg = r._avg;
    _hi = r._hi;
    return *this;
}
