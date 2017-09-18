#include "windsnip.h"

WindSnip::WindSnip() :
    _gust(0),
    _avg(0),
    _hi(0)
{

}

WindSnip::WindSnip(const real & gust, const real & avg, const real & hi) :
    _gust(gust),
    _avg(avg),
    _hi(hi)
{

}


