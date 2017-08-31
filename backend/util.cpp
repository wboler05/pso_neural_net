#include "util.h"


std::queue<std::string> Logger::_queue;
std::string Logger::_file;
std::mutex Logger::_writeMtx;
std::thread Logger::_loggingThread(Logger::LoggingConsummer);
bool Logger::_fileSet = false;
bool Logger::_verboseFlag = true;
bool Logger::_terminateFlag = false;
QPointer<QTextBrowser> Logger::_outputBrowser;


/// Sets write file and calls LoggingConsumer() thread
void Logger::setOutputFile(std::string file) {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
  _file = file;
  _fileSet = true;
}

/// Allows user to print to screen and file.
void Logger::write(std::string s) {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
    _queue.push(s);
}

void Logger::LoggingConsummer() {
  for(;;) {

    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();

    if (_queue.size() > 0) {
        QString s(_queue.front().c_str());
        _queue.pop();

        if (_fileSet) {

            std::ofstream outputFile(_file, std::ios::app);
            if (outputFile.is_open()) {
              outputFile.write(s.toStdString().c_str(), s.toStdString().size());
              outputFile.close();
            }
        }

        if (_verboseFlag) {
            if (_outputBrowser != nullptr) {
                _outputBrowser->append(s);

                const int & scrollPos = _outputBrowser->verticalScrollBar()->value();
                const int & scrollMax = _outputBrowser->verticalScrollBar()->maximum();
                if (scrollMax - scrollPos < 5) {
                    _outputBrowser->verticalScrollBar()->setValue(scrollMax);
                }
            }


            QTextStream stream (stdout);
            stream << s;
          //std::cout << s;
        }
    }

    if (_terminateFlag) {
        std::cout << "Logger: Oh, you want to quit?" << std::endl;
        return;
    }

  }

  // Don't bog me down.
  std::chrono::milliseconds sleepTime(16);
  std::this_thread::sleep_for(sleepTime);

}

void Logger::setOutputBrowser(const QPointer<QTextBrowser> & outputBrowser) {
    _outputBrowser = outputBrowser;
    _outputBrowser->append(QString("Boink!\n"));
}

void Logger::setVerbose(bool t) {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
    _verboseFlag = t;
}

void Logger::terminate() {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
    _terminateFlag = true;
    std::cout << "Terminator: Terminating logger thread." << std::endl;
    lock1.unlock();
    _loggingThread.join();
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
