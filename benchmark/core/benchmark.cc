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

#include <iostream>

#include "workload.h"
#include "benchmark.h"
#include "utils/lexical_cast.h"

// Constructs a new benchmark
Benchmark::Benchmark(Workload &workload, Logger &logger):
  initialized_(false),executed_(false),workload_(workload),logger_(logger),
  statistics_(NULL){
    // Nothing...
};

// Initializes the benchmark using the given properties
void Benchmark::Init(BenchmarkProperties props) {
  properties_ = props;
  
  // Start the time measurement
  total_runtime_.Start();
  
  logger_.AddSection("Workload Initialization","Initializing workload");
  
  // Initialize the workload
  if(!workload_.Init(props)){
    logger_.Error("Could not initialize workload. Will abort now...");
  }else{
    initialized_ = true;
  }
  
  logger_.CloseSection();
}

// Runs the benchmark
bool Benchmark::Run(){
  if(!initialized_)
    return false;
  
  // Measurement
  logger_.AddSection("Measurement","Measuring for "
                     +lexical_cast<int>(properties_.measurement_time())
                     +" seconds");
  statistics_ = workload_.Run();
  logger_.CloseSection();
  
  executed_ = true;
  return true;
}

// Runs the benchmark including a warm-up period (as defined inside the
// respective benchmark properties)
bool Benchmark::WarmupAndRun(){
  if(!initialized_)
    return false;
  // Warm-up
  logger_.AddSection("Warm-up","Warming up for "
                     +lexical_cast<int>(properties_.warmup_time())
                     +" seconds");
  workload_.Warmup();
  logger_.CloseSection(true);
    
  // Run
  this->Run();
  return true;
}

// Completes the benchmark and displays some statistics
void Benchmark::Close(){
  if(!executed_)
    return;
  // Do some cleanup work
  logger_.AddSection("Cleanup","Cleaning up");
  if(!workload_.Cleanup())
    logger_.Warning("Could not free all resources. Remaining resources will "
                    "be deleted when benchmark exits.");
  logger_.CloseSection();
  
  // Stop the time measurement
  total_runtime_.Stop();
  
  // Update and print the workload statistics
  statistics_->Add("Overall execution time",
                   lexical_cast<unsigned long>(total_runtime_.milliseconds())
                   +" ms");
  logger_.PrintStatistics(*statistics_);
}
