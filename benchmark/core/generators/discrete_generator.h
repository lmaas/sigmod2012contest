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

#ifndef BENCHMARK_CORE_GENERATORS_DISCRETE_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_DISCRETE_GENERATOR_H_

#include <vector>
#include <cassert>

#include "integer_generator.h"

// Defines a number generator that returns a value based on an individual
// weight. The output range is [0, <number of weights>)
class DiscreteGenerator : public IntegerGenerator {
 public:
  // Creates a new discrete generator using the given weights and random number
  // generator
  DiscreteGenerator(const std::vector<int> &weights, RandomNumberGenerator &rng)
   :rng_(rng){
    int total_weight = 0;
    for(unsigned int i=0; i < weights.size(); i++){
      assert(weights[i] >= 0);
      
      total_weight+=weights[i];
      max_weights_.push_back(total_weight);
    }
  }
  
  // Generates and returns the next number
  int64_t next(){
    int r = rng_.Next()%max_weights_.back();
    for(unsigned int i = 0; i < max_weights_.size(); i++){
      if(r < max_weights_[i]){
        return (this->current_=i);
      }
    }
    
    // Should never get here
    assert(false);
  }
  
  // Resets the generator to its start configuration
  void reset(){
    // Nothing...
  }
  
  // Changes the random number generator of the generator.
  void rng(RandomNumberGenerator &rng){rng_ = rng;}
  
  // Clones the generator
  Generator* clone() const {return new DiscreteGenerator(*this);}
  
 private:
  // A list of calculated max. weights
  std::vector<int> max_weights_;
  
  // The RNG to be used
  RandomNumberGenerator &rng_;
};

#endif // BENCHMARK_CORE_GENERATORS_DISCRETE_GENERATOR_H_

