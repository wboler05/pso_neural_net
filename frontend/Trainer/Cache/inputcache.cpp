#include "inputcache.h"

InputCache::InputCache(const QString &inputFileName,
                       const unsigned long & maxBytes,
                       const size_t & totalSlicesPerCache,
                       const size_t & headerSize) :
    _inputFileName(inputFileName),
    _maxBytes(DEFAULT_MAX_BYTES),
    _totalSlicesPerCache(1),
    _validFile(false),
    _headerSize(headerSize)
{
    if (!verifyInputFile()) {
        qWarning() << "Error: could not open input file.";
    }

    setMaxBytes(maxBytes);
    setTotalSlicesPerCache(totalSlicesPerCache);

}

InputCache::~InputCache() {
    _cacheSlices.clear();
}

OutageDataWrapper & InputCache::operator[](size_t index) {
    OutageDataWrapper nullObject;

    if (_cacheSlices.size() == 0 || _maxBytes == 0 || _totalSlicesPerCache == 0 || _validFile == false) {
        return nullObject;
    }
    if (index >= _maxBytes) {
        return nullObject;
    }

    // For now, lets load consecutive chunks of data
    size_t sliceIndex = index / _totalSlicesPerCache;
    if (sliceIndex > _cacheSlices.size()) {
        return nullObject;
    }

    QPair<size_t, CacheSlice> & cacheSlicePair = _cacheSlices.at(sliceIndex);

    if (cacheSlicePair.first != index) {
        //reloadCacheSlice(sliceIndex);
        return nullObject;
    }

}

void InputCache::setMaxBytes(const unsigned long &maxBytes) {
    //!TODO: Find max of system
    if (_maxBytes < DEFAULT_MAX_BYTES) {
        qWarning() << "InputCache: Warning, too small of value for maximum bytes. Set to default (64MB).";
        _maxBytes = DEFAULT_MAX_BYTES;
    } else {
        _maxBytes = maxBytes;
    }
    updateCache();
}

void InputCache::setTotalSlicesPerCache(const unsigned int &slices) {
    unsigned int maxCacheSliceNum = _maxBytes / sizeof(CacheSlice);

    if (slices == 0) {
        qWarning() << "InputCache: Error, cannot set cache slice to zero.";
    } else if (slices > maxCacheSliceNum) {
        qWarning() << "InputCache: Error, too many slices. Using " << maxCacheSliceNum << " slices";
        _totalSlicesPerCache = maxCacheSliceNum;
    } else {
        _totalSlicesPerCache = slices;
    }
    updateCache();
}

bool InputCache::verifyInputFile() {
    if (_inputFileName.isEmpty()) {
        qWarning() << "InputCache: Error, input file name is empty.";
        return false;
    }

    QFile inFile (_inputFileName);
    if (!inFile.open(QFile::ReadOnly)) {
        qWarning() << "InputCache: Unable to open file: " << _inputFileName;
        return false;
    }

    _totalInputItemsInFile = 0;
    QTextStream input(&inFile);
    QString line;
    do {
        line = input.readLine();
        if (_totalInputItemsInFile++ == 0) {
            QStringList columns = line.split(",");
            _totalColumns = columns.length();
        }
    } while (!line.isNull());
    _totalInputItemsInFile--;

    if (_totalInputItemsInFile >= _headerSize) {
        _totalInputItemsInFile -= _headerSize;
    } else {
        _totalInputItemsInFile = 0;
    }

    if (_totalInputItemsInFile == 0 || _totalColumns == 0) {
        qWarning() << "InputCache: Error, empty lines in input file.";
        return false;
    } else if (_totalColumns == 0) {
        qWarning() << "InputCache: Error, empty columns in input file.";
        return false;
    } else {
        return true;
    }
}

void InputCache::updateCache() {
    using namespace CustomMath;
    qDebug() << "OutageDataItem Size: " << sizeof(OutageDataItem);
    _cacheSlices.resize(_totalSlicesPerCache);
    _itemsPerSlice = ceilDiv(ceilDiv(_maxBytes, _totalSlicesPerCache), sizeof(OutageDataItem));
    _totalSlicesInFile = ceilDiv(_totalInputItemsInFile, _itemsPerSlice);
}
