#include "activationfunctions.h"

namespace ActivationFunctions {

real ReLU(const real & in) {
    if (in > 0) {
        return in;
    } else {
        return 0;
    }
}

real Sin(const real & in) {
    return sin(in * 2.0 * M_PI);
}

real Sigmoid(const real & in, const real & k) {
    return 1.0 / (1.0 + exp(- k * in));
}

real HypTan(const real & in) {
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
    real act = CustomMath::poly(in * M_PI, coeffs, 9);
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

real Sinc(const real & in) {
    if (in == 0.0) {
        return 1.0;
    } else {
        return sin(2.0 * M_PI * in) / (2.0 * M_PI * in);
    }
}

real Step(const real & in, const real & k) {
    if (in < 0) {
        return 0.0;
    } else {
        return 1.0 * k;
    }

}

real deriveReLU(const real & in) {
    if (in < 0) {
        return 0.0;
    } else {
        return 1.0;
    }
}

real deriveSin(const real & in) {
    return 2.0 * M_PI * cos(2.0 * M_PI * in);
}

real deriveSigmoid(const real & in, const real & k) {
    real sig = Sigmoid(in, k);
    return sig * (1.0 - sig);
}

real deriveHypTan(const real & in) {
    return 1.0 - CustomMath::pow(HypTan(in), 2);
}

real deriveGaussian(const real & in, const real & k) {
    return -2.0 * in * Gaussian(in, k);
}

real deriveSinc(const real & in) {
    if (in == 0.0) {
        return 0;
    } else {
        return (cos(2*M_PI*in) / (2*M_PI*in)) - (sin(2*M_PI*in)/(CustomMath::pow(2*M_PI*in, 2)));
    }
}

real deriveStep(const real & in) {
    if (in == 0.0) {
        return 1.0;
    } else {
        return 0.0;
    }
}

}
