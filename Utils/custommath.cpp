#include "custommath.h"


/**
 * @brief pow - Returns the simple power defined by the base and exponent.
 *  In opposition to std::pow, pow only works for integer type exponents
 *  and real values for base. Function does handle negative exponents.
 * @param base - Real value to be raised.
 * @param exponent - Integer value of the exponent
 * @return Raised value of the base.
 */
real CustomMath::pow(real base, int exponent) {
    if (base == 0) return 0;
    else if (exponent == 0) return 1;
    else if (exponent > 0) {
        return base * CustomMath::pow(base, exponent - 1);
    } else {
        return CustomMath::pow(base, exponent + 1) / base;
    }
} // end pow()

/**
 * @brief CustomMath::poly - Returns the polynomial solution based on the
 *  array of coefficients and the order expected.
 *  - Order of coeffs wrt exponents: [ order, order-1, order-2, ...., 1, 0 ]
 * @warning User must guard against passing a buffer not consistent with
 *  the order of the function.  coeffs[order + 1] is the expected size.
 * @param input - real value of 'x' input
 * @param coeffs - Array of N coefficients, where N = order + 1.
 * @param order - Order of the polynomial equation
 * @return Polynomial function output (real).
 */
real CustomMath::poly(real input, real *coeffs, int order) {
    if (coeffs == nullptr) return qQNaN();

    // coeffs[order+1] = { Nth order, Nth-1 order, .... , 0th order }
    real output = 0;
//    for (int i = 0; i < order + 1; i++) {
//        output += coeffs[i] * CustomMath::pow(input, order - i);
//    }

    // Convert to Horner's Rule (wboler)
    for (int i = order; i >= 0; i--) {
        output = coeffs[order-i] + input * output;
    }

    return output;
} // end poly()

real CustomMath::poly(real input, QVector<real> coeffs, int order) {
    if (coeffs.count() < order - 1 || coeffs.count() == 0) {
        return qQNaN();
    }
    real output = 0;
    /*
    for (int i = 0; i < order + 1; i++) {
        output += coeffs.at(i) * CustomMath::pow(input, order - i);
    }
    */

    for (int i = order; i >= 0; i--) {
        output = coeffs.at(order-i) + input * output;
    }

    return output;
}


/**
 * @brief CustomMath::bestFitLine
 * @details Returns the coefficients for a best fit line.  Currently implemented
 *   for a straight line that can either intercept at 0 or return:
 *   y = C*t + D
 *   result[2] = { C, D }
 * @param t
 * @param b
 * @param intZero
 * @return QVector<real> result
 * TODO: UPDATE TO ANY POLY
 */
QVector<real> CustomMath::bestFitLine(const QVector<real> &t, const QVector<real> &b, bool intZero) {
    QVector<real> result;
    size_t n = t.count();
    if (t.count() == 0) return result;
    if (b.count() == 0) return result;
    if (t.count() != b.count()) return result;

    if (intZero) {
        // Best fit line for intercept at zero
        real dotProd = 0;
        real t_sqr = 0;
        for (size_t i = 0; i < n; i++) {
            dotProd += t.at(i)*b.at(i);
            t_sqr += t.at(i)*t.at(i);
        }
        if (t_sqr == 0) {
            // Break if dividing by zero
            return result; // Result size 0
        } else {
            real c = dotProd / t_sqr;
            result.append(c);
            result.append(0);
            return result; // Result size 1
        }
    } else {
        // Best fit line with C*t + D
        real sum_t = 0;
        real t_sqr = 0;
        real alpha = 0;
        real C = 0, D = 0;
        for (unsigned  int i = 0; i < n; i++) {
            sum_t += t.at(i);
            t_sqr += t.at(i)*t.at(i);
        }
        alpha = n*t_sqr - (sum_t * sum_t);
        if (alpha == 0) {
            // Break if dividing by zero
            return result; // Result size 0
        } else {
            for (size_t i = 0; i < n; i++) {
                C += b.at(i) * (n * t.at(i) - sum_t);
                D += b.at(i) * (t_sqr - (t.at(i) * sum_t));
            }
            result.append(C / alpha);
            result.append(D / alpha);
            return result; // Result size 2 { C, D }
        }
    }
}

int CustomMath::mod(int a, int N) {
    if (a >= 0) {
        return a % N;
    } else {
        while (a < 0) {
            return (a % N) + N;
        }
    }
    return 0;
}

double CustomMath::arcDistanceFromLatLong(const double &lat1,
                                          const double &long1,
                                          const double &lat2,
                                          const double &long2)
{
   static const double earthRad = 6371.009; // Earth radius in km.

   double delLat = qDegreesToRadians(lat2 - lat1);      // Del Phi
   double delLong = qDegreesToRadians(long2 - long1);   // Del Lambda
   double meanLat = qDegreesToRadians(lat1 + lat2) / 2; // Phi m

   // Spherical Earth projected to a plane
   double distanceKm = earthRad * qSqrt(qPow(delLat,2.0) + qPow(qCos(meanLat)*delLong,2));
   qDebug() << "LatLongDistance: "<<lat1 << ", " << long1 << "; "<<lat2<<", "<<long2<<": "<<distanceKm;
   return distanceKm;
}

int CustomMath::ceilDiv(int a, int b) {
    if (b != 0) {
        int ret = (a % b) ? a / b + 1 : a / b;
        return ret;
    } else {
        return std::numeric_limits<real>::infinity();
    }
}

