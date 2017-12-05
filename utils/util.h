#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <sstream>

#include <QDebug>
#include <QTextStream>
#include <QTextBrowser>
#include <QScrollBar>
#include <QPointer>
#include <thread>
#include <mutex>

#include "util.h"
#include "custommath.h"

template <class T>
std::string stringPut(T s);

template <class T>
T numberFromString(const std::string & s);

std::vector<real> cdfUniform(const std::vector<real> & nonNormalPdf);

int median(std::vector<int> vect);
// assumes all the ints in the input vector are supposed to be > 0
int mode(std::vector<int> vect);


#endif // UTIL_H
