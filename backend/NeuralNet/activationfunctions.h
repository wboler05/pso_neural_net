#ifndef ACTIVATIONFUNCTIONS_H
#define ACTIVATIONFUNCTIONS_H

#include "custommath.h"

namespace ActivationFunctions {

real ReLU(const real & in);
real Sin(const real & in, const real & k);
real Sigmoid(const real & in, const real & k);
real HypTan(const real & in, const real &k);
real Gaussian(const real & in, const real & k);
real Sinc(const real & in, const real & k);
real Step(const real & in, const real & k);

}

#endif // ACTIVATIONFUNCTIONS_H
