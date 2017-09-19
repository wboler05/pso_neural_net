#ifndef LATLONGOBJECT_H
#define LATLONGOBJECT_H

#include "custommath.h"

class LatLongObject
{
public:
    LatLongObject();
    ~LatLongObject();
    LatLongObject(const real & lat, const real &longi);

    LatLongObject(const LatLongObject &);
    LatLongObject(LatLongObject &&);

    LatLongObject & operator= (const LatLongObject &);
    LatLongObject & operator= (LatLongObject &&);

    const real & latitude() { return _latitude; }
    const real & longitude() { return _longitude; }

    void latitude(const real & l) { _latitude = l; }
    void longitude(const real & l) { _longitude = l; }

    static const unsigned long & totalObjects() { return _totalObjects; }

private:
    real _latitude;
    real _longitude;

    static unsigned long _totalObjects;
};

#endif // LATLONGOBJECT_H
