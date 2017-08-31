#include "util.h"

/** @brief cdfUniform()
 *  @detail Returns the uniform CDF of the ordered probability sets in the form
 *        [ 1st limit, 2nd limit, ... , kth limit ]
**/
std::vector<double> cdfUniform(std::vector<double> n) {
  std::vector<double> cdf;
  double sum = 0;
  for (uint32_t i = 0; i < n.size(); i++) {
    sum += n[i];
  }
  double nSum = 0;
  for (uint32_t i = 0; i < n.size(); i++) {
    nSum += n[i] / sum;
    cdf.push_back(nSum);
  }
  return cdf;
}
