#ifndef FNNPSOGSA_H
#define FNNPSOGSA_H

#include <vector>

class FNNPSOGSA
{
public:
    FNNPSOGSA();

    std::vector<double> inputize();
    static double boolToDouble(bool t);

    double etco2=0;
    double age=0;
    double sysBloodPres=0;
    double sao2=0;

    bool hemoptysis=0;
    bool tobacco=0;
    bool surgery=0;
    bool cardiac=0;
    bool dvtpe=0;
    bool uls=0;

    bool PE=0;
};

#endif // FNNPSOGSA_H
