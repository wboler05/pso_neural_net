#ifndef INPUTCACHE_H
#define INPUTCACHE_H

#include "cacheslice.h"
#include "outagedatawrapper.h"

#include <QFile>

class InputCache
{
public:
    explicit InputCache(const QString &inputFileName,
               const unsigned long & maxBytes,
               const size_t &totalSlicesPerCache,
               const size_t & headerSize=0);
    ~InputCache();

    OutageDataWrapper & operator[](size_t index);

    const unsigned long DEFAULT_MAX_BYTES = 1024*1024*64;

    const bool & validFile() { return _validFile; }

    void setMaxBytes(const unsigned long & maxBytes);
    const unsigned long & maxBytes() { return _maxBytes; }
    double maxKBytes() { return (double) _maxBytes * 1024.0; }
    double maxMBytes() { return maxKBytes() * 1024.0; }

    void setTotalSlicesPerCache(const size_t & slices);
    const size_t & totalSlicesInFile() { return _totalSlicesInFile; }
    const size_t & totalSlicesPerCache() { return _totalSlicesPerCache; }
    const size_t & itemsPerSlice() { return _itemsPerSlice; }
    const size_t & totalInputItemsInFile() { return _totalInputItemsInFile; }
    const size_t & totalColumns() { return _totalColumns; }

private:
    InputCache(const InputCache & l)=delete;
    InputCache & operator=(const InputCache & l)=delete;

    QString _inputFileName;
    bool _validFile;
    size_t _headerSize;
    std::vector<QPair<size_t, CacheSlice>> _cacheSlices;

    unsigned long _maxBytes;
    size_t _totalSlicesPerCache;
    size_t _totalSlicesInFile;
    size_t _itemsPerSlice;
    size_t _totalInputItemsInFile;
    size_t _totalColumns;

    bool verifyInputFile();
    void updateCache();
};

#endif // INPUTCACHE_H
