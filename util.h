#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <sstream>

#include <boost/thread.hpp>

class Logger {

public:
  Logger() = delete;

  /// Sets write file and calls LoggingConsumer() thread
  static void setOutputFile(std::string file);

  /// Allows user to print to screen and file.
  static void write(std::string s);

  static void setVerbose(bool t);

private:

  static void LoggingConsummer();

  static std::queue<std::string> _queue;
  static std::string _file;
  static boost::mutex _writeMtx;
  static boost::thread _loggingThread;
  static bool _fileSet;
  static bool _verboseFlag;

};

template <class T>
std::string stringPut(T s) {
  std::stringstream ss;
  std::string out;
  ss << s;
  ss >> out;
  return out;
}

std::vector<double> cdfUniform(std::vector<double> nonNormalPdf);

#endif // UTIL_H