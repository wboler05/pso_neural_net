#include "statobject.h"
#include <QDebug>

uint32_t StatObject::_stat_object_count = 0;
uint32_t StatObject::_snapShots = 0;

/**
 * @brief StatObject::StatObject
 * @param max - Maximum size of queue.
 * @details Constructor which takes the maximum window size for the internal queue.
 */
StatObject::StatObject(uint32_t max) :
    _name(QString()),
    _values(new QQueue<double>),
    _valuesTotal(0),
    _valuesTotalSqr(0),
    _events(0),
    _maxEvents(max)
{
    //qDebug() << "Creating StatObject(" << _stat_object_count << ")";
    _stat_object_count++;
}

/**
 * @brief StatObject::StatObject
 * @param name
 * @param max
 * @details Constructor which takes both an object name and a window size.
 */
StatObject::StatObject(QString name, uint32_t max) :
    _name(name),
    _values(new QQueue<double>),
    _valuesTotal(0),
    _valuesTotalSqr(0),
    _events(0),
    _maxEvents(max)
{
    //qDebug() << "Creating StatObject(" << _stat_object_count << ")";
    _stat_object_count++;
}

/**
 * @brief StatObject::~StatObject
 * @details Destructor
 */
StatObject::~StatObject()
{
    delete _values;
    _stat_object_count--;
    //qDebug() << "Deleting StatObject(" << _stat_object_count << ")";
}

/**
 * @brief StatObject::StatObject
 * @param s - StatObject
 * @details Copy-Constructor
 */
StatObject::StatObject(const StatObject &s) {
    for (int i = 0; i < s._values->count(); i++) {
        _values->append(s._values->at(i));
    }

    _name = s._name;
    _valuesTotal = s._valuesTotal;
    _valuesTotalSqr = s._valuesTotalSqr;
    _valuesAvg = s._valuesAvg;
    _valuesVariance = s._valuesVariance;
    _valuesStdDev = s._valuesStdDev;
    _events = s._events;
    _maxEvents = s._maxEvents;

    //qDebug() << "Creating StatObject(" << _stat_object_count << ")";
    _stat_object_count++;
}

/**
 * @brief StatObject::StatObject
 * @param s - StatObject
 * @details Move-Constructor
 */
StatObject::StatObject(StatObject &&s) {
    _name = std::move(s._name);
    _values = std::move(s._values);
    _valuesTotal = std::move(s._valuesTotal);
    _valuesTotalSqr = std::move(s._valuesTotalSqr);
    _valuesVariance = std::move(s._valuesVariance);
    _valuesStdDev = std::move(s._valuesStdDev);
    _events = std::move(s._events);
    _maxEvents = std::move(s._maxEvents);

    //qDebug() << "Creating StatObject(" << _stat_object_count << ")";
    _stat_object_count++;
}

/**
 * @brief StatObject::operator =
 * @param s - StatObject
 * @return &StatObject
 * @details Copy-Assignment
 */
StatObject & StatObject::operator= (const StatObject &s) {
    for (int i = 0; i < s._values->count(); i++) {
        _values->append(s._values->at(i));
    }

    _name = s._name;
    _valuesTotal = s._valuesTotal;
    _valuesTotalSqr = s._valuesTotalSqr;
    _valuesAvg = s._valuesAvg;
    _valuesVariance = s._valuesVariance;
    _valuesStdDev = s._valuesStdDev;
    _events = s._events;
    _maxEvents = s._maxEvents;

    //qDebug() << "Creating StatObject(" << _stat_object_count << ")";
    _stat_object_count++;

    return *this;
}

/**
 * @brief StatObject::operator =
 * @param s - StatObject
 * @return &StatObject
 * @details Move-Assignment
 */
StatObject & StatObject::operator= (StatObject &&s) {

    _name = std::move(s._name);
    _values = std::move(s._values);
    _valuesTotal = std::move(s._valuesTotal);
    _valuesTotalSqr = std::move(s._valuesTotalSqr);
    _valuesVariance = std::move(s._valuesVariance);
    _valuesStdDev = std::move(s._valuesStdDev);
    _events = std::move(s._events);
    _maxEvents = std::move(s._maxEvents);

    qDebug() << "Creating StatObject(" << _stat_object_count << ")";
    _stat_object_count++;

    return *this;
}

/**
 * @brief StatObject::add_val
 * @param val (double)
 * @details This is the primary method to add data points to the list.
 * @details StatObjects only takes data of double type.
 */
void StatObject::add_val(double val)
{
    if (_events == _maxEvents) {
        double dequeueVal = _values->dequeue();
        _valuesTotal -= dequeueVal;
        _valuesTotalSqr -= dequeueVal*dequeueVal;
    } else {
        _events++;
    }

    _valuesTotal += val;
    _valuesTotalSqr += val * val;
    _values->enqueue(val);
}

/**
 * @brief StatObject::reset
 * @details Erases the queue and sets the stats back to zero.
 * @note Maintains the max window size.
 */
void StatObject::reset() {
    while(!_values->empty()) _values->dequeue();
    _valuesTotal = 0;
    _valuesTotalSqr = 0;
    _events = 0;
}

/**
 * @brief StatObject::avg
 * @return average (double)
 * @details Calculates and returns the recorded average.
 * @details A total of the values are kept in memory in order to reduce
 * @details computational cost of executing function.
 */
double StatObject::avg() {
    if (_events != 0) {
        _valuesAvg = _valuesTotal / _events;
        return _valuesAvg;
    } else {
        return 0;
    }
}

/**
 * @brief StatObject::var
 * @return statistical_variance (double)
 * @details Calculates and returns the variance within the recorded window.
 * @details \f$var(valuesTotalSqr,valuesTotal,events)=frac{|valuesTotalSqr-valuesTotal^2|,events}\f$
 * @details where
 * @details \f$valuesTotal=sum_{n=1}^{events}val_n\f$
 * @details \f$valuesTotalSqr=sum_{n=1}^{events}val_n^2\f$
 * @todo Fix doxygen formula to both show correctly and calculate correctly.
 */
double StatObject::var() {
    if (_events <= 1) return 0;
    double variance = 0;
    //double avg_ = avg();
/*
    for (uint32_t i = 0; i < _events; i++) {
        variance += pow(_values->at(i) - avg_, 2);
    }
    variance /= _events - 1;
    */

    variance = std::abs(_valuesTotalSqr - ((_valuesTotal * _valuesTotal)) / (double) _events);
    variance /= (double) (_events - 1);
    _valuesVariance = variance;

    return variance;
}

/**
 * @brief StatObject::std_dev
 * @return standard_deviation (double)
 * @details Calculates the standard deviation by taking the square root of the variance.
 */
double StatObject::std_dev() {
    return std::sqrt(var());
}

/**
 * @brief StatObject::median
 * @return median (double)
 * @details Calculates the median by sorting the values in a separate list and returning the median value.
 * @todo Sort on demand is expensive.
 */
double StatObject::median() {
    //FIXME: Expensive, needs to be corrected
    // - Limitations: Can't sort queue because after certain max, oldest element is removed.
    // - Options: Could track an array of element positions in a queue and keep actual
    //            values in a sorted list.

    QList<double> sortedList = sortIncreasingValue();

    int midway = sortedList.size() / 2;
    return sortedList.at(midway);
}

/**
 * @brief QObject::sortIncreasingValue
 * @details Returns a sorted list.  Costly to sort and return the list.
 * @return QList<double> sorted list of values
 */
QList<double> StatObject::sortIncreasingValue() {
    QList<double> a;
    for (size_t i = 0; i < _values->size(); i++) {
        a.push_back(_values->at(i));
    }
    qs_sort(a, 0, a.size()-1);
    return a;
}

void StatObject::qs_sort(QList<double> &a, const int &lo, const int &hi) {
    // QuickSort from Wikipedia
    if (hi >= a.size()) return;
    if (lo <  hi) {
        int p = qs_partition(a, lo, hi);
        qs_sort(a, lo, p);
        qs_sort(a, p+1, hi);
    }
}

int StatObject::qs_partition(QList<double> &a, const int &lo, const int &hi) {
    // QuickSort Partition from Wikipedia
    const double & p = (a.at(hi) + a.at(lo)) / 2.0;
    int i = lo;
    int j = hi;
    for (;;) {
        while (a[i] < p) {
            i++;
        }
        while (a[j] > p) {
            j--;
        }
        if (i >= j) {
            return j;
        }
        std::swap(a[i], a[j]);
    }
    //!TODO i and j gets optimized out. Fix it.
    j = i; // This prevents the optimizer from killing i and j
}

/**
 * @brief StatObject::getSnapShot
 * @details Creates a new snapshot and returns the object's statistics. Since variables
 * @details are constantly being calculated, it may be necessary to collect a sample at
 * @details any given time.
 * @warning Developer is responsible for deleting SnapShot object.
 * @return StatObject::SnapShot *
 */
StatObject::SnapShot * StatObject::getSnapShot() {
    StatObject::SnapShot *s = new StatObject::SnapShot();
    s->name = this->name();
    s->id = _snapShots++;
    s->total = total();
    s->mean = this->avg();
    //s->median = median();
    s->median = 0;
    s->variance = this->var();
    s->std_dev = this->std_dev();
    s->N = _events;
    s->lastVal = lastVal();
    return s;
}

/**
 * @brief StatObject::lastVal
 * @return last_value (double)
 * @details Gets the latest value added to the queue.
 */
double StatObject::lastVal() {
    if (_values->count() > 0)
        return _values->last();
    else
        return 0;
}
