#ifndef CACHESLICE_H
#define CACHESLICE_H

#include "outagedatawrapper.h"

class CacheSlice
{
public:
    CacheSlice();

    std::vector<OutageDataItem> _slice;
    size_t _sliceId;
};

#endif // CACHESLICE_H
