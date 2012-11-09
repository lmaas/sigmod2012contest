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

#ifndef BENCHMARK_CORE_UTILS_TIMER_H_
#define BENCHMARK_CORE_UTILS_TIMER_H_

#include <sys/time.h>

// A class that represents a timer
// 
// The same timer may be used to retrieve multiple time intervals
// from the same start:
// 
//   Timer timer();
//   ...
//   timer.Start();
//   ...
//   timer.Stop();
//   a = timer.milliseconds(); <-- interval a
//   ...
//   timer.Stop();
//   b = timer.milliseconds(); <-- interval b
class Timer {
 public:
  // Creates a new timer
  Timer();
  
  // Starts the time measurement
  void Start();
  
  // Stops the time measurement.
  // It might be called multiple times to retrieve the length of different
  // intervals starting from the same point of time.
  void Stop();
  
  // Return the length of the interval Stop()-Start() in milliseconds
  unsigned long milliseconds();
  
  // Return the length of the interval Stop()-Start() in seconds
  unsigned long seconds();

 private:
  // The start time of the timer
  timeval start_;
  
  // The stop time of the timer
  timeval stop_;
  
  // Whether the timer has been started
  bool started_;
};

#endif // BENCHMARK_CORE_UTILS_TIMER_H_
