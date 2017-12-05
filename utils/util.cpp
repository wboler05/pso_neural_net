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

int median(std::vector<int> vect){
    std::sort(std::begin(vect), std::end(vect));
    if (vect.size() % 2 == 1){
        return vect[vect.size()/2];
    }
    else{
        return std::round((vect[vect.size()/2] + vect[(vect.size()/2)+1])/2);
    }
}


// assumes all the ints in the input vector are supposed to be > 0
int mode(std::vector<int> vect){
    std::vector<std::pair<int,int>> histogram;
    std::sort(std::begin(vect), std::end(vect));
    int previous = -1;
    int current;
    int histIdx = -1;
    for(size_t i = 0; i < vect.size(); i++){
        current = std::abs(vect[i]);
        if (previous != current){
            histogram.push_back(std::pair<int,int>(current,1));
            histIdx++;
        }
        else{
            histogram[histIdx].second++;
        }
        previous = current;
    }
    int maxCount = 0;
    int modeVal;
    for (size_t i = 0; i < histogram.size(); i++){
        if(histogram[i].second >= maxCount){
            modeVal = histogram[i].first;
        }
    }
    return modeVal;
}
