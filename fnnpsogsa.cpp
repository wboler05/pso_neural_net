#include "fnnpsogsa.h"

FNNPSOGSA::FNNPSOGSA()
{

}

std::vector<double> FNNPSOGSA::inputize() {
    std::vector<double> vals;
    vals.resize(10);

    vals[0] = etco2;
    vals[1] = age;
    vals[2] = sysBloodPres;
    vals[3] = sao2;
    vals[4] = boolToDouble(hemoptysis);
    vals[5] = boolToDouble(tobacco);
    vals[6] = boolToDouble(surgery);
    vals[7] = boolToDouble(cardiac);
    vals[8] = boolToDouble(dvtpe);
    vals[9] = boolToDouble(uls);

    return vals;
}

double FNNPSOGSA::boolToDouble(bool t) {
    if (t) {
        return 1.0f;
    } else {
        return 0.0f;
    }
}
