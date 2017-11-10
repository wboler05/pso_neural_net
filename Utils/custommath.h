#ifndef CUSTOMMATH_H
#define CUSTOMMATH_H

#include <QVector>
#include <QtMath>
#include <QDebug>

//typedef long double real;
typedef double real;

/**
 * @brief The CustomMath namespace is implemented to store static methods for faster math functions.
 * @author William Boler
 */
namespace CustomMath
{
    real pow(real base, int exponent);
    real poly(real input, real *coeffs, int order);
    real poly(real input, QVector<real> coeffs, int order);

    int mod(int a, int N);

    QVector<real> bestFitLine(const QVector<real> &t, const QVector<real> &b, bool intZero);

    double arcDistanceFromLatLong(const double &lat1,
                                  const double &long1,
                                  const double &lat2,
                                  const double &long2);

    int ceilDiv(int a, int b);

    template <class T>
    T getSign(const T & val) {
        if (val >= 0) {
            return static_cast<T>(1);
        } else {
            return static_cast<T>(-1);
        }
    }

    template <class T>
    real mean(const std::vector<T> & v) {
        if (v.size() == 0) {
            return 0;
        }

        real tot = static_cast<real>(total(v));
        return tot / static_cast<real>(v.size());
    }

    template <class T>
    T total(const std::vector<T> & v) {
        T tot = 0;
        for (size_t i = 0; i < v.size(); i++) {
            tot += v[i];
        }
        return tot;
    }
}

#endif // CUSTOMMATH_H
