#ifndef RANDOMNUMBERENGINE_H
#define RANDOMNUMBERENGINE_H

#include <random>
#include "custommath.h"

class RandomNumberEngine
{
public:
    RandomNumberEngine(unsigned long s = 0);

    const unsigned long & seed() { return _seed; }
    const std::default_random_engine & engine() { return _engine; }

    void setSeed(unsigned long s);

    size_t uniformUnsignedInt(const size_t & low, const size_t & high);
    int uniformSignedInt(const int & low, const int & high);
    double uniformReal(const double & low, const double & high);
    long double uniformReal(const long double & low, const long double & high);

private:
    std::default_random_engine _engine;
    unsigned long _seed;


};

#endif // RANDOMNUMBERENGINE_H
