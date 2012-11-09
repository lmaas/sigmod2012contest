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

#ifndef BENCHMARK_CORE_BENCHMARK_PROPERTIES_H_
#define BENCHMARK_CORE_BENCHMARK_PROPERTIES_H_

#include <map>
#include <string>

// Defines a set of common as well as arbitrary benchmark properties
class BenchmarkProperties {
 public:
  // Returns the number of threads to be used to run the benchmark
  unsigned int thread_count() const{return thread_count_;}
  
  // Returns the duration of the warm-up period
  unsigned int warmup_time() const{return warmup_time_;}
  
  // Returns the duration of the measurement period
  unsigned int measurement_time() const{return measurement_time_;}
  
  // Returns the seed used to initialize the random number generator
  unsigned int seed() const{return seed_;}
  
  // Sets the number of threads to be used to run the benchmark
  void thread_count(unsigned int thread_count){thread_count_=thread_count;}
  
  // Sets the duration of the warm-up period
  void warmup_time(unsigned int warmup_time){warmup_time_=warmup_time;}
  
  // Sets Returns the duration of the measurement period
  void measurement_time(unsigned int measurement_time){
    measurement_time_=measurement_time;
  }
  
  // Sets the seed used to initialize the random number generator
  void seed(unsigned int seed){seed_=seed;}
  
  // Set the given key/value pair
  void Set(std::string key,std::string value){settings[key]=value;}
  
  // Return the value stored under the given key. If no value is stored under
  // the given key, default_value will be returned
  std::string Get(const std::string &key, const std::string &default_value)
    const {
    std::map<std::string,std::string>::const_iterator it = settings.find(key);
    if(it==settings.end())
      return default_value;
    return it->second;
  }
  
 private:
  // The number of threads to be used to run the benchmark
  unsigned int thread_count_;
  
  // The duration of the warm-up period
  unsigned int warmup_time_;
  
  // The duration of the measurement period
  unsigned int measurement_time_;
  
  // The seed used to initialize the random number generator
  unsigned int seed_;
  
  // A set of custom key/value pairs
  std::map<std::string,std::string> settings;
};

#endif // BENCHMARK_CORE_BENCHMARK_PROPERTIES_H_
