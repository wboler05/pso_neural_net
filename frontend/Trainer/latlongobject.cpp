#include "latlongobject.h"

LatLongObject::LatLongObject() :
    _latitude(0),
    _longitude(0)
{

}

LatLongObject::LatLongObject(const real &lat, const real &longi) :
    _latitude(lat),
    _longitude(longi)
{

}

LatLongObject::LatLongObject(const LatLongObject &l) :
    _latitude(l._latitude),
    _longitude(l._longitude)
{

}

LatLongObject::LatLongObject(LatLongObject &&r) :
    _latitude(r._latitude),
    _longitude(r._longitude)
{

}

LatLongObject & LatLongObject::operator= (const LatLongObject &l) {
    _latitude = l._latitude;
    _longitude = l._longitude;
    return *this;
}

LatLongObject & LatLongObject::operator= (LatLongObject &&r) {
    _latitude = r._latitude;
    _longitude = r._longitude;
    return *this;
}
