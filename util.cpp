#include "util.h"


std::queue<std::string> Logger::_queue;
std::string Logger::_file;
boost::mutex Logger::_writeMtx;
boost::thread Logger::_loggingThread(Logger::LoggingConsummer);
bool Logger::_fileSet = false;
bool Logger::_verboseFlag = true;


/// Sets write file and calls LoggingConsumer() thread
void Logger::setOutputFile(std::string file) {
  _writeMtx.lock();
  _file = file;
  _fileSet = true;
  _writeMtx.unlock();
}

/// Allows user to print to screen and file.
void Logger::write(std::string s) {
  _writeMtx.lock();
  _queue.push(s);
  _writeMtx.unlock();
}

void Logger::LoggingConsummer() {
  for(;;) {

    _writeMtx.lock();

    if (!_fileSet)
      continue;

    if (_queue.size() > 0) {
      std::string s = _queue.front();
      _queue.pop();

      std::ofstream outputFile(_file, std::ios::app);
      if (!outputFile.is_open()) {
        std::cout << "Failed to open output file.";
        continue;
      } else {
        outputFile.write(s.c_str(), s.size());
      }
      if (_verboseFlag)
        std::cout << s;
      outputFile.close();
    }
    _writeMtx.unlock();

  }

}

void Logger::setVerbose(bool t) {
  _writeMtx.lock();
  _verboseFlag = t;
  _writeMtx.unlock();
}


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
