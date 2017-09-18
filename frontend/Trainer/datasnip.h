#ifndef DATASNIP_H
#define DATASNIP_H

#include "custommath.h"

class DataSnip
{
public:
    DataSnip();
    DataSnip(const real & lo, const real & avg, const real & hi);
    ~DataSnip();

    DataSnip(const DataSnip & l);
    DataSnip(DataSnip && r);

    DataSnip & operator=(const DataSnip & l);
    DataSnip & operator=(DataSnip && r);

    const real & lo() { return _lo; }
    const real & avg() { return _avg; }
    const real & hi() { return _hi; }

    void lo(const real & l) { _lo = l; }
    void avg(const real & a) { _avg = a; }
    void hi(const real & h) { _hi = h; }

private:
    real _lo;
    real _avg;
    real _hi;
};

#endif // DATASNIP_H
