#ifndef INPUTCACHE_H
#define INPUTCACHE_H

#include "cacheslice.h"
#include "outagedatawrapper.h"

#include <QFile>

struct CacheParameters {
    QString inputFileName;
    unsigned long maxBytes=0;
    size_t totalSlicesPerCache=0;
    size_t headerSize=0;
};

class InputCache
{
public:
    explicit InputCache(const CacheParameters & c);
    ~InputCache();

    //This is how we access the cache
    OutageDataWrapper operator[](size_t index);

    const unsigned long DEFAULT_MAX_BYTES = 1024*1024*64;

    bool reloadCache(const QString & filename);
    const bool & validFile() { return _validFile; }

    void setMaxBytes(const unsigned long & maxBytes);
    const unsigned long & maxBytes() { return _cacheParams.maxBytes; }
    const unsigned long & effectiveMaxBytes() { return _effectiveMaxBytes; }
    const unsigned long & fileEffectiveBytes() { return _fileEffectiveBytes; }
    double maxKBytes() { return static_cast<double>(_cacheParams.maxBytes) * 1024.0; }
    double maxMBytes() { return maxKBytes() * 1024.0; }
    double effectiveMaxKBytes() { return static_cast<double>(_effectiveMaxBytes) * 1024.0; }
    double effectiveMaxMBytes() { return effectiveMaxKBytes() * 1024.0; }
    double fileEffectiveKBytes() { return static_cast<double>(_fileEffectiveBytes) * 1024.0; }
    double fileEffectiveMBytes() { return fileEffectiveKBytes() * 1024.0; }

    void setTotalSlicesPerCache(const size_t & slices);
    const size_t & totalSlicesInFile() { return _totalSlicesInFile; }
    const size_t & totalSlicesPerCache() { return _totalSlicesPerCache; }
    const size_t & itemsPerSlice() { return _itemsPerSlice; }
    const size_t & totalInputItemsInFile() { return _totalInputItemsInFile; }
    const size_t & totalColumns() { return _totalColumns; }
    const size_t & sliceSize() { return _sliceSize; }
    const size_t & totalGroups() { return _totalGroups; }

    size_t groupId(const size_t & itemIndex);
    size_t cacheIndex(const size_t & itemIndex);
    size_t sliceId(const size_t & itemIndex);
    size_t sliceIndex(const size_t & itemIndex);

    void clearCache();
    const size_t & length() { return _totalInputItemsInFile; }

    void printHistogram();

    const CacheParameters & cacheParams() const { return _cacheParams; }

private:
    InputCache(const InputCache & l)=delete;
    InputCache & operator=(const InputCache & l)=delete;

    CacheParameters _cacheParams;
    bool _validFile;
    std::vector<CacheSlice> _cacheSlices;

    std::vector<unsigned long> _histogram;
    size_t _histogramSize;

    unsigned long _effectiveMaxBytes;
    unsigned long _fileEffectiveBytes;
    size_t _totalSlicesPerCache;
    size_t _totalSlicesInFile;
    size_t _itemsPerSlice;
    size_t _totalInputItemsInFile;
    size_t _totalColumns;
    size_t _sliceSize;
    size_t _totalGroups;

    bool verifyInputFile();
    void updateCache();
    bool reloadCacheSlice(const size_t & itemIndex);

    // Histogram
    void calculateHistogramSize();
    void incHisto(const size_t & index);
};

#endif // INPUTCACHE_H
