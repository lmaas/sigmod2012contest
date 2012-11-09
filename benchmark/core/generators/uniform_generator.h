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

#ifndef BENCHMARK_CORE_GENERATORS_UNIFORM_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_UNIFORM_GENERATOR_H_

#include "core/utils/rng.h"
#include "integer_generator.h"

// Defines a number generator that generates integers randomly using an uniform
// distribution in a given interval
class UniformGenerator: public IntegerGenerator {
 public:
  // Creates a new uniform generator that returns integers uniformly
  // randomly from the interval [lower_bound, upper_bound]
  UniformGenerator(int64_t lower_bound, int64_t upper_bound,
                   RandomNumberGenerator &rng):
                   lower_bound_(lower_bound),rng_(rng){
    interval_ = upper_bound-lower_bound+1;
  }
  
  // Generates and returns the next number
  int64_t next() {
    current_ = lower_bound_ + (rng_.Next()%(interval_));
    return current_;
  }
  
  // Resets the generator to its start configuration
  void reset(){}
  
  // Changes the random number generator of the generator
  void rng(RandomNumberGenerator &rng){rng_ = rng;}
  
  // Clone the generator
  Generator* clone() const {return new UniformGenerator(*this);}
  
 private:
  // The length of the interval
  uint64_t interval_;
  
  // The lower bound of the interval
  int64_t lower_bound_;
  
  // The RNG to be used to generate the next values
  // (should use a uniform distribution itself) 
  RandomNumberGenerator &rng_;
};

#endif // BENCHMARK_CORE_GENERATORS_UNIFORM_GENERATOR_H_

