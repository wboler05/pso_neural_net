#ifndef CUSTOMMATH_H
#define CUSTOMMATH_H

#include <QVector>
#include <QtMath>
#include <QDebug>

/**
 * @brief The CustomMath namespace is implemented to store static methods for faster math functions.
 * @author William Boler
 */
namespace CustomMath
{
    double pow(double base, int exponent);
    double poly(double input, double *coeffs, int order);
    double poly(double input, QVector<double> coeffs, int order);

    int mod(int a, int N);

    QVector<double> bestFitLine(const QVector<double> &t, const QVector<double> &b, bool intZero);

    double arcDistanceFromLatLong(const double &lat1,
                                  const double &long1,
                                  const double &lat2,
                                  const double &long2);
}

#endif // CUSTOMMATH_H
