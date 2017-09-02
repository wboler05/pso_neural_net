#include "logger.h"

std::queue<std::string> Logger::_queue;
std::string Logger::_file;
std::mutex Logger::_writeMtx;
std::mutex Logger::_textBrowserMtx;
std::thread Logger::_loggingThread(Logger::LoggingConsummer);
bool Logger::_fileSet = false;
bool Logger::_verboseFlag = true;
bool Logger::_terminateFlag = false;
bool Logger::_scrollToBottomFlag = true;
bool Logger::_enableTbFlag = true;
QPointer<QTextBrowser> Logger::_outputBrowser;


/// Sets write file and calls LoggingConsumer() thread
void Logger::setOutputFile(std::string file) {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
    _file = file;
    _fileSet = true;
    lock1.unlock();
}

/// Allows user to print to screen and file.
void Logger::write(std::string s) {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
    _queue.push(s);
    lock1.unlock();
}

void Logger::LoggingConsummer() {
    std::chrono::milliseconds sleepTime(16);

    for(;;) {

        std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
        lock1.lock();

        if (_queue.size() > 0) {
            QString s(_queue.front().c_str());
            _queue.pop();

            if (_fileSet) {

                //!TODO Replace with Qt object
                std::ofstream outputFile(_file, std::ios::app);
                if (outputFile.is_open()) {
                    outputFile.write(s.toStdString().c_str(), s.toStdString().size());
                    outputFile.close();
                }
            }

            if (_verboseFlag) {

                // Write to TextBrowser
                if (_enableTbFlag) {
                    if (_outputBrowser != nullptr) {

                        _outputBrowser->append(s);

                        if (_scrollToBottomFlag) {
                            _outputBrowser->verticalScrollBar()->setValue(
                                        _outputBrowser->verticalScrollBar()->maximum());
                            _outputBrowser->verticalScrollBar()->update();
                        }
                    }
                }

                // Write to Console
                QTextStream stream (stdout);
                stream << s;
            }
        }

        if (_terminateFlag) {
            std::cout << "Logger: Oh, you want to quit?" << std::endl;
            return;
        }

        // Don't bog me down.
        //std::this_thread::sleep_for(sleepTime);

    }

    // Prevents hiccup
    std::this_thread::sleep_for(sleepTime);
}

void Logger::setOutputBrowser(const QPointer<QTextBrowser> & outputBrowser) {
    std::unique_lock<std::mutex> lock1 (_textBrowserMtx, std::defer_lock);
    lock1.lock();
    _outputBrowser = outputBrowser;
    lock1.unlock();
}

void Logger::setVerbose(bool t) {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
    _verboseFlag = t;
    lock1.unlock();
}

void Logger::terminate() {
    std::unique_lock<std::mutex> lock1(_writeMtx, std::defer_lock);
    lock1.lock();
    _terminateFlag = true;
    std::cout << "Terminator: Terminating logger thread." << std::endl;
    lock1.unlock();
    _loggingThread.join();
}

void Logger::setEnableScrollToBottom(const bool & b) {
    std::unique_lock<std::mutex> lock1 ( _textBrowserMtx, std::defer_lock);
    lock1.lock();
    _scrollToBottomFlag = b;
    lock1.unlock();
}

void Logger::setEnableTextBrowser(const bool &b) {
    std::unique_lock<std::mutex> lock1 ( _textBrowserMtx, std::defer_lock);
    lock1.lock();
    _enableTbFlag = b;
    lock1.unlock();
}
