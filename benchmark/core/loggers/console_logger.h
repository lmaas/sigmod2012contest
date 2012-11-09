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

#ifndef BENCHMARK_CORE_LOGGERS_CONSOLE_LOGGER_H_
#define BENCHMARK_CORE_LOGGERS_CONSOLE_LOGGER_H_

#include "../logger.h"

// A logger that prints information to the standard output
//
// NOTE: This logger is not thread-safe.
class ConsoleLogger : public Logger {
 public:
  // Constructs a new ConsoleLogger
  ConsoleLogger():Logger(),indent_(0),indentation_width_(2),verbose_count_(0){};

  // Constructs a new ConsoleLogger with the given indentation level
  ConsoleLogger(unsigned int indentation_width):Logger(),indent_(0),
  indentation_width_(indentation_width),verbose_count_(0){};

  // Adds a new section with the given name and description
  void AddSection(const std::string &name, const std::string &description,
                  bool verbose=false);

  // Closes the last opened section given a return state (success or failure)
  void CloseSection(bool success=true, std::string details = "");

  // Prints the given statistics
  void PrintStatistics(const class Statistics& stats);

  // Logs a message using the given LogLevel.
  // When newline is set, message will be printed in a new line
  void Log(LogLevel level, const std::string &message, bool newline=true);

 private:
  // The current indendation level
  unsigned int indent_;

  // The number of characters per indentation level
  const unsigned int indentation_width_;

  // Whether the current section can still be printed in one line
  bool section_compact_;

  // The number of section levels after the first verbose section
  unsigned int verbose_count_;

  // Indent the current line
  void indent();
};

#endif // BENCHMARK_CORE_LOGGERS_CONSOLE_LOGGER_H_
