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
    _validFile = false;
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

    inFile.close();

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
        _validFile = true;
        return true;
    }
}

void InputCache::updateCache() {
    using namespace CustomMath;
    qDebug() << "OutageDataItem Size: " << sizeof(OutageDataItem);
    _cacheSlices.resize(_totalSlicesPerCache);
    _itemsPerSlice = ceilDiv(_maxBytes, (_totalSlicesPerCache * sizeof(OutageDataItem)));
    _sliceSize = _itemsPerSlice * sizeof(OutageDataItem);
    _totalSlicesInFile = ceilDiv(_totalInputItemsInFile, _itemsPerSlice);

    //TEST
//    qDebug() << "Test cacheId";
//    for (size_t i = 0; i < _totalInputItemsInFile; i++) {
//        if (i % _itemsPerSlice == 0 || i+1 == _totalInputItemsInFile)
//            qDebug() << "Index(" << i << "): " << cacheId(i);
//    }
//    qDebug() << "End test cacheId";
}

size_t InputCache::cacheId(const size_t itemIndex) {
    if (itemIndex < _totalInputItemsInFile && _validFile) {
        return sliceId(itemIndex) % _totalSlicesPerCache;
    } else {
        return 0;
    }
}

size_t InputCache::sliceId(const size_t itemIndex) {
    if (itemIndex < _totalInputItemsInFile && _validFile) {
        return itemIndex / _totalSlicesInFile;
    } else {
        return 0;
    }
}

size_t InputCache::sliceIndex(const size_t itemIndex) {
    if (itemIndex < _totalInputItemsInFile && _validFile) {
        return itemIndex % _totalSlicesInFile;
    }
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
    size_t cacheIndex = cacheId(index);
    if (cacheIndex > _cacheSlices.size()) {
        return nullObject;
    }

    CacheSlice & cacheSlice = _cacheSlices[cacheIndex];
    size_t sliceId_ = sliceId(index);
    if (sliceId_ != cacheSlice._sliceId) {
        if (reloadCacheSlice(index)) {
            return OutageDataWrapper(cacheSlice._slice.at(sliceIndex(index)));
        } else {
            return nullObject;
        }
    } else {
        return OutageDataWrapper(cacheSlice._slice.at(sliceIndex(index)));
    }

}

bool InputCache::reloadCacheSlice(const size_t &itemIndex) {
    if (itemIndex >= _totalInputItemsInFile) {
        qWarning() << "InputCache::reloadCacheSlice: Error, stack overflow.";
        return false;
    }

    if (!_validFile) {
        qWarning() << "InputCache::reloadCacheSlice: Error, file is invalid.";
        return false;
    }
    QFile inputFile(_inputFileName);
    if (!inputFile.open(QFile::ReadOnly)) {
        qWarning() << "InputCache::reloadCacheSlice: Error, cannot open file.";
        return false;
    }

    QTextStream input(&inputFile);

    size_t startIndex = (itemIndex / _itemsPerSlice) * _itemsPerSlice + _headerSize;
    size_t endIndex = startIndex + _itemsPerSlice;

    for (size_t i = 0; i < startIndex; i++) {
        if (input.readLine().isEmpty()) {
            qWarning() << "InputCache::reloadCacheSlice: Error, end of file.";
            return false;
        }
    }

    CacheSlice & cacheSlice = _cacheSlices.at(cacheId(itemIndex));
    cacheSlice._slice.clear();
    cacheSlice._sliceId = sliceId(itemIndex);

    for (size_t i = 0; i < _itemsPerSlice; i++) {
        QString line = input.readLine();
        if (line.isEmpty()) {
            break;
        } else {
            OutageDataItem newItem = OutageDataWrapper::parseInputString(line);
            cacheSlice._slice.push_back(newItem);
        }
    }

    if (cacheSlice._slice.size() > 0) {
        return true;
    } else {
        qWarning() << "InputCache::reloadCacheSlice: Loaded empty slice.";
        return false;
    }

}
