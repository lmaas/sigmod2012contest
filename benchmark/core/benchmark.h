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

#ifndef BENCHMARK_CORE_BENCHMARK_H_
#define BENCHMARK_CORE_BENCHMARK_H_

#include "benchmark_properties.h"
#include "logger.h"
#include "statistics.h"
#include "utils/timer.h"
#include "utils/thread.h"
#include "workload.h"

// Defines the base benchmark class
class Benchmark {
 public:
  // Constructs a new benchmark
  Benchmark(Workload &workload, Logger &logger);
  
  // Initializes the benchmark using the given properties
  void Init(BenchmarkProperties props);
  
  // Runs the benchmark including a warmup period (as defined inside the
  // respective benchmark properties)
  bool WarmupAndRun();
  
  // Runs the benchmark
  bool Run();
  
  // Completes the benchmark and display some statistics
  void Close();
  
  // Whether the benchmark has been initialized
  bool initialized() const {return initialized_;}
  
  // Whether the benchmark has already been executed
  bool executed() const {return executed_;}
  
 private:
  // Whether the benchmark has already been initialized
  bool initialized_;
  
  // Whether the benchmark has already been executed
  bool executed_;
  
  // The Benchmark properties
  BenchmarkProperties properties_;
  
  // The workload that will be executed
  Workload &workload_;
  
  // The logger to be used 
  Logger &logger_;
  
  // A timer to measure the time between the Init() and the Close() call
  Timer total_runtime_;
  
  // The statistics returned by
  Statistics *statistics_;
};

// Benchmark thread supporting warmup and measurement period
class BenchmarkThread: public Thread{
 public:
  // Constructs a new benchmark thread
  BenchmarkThread():incv_(0){};
  
  // Enables measurement mode
  void EnableMeasurement(){incv_=1;}
  
  // Disables measurement mode
  void DisableMeasurement(){incv_=0;}
  
  // Whether measurement mode is enabled
  bool measuring(){return (incv_!=0);}
  
  // Increments the given value
  inline void increment(int &value){value+=incv_;}
  
  // Decrements the given value
  inline void decrement(int &value){value+=incv_;}
  
  // Runs the benchmark thread
  virtual void Run() = 0;
  
 private:
  // The increment value (depends on the current mode of the thread)
  volatile int incv_;
};

#endif // BENCHMARK_CORE_BENCHMARK_H_
