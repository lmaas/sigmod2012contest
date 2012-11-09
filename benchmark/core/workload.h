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

#ifndef BENCHMARK_CORE_WORKLOAD_H_
#define BENCHMARK_CORE_WORKLOAD_H_

#include <vector>
#include "benchmark_properties.h"
#include "statistics.h"
#include "logger.h"

// Defines an abstract interface for workloads.
class Workload {
 public:
  // Constructs a new Workload that uses the given logger
  Workload(Logger &logger):logger_(logger){}
  
  // Initializes the workload with the given properties
  virtual bool Init(BenchmarkProperties &properties)=0;
  
  // Runs the workload and returns a statistics object
  virtual Statistics *Run()=0;
  
  // Cleans up and frees all allocated memory
  virtual bool Cleanup()=0;
  
 protected:
  // The logger to be used
  Logger &logger_;
  
  // Runs the workload without gathering statistics (warm up) 
  virtual void Warmup()=0;
  
  friend class Benchmark;
};

#endif // BENCHMARK_CORE_WORKLOAD_H_

