#ifndef LOGGER_H
#define LOGGER_H

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

class Logger {

public:
  Logger() = delete;

  /// Sets write file and calls LoggingConsumer() thread
  static void setOutputFile(std::string file);

  static void setOutputBrowser(const QPointer<QTextBrowser> & outputBrowser);

  static void setEnableScrollToBottom(const bool & b);
  static void setEnableTextBrowser(const bool &b);

  /// Allows user to print to screen and file.
  static void write(std::string s);

  static void setVerbose(bool t);

  static void terminate();

private:

  static void LoggingConsummer();

  static std::queue<std::string> _queue;
  static std::string _file;
  static std::mutex _writeMtx;
  static std::mutex _textBrowserMtx;
  static std::thread _loggingThread;
  static bool _fileSet;
  static bool _verboseFlag;
  static bool _terminateFlag;
  static bool _scrollToBottomFlag;
  static bool _enableTbFlag;

  static QPointer<QTextBrowser> _outputBrowser;

};
#endif // LOGGER_H
