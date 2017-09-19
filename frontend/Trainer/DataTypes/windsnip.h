#ifndef WINDSNIP_H
#define WINDSNIP_H

#include "custommath.h"

class WindSnip
{
public:
    WindSnip();
    ~WindSnip();
    WindSnip(const real & gust, const real & avg, const real & hi);

    WindSnip(const WindSnip & l);
    WindSnip(WindSnip && r);

    WindSnip & operator=(const WindSnip & l);
    WindSnip & operator=(WindSnip && r);

    const real & hi() { return _hi; }
    const real & avg() { return _avg; }
    const real & gust() { return _gust; }

    void hi(const real & h) { _hi = h; }
    void avg(const real & a) { _avg = a; }
    void gust(const real & g) { _gust = g; }

    static const unsigned long & totalObjects() { return _totalObjects; }

private:
    real _hi;
    real _avg;
    real _gust;

    static unsigned long _totalObjects;
};

#endif // WINDSNIP_H
