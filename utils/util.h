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
std::string stringPut(T s) {
  std::stringstream ss;
  std::string out;
  ss << s;
  ss >> out;
  return out;
}

template <class T>
T numberFromString(const std::string & s) {
    std::istringstream ss(s);
    T result;
    return ss >> result ? result : 0;
}

std::vector<real> cdfUniform(std::vector<real> nonNormalPdf);

#endif // UTIL_H
