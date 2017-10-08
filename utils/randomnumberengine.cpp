#include "randomnumberengine.h"

RandomNumberEngine::RandomNumberEngine(unsigned long s = 0) : _engine(0), _seed(s)
{

}

void RandomNumberEngine::setSeed(unsigned long s) {
    _seed = s;
    _engine.seed(s);
}

size_t RandomNumberEngine::uniformUnsignedInt(const size_t & low, const size_t & high) {
    std::uniform_int_distribution<size_t> dist(low, high);
    return dist(_engine);
}

int RandomNumberEngine::uniformSignedInt(const int & low, const int & high) {
    std::uniform_int_distribution<int> dist(low, high);
    return dist(_engine);
}

double RandomNumberEngine::uniformReal(const double & low, const double & high) {
    std::uniform_real_distribution<double> dist(low, high);
    return dist(_engine);
}

long double RandomNumberEngine::uniformReal(const long double & low, const long double & high) {
    std::uniform_real_distribution<long double> dist(low, high);
    return dist(_engine);
}
