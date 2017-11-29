#ifndef ACTIVATIONFUNCTIONS_H
#define ACTIVATIONFUNCTIONS_H

#include "custommath.h"

namespace ActivationFunctions {

real ReLU(const real & in);
real Sin(const real & in);
real Sigmoid(const real & in, const real & k);
real HypTan(const real & in);
real Gaussian(const real & in, const real & k);
real Sinc(const real & in);
real Step(const real & in, const real & k);

real deriveReLU(const real & in);
real deriveSin(const real & in);
real deriveSigmoid(const real & in, const real & k);
real deriveHypTan(const real & in);
real deriveGaussian(const real & in, const real & k);
real deriveSinc(const real & in);
real deriveStep(const real & in);

}

#endif // ACTIVATIONFUNCTIONS_H
