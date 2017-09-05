#ifndef STATOBJECT_H
#define STATOBJECT_H

#include <QQueue>
#include <QList>
#include <cmath>

#define STAT_DEFAULT_MAX 100000
/**
 * @class StatObject
 * @brief The StatObject class is a generic statistics object.
 * @details It stores statistics data and presents values for mean, variance, and standard deviation
 * @details with methods for appending values within a defined window and resetting data when needed.
 * @author William Boler
 */
class StatObject {
public:

    /**
     * @struct SnapShot
     * @brief Data type for passing an instance of statistics data.
     * @details Can be used to gather snapshots of data at different times
     * @details rather than creating multiple StatObject classes.
     * @details Reduces the size of memory usage by eradicating the value queue.
     */
    // Stores a single instance of statistics data
    struct SnapShot {
        QString name;           // Name assigned to StatObject for identification
        uint id;                // Struct ID
        double total;
        double mean;            // Mean
        double median;          // Median
        double variance;        // Statistical Variance
        double std_dev;         // Statistical Standard Deviation
        int N;                  // Current N-number of events
        double lastVal;
    };


    StatObject(unsigned int max=STAT_DEFAULT_MAX);  // Constructor (max events)
    StatObject(QString name, unsigned int max=STAT_DEFAULT_MAX);    // Constructor(name, max events)
    ~StatObject();                                  // Destructor

    StatObject(const StatObject &);                 // Copy Constructor
    StatObject(StatObject &&);                      // Move Constructor
    StatObject & operator= (const StatObject &);    // Copy Assignment
    StatObject & operator= (StatObject &&);         // Move Assignment

    //void copyWithoutList(const StatObject &);       // Obsolete due to SnapShot

    const QString & name() { return _name; }        // Constant reference to name
    void name(QString n) { _name = n; }             // Set the name of the StatObject
    const double &total() { return _valuesTotal; }  // Return the total of all stored values
    double avg();                                   // Return the average
    double var();                                   // Return the statistical variance
    double std_dev();                               // Return the statistical standard deviation
    double median();                                // Return the median
    const QQueue<double> * val() { return _values; }

    void add_val(double val);                       // Add a new event to the data
    double lastVal();                               // Get the last value or return 0 if empty
    void reset();                                   // Reset (Clear) all data

    const unsigned int &events() { return _events; }            // Total number of events (N)
    const unsigned int &max_events() { return _maxEvents; }     // Max events stored
    void max_events(const unsigned int &m) { _maxEvents = m; }  // Set the max events

    static uint stat_object_count() { return _stat_object_count; }  // Static count of StatObjects

    StatObject::SnapShot * getSnapShot();

protected:
    QString _name;              // Name of StatObject
    QQueue<double> *_values;    // Vector of values
    double _valuesTotal;        // Sum of the values
    double _valuesAvg;          // Average of the values
    double _valuesTotalSqr;     // Square of the sum of values (for variance)
    double _valuesVariance;     // Statistical Variance of values
    double _valuesStdDev;       // Statistical standard deviation of values
    unsigned int _events;       // Total events (N)
    unsigned int _maxEvents;    // Max events (Restrict Queue size)

private:
    static uint _stat_object_count;     // StatObject static count
    static uint _snapShots; // Number of snapshots created

};

#endif // STATOBJECT_H
