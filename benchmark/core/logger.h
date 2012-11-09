//
// Copyright (c) 2012 TU Dresden - Database Technology Group
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Author: Lukas M. Maas <Lukas_Michael.Maas@mailbox.tu-dresden.de>
//

#ifndef BENCHMARK_CORE_LOGGER_H_
#define BENCHMARK_CORE_LOGGER_H_

#include "statistics.h"

// Identifiers for the supported log levels
enum LogLevel{
 kLogLevelDebug,
 kLogLevelVerbose,
 kLogLevelInfo,
 kLogLevelWarning,
 kLogLevelError,
 kLogLevelFatal
};

// Defines the interface of a logger used to log the events happening
// when executing a workload
class Logger {
 public:
  // Constructor
  Logger():verbose_(false),quiet_(false){};

  // Adds a new section with the given name and description
  virtual void AddSection(const std::string &name,
                          const std::string&description, bool verbose=false)=0;

  // Closes the last opened section given a return state (success or failure)
  virtual void CloseSection(bool success=true, std::string details="") = 0;

  // Prints the given statistics
  virtual void PrintStatistics(const Statistics& stats) = 0;

  // Logs a message using the given LogLevel.
  // When newline is set, message will be printed on a new line
  virtual void Log(LogLevel level,
                   const std::string &message, bool newline=true) = 0;

  // Prints a debug message to the log
  void Debug(const std::string &message, bool newline=true){
    this->Log(kLogLevelDebug,message,newline);
  }

  // Prints a normal log message
  void Info(const std::string &message, bool newline=true){
    this->Log(kLogLevelInfo,message,newline);
  }

  // Prints a verbose log message
  void Verbose(const std::string &message, bool newline=true){
    this->Log(kLogLevelVerbose,message,newline);
  }

  // Prints a warning to the log
  void Warning(const std::string &message, bool newline=true){
    this->Log(kLogLevelWarning,message,newline);
  }

  // Prints an error message to the log
  void Error(const std::string &message, bool newline=true){
    this->Log(kLogLevelError,message,newline);
  }

  // Prints a fatal error to the log
  void Fatal(const std::string &message, bool newline=true){
    this->Log(kLogLevelFatal,message,newline);
  }

  // Sets verbose output mode
  void verbose(bool verbose){verbose_=verbose;}

  // Returns whether verbose output is enabled or disabled
  bool verbose() const {return verbose_;}

  // Sets quiet output mode
  // If set to true the logger will ignore all log messages that do not indicate
  // an error
  void quiet(bool quiet){quiet_=quiet;}

  // Returns whether quiet mode is enabled or disabled
  bool quiet() const {return quiet_;}

 private:
  // Whether verbose output is enabled or disabled
  bool verbose_;

  // Whether quiet mode is enabled or disabled
  bool quiet_;
};

#endif // BENCHMARK_CORE_LOGGER_H_
