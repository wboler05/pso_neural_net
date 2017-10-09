#include "util.h"

/** @brief cdfUniform()
 *  @detail Returns the uniform CDF of the ordered probability sets in the form
 *        [ 1st limit, 2nd limit, ... , kth limit ]
**/
std::vector<real> cdfUniform(const std::vector<real> &n) {
  std::vector<real> cdf;
  real sum = 0;
  for (size_t i = 0; i < n.size(); i++) {
    sum += n[i];
  }
  real nSum = 0;
  for (size_t i = 0; i < n.size(); i++) {
    nSum += n[i] / sum;
    cdf.push_back(nSum);
  }
  return cdf;
}
