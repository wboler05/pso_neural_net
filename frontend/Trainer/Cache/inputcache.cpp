#include "inputcache.h"

InputCache::InputCache(const CacheParameters &c) :
    _effectiveMaxBytes(0),
    _fileEffectiveBytes(0),
    _totalSlicesPerCache(1),
    _validFile(false)
{
    _cacheParams.maxBytes = DEFAULT_MAX_BYTES;
    _cacheParams.inputFileName = c.inputFileName;
    _cacheParams.headerSize = c.headerSize;

    reloadCache(c.inputFileName);

    setMaxBytes(c.maxBytes);
    setTotalSlicesPerCache(c.totalSlicesPerCache);

}

InputCache::~InputCache() {
    _cacheSlices.clear();
}

bool InputCache::reloadCache(const QString & fileName) {
    _cacheParams.inputFileName = fileName;
    if (!verifyInputFile()) {
        qWarning() << "Error: could not open input file.";
        return false;
    }
    return true;
}

void InputCache::setMaxBytes(const unsigned long &maxBytes) {
    //!TODO: Find max of system
    if (_cacheParams.maxBytes < DEFAULT_MAX_BYTES) {
        qWarning() << "InputCache: Warning, too small of value for maximum bytes. Set to default (64MB).";
        _cacheParams.maxBytes = DEFAULT_MAX_BYTES;
    } else {
        _cacheParams.maxBytes = maxBytes;
    }

    if (_fileEffectiveBytes < maxBytes){
        _effectiveMaxBytes = _fileEffectiveBytes;
    } else {
        _effectiveMaxBytes = maxBytes;
    }
    updateCache();
}

void InputCache::setTotalSlicesPerCache(const unsigned int &slices) {
    unsigned int maxCacheSliceNum = _effectiveMaxBytes / sizeof(CacheSlice);

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
    if (_cacheParams.inputFileName.isEmpty()) {
        qWarning() << "InputCache: Error, input file name is empty.";
        return false;
    }

    QFile inFile (_cacheParams.inputFileName);
    if (!inFile.open(QFile::ReadOnly)) {
        qWarning() << "InputCache: Unable to open file: " << _cacheParams.inputFileName;
        return false;
    }

    _totalInputItemsInFile = 0;
    _fileEffectiveBytes = 0;
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

    if (_totalInputItemsInFile >= _cacheParams.headerSize) {
        _totalInputItemsInFile -= _cacheParams.headerSize;
    } else {
        _totalInputItemsInFile = 0;
    }

    _fileEffectiveBytes = _totalInputItemsInFile * sizeof(OutageDataItem);

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
    _cacheSlices.clear();
    _cacheSlices.resize(_totalSlicesPerCache);
    _itemsPerSlice = ceilDiv(_effectiveMaxBytes, (_totalSlicesPerCache * sizeof(OutageDataItem)));
    _sliceSize = _itemsPerSlice * sizeof(OutageDataItem);
    _totalSlicesInFile = ceilDiv(_totalInputItemsInFile, _itemsPerSlice);
    _totalGroups = ceilDiv(_totalSlicesInFile, _totalSlicesPerCache);

    //TEST
//    qDebug() << "Test cacheId";
//    for (size_t i = 0; i < _totalInputItemsInFile; i++) {
//        if (i % _itemsPerSlice == 0 || i+1 == _totalInputItemsInFile)
//            qDebug() << "Index(" << i << "): " << cacheId(i);
//    }
//    qDebug() << "End test cacheId";
}

size_t InputCache::groupId(const size_t & itemIndex) {
    if (itemIndex < _totalInputItemsInFile
            && _validFile
            && _totalSlicesPerCache != 0) {
        return sliceId(itemIndex) / _totalSlicesPerCache;
        //return (itemIndex / _totalSlicesInFile) % _totalSlicesPerCache;
        //return sliceId(itemIndex) % _totalSlicesPerCache;
    } else {
        return 0;
    }
}

size_t InputCache::sliceId(const size_t & itemIndex) {
    if (itemIndex < _totalInputItemsInFile
            && _validFile
            && _itemsPerSlice != 0) {
        return itemIndex / _itemsPerSlice;
        //return (itemIndex / _totalSlicesInFile) / _itemsPerSlice;
    } else {
        return 0;
    }
}

size_t InputCache::sliceIndex(const size_t & itemIndex) {
    if (itemIndex < _totalInputItemsInFile
            && _validFile
            && _itemsPerSlice != 0) {
        return itemIndex % _itemsPerSlice;
    } else {
        return 0;
    }
}

size_t InputCache::cacheIndex(const size_t & itemIndex) {
    if (itemIndex < _totalInputItemsInFile
            && _validFile
            && _totalSlicesPerCache != 0) {
        return sliceId(itemIndex) % _totalSlicesPerCache;
    } else {
        return 0;
    }
}

OutageDataWrapper & InputCache::operator[](size_t index) {
    OutageDataWrapper nullObject;

    if (_cacheSlices.size() == 0
            || _effectiveMaxBytes == 0
            || _totalSlicesPerCache == 0
            || _validFile == false) {
        return nullObject;
    }
    if (index >= _effectiveMaxBytes) {
        return nullObject;
    }

    // For now, lets load consecutive chunks of data
    size_t cacheIndex_ = cacheIndex(index);
    if (cacheIndex_ > _cacheSlices.size()) {
        return nullObject;
    }

    CacheSlice & cacheSlice = _cacheSlices[cacheIndex_];
    size_t groupId_ = groupId(index);
    if (groupId_ != cacheSlice._groupId) {
        if (reloadCacheSlice(index)) {
            size_t sliceIndex_ = sliceIndex(index);
            return OutageDataWrapper(cacheSlice._slice.at(sliceIndex_));
        } else {
            return nullObject;
        }
    } else {
        size_t sliceIndex_ = sliceIndex(index);
        return OutageDataWrapper(cacheSlice._slice.at(sliceIndex_));
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
    QFile inputFile(_cacheParams.inputFileName);
    if (!inputFile.open(QFile::ReadOnly)) {
        qWarning() << "InputCache::reloadCacheSlice: Error, cannot open file.";
        _validFile = false;
        return false;
    }

    QTextStream input(&inputFile);

    size_t startIndex = (itemIndex / _itemsPerSlice) *
            _itemsPerSlice + _cacheParams.headerSize;
    //size_t endIndex = startIndex + _itemsPerSlice;

    for (size_t i = 0; i < startIndex; i++) {
        if (input.readLine().isEmpty()) {
            qWarning() << "InputCache::reloadCacheSlice: Error, end of file.";
            return false;
        }
    }

    CacheSlice & cacheSlice = _cacheSlices.at(cacheIndex(itemIndex));
    cacheSlice._slice.clear();
    cacheSlice._groupId = groupId(itemIndex);

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

void InputCache::clearCache() {
    _cacheSlices.clear();
}
