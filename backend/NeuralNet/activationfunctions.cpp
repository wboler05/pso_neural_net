#include "activationfunctions.h"

namespace ActivationFunctions {

real ReLU(const real & in) {
    if (in > 0) {
        return in;
    } else {
        return 0;
    }
}

real Sin(const real & in, const real & k) {
    return k*sin(in * 2.0 * M_PI);
}

real Sigmoid(const real & in, const real & k) {
    return 1.0 / (1.0 + k*exp(-in));
}

real HypTan(const real & in, const real & k) {
    using namespace std;
    // 9th order approximation to tanh
    static real coeffs[10] = {
        4.42727699125780,
        -2.78592124641418e-14,
        -12.3878821958288,
        4.49018884445568e-14,
        13.4092500380177,
        -2.26603901811258e-14,
        -7.48382418797760,
        3.72216702500625e-15,
        3.04221199452273,
        -5.55537460355953e-17
    };

    //real act = tanh(in * M_PI);
    // Faster than tanh function
    real act = CustomMath::poly(in * M_PI * k, coeffs, 9);
    act = max(min(act, static_cast<real>(1.0)), static_cast<real>(-1.0));
    return act;
}

real Gaussian(const real & in, const real & k) {
    const real divisor = sqrt(2.0*M_PI);
    if (k == 0.0) {
        return 0;
    } else {
        return exp(-CustomMath::pow(in / k, 2)) / (divisor*k);
    }
}

real Sinc(const real & in, const real & k) {
    if (in*k == 0.0) {
        return 1.0;
    } else {
        return sin(2.0 * M_PI * in * k) / (2.0 * M_PI * in * k);
    }
}

real Step(const real & in, const real & k) {
    if (in < 0) {
        return 0.0;
    } else {
        return 1.0 * k;
    }

}

}
