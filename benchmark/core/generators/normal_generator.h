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

#ifndef BENCHMARK_CORE_GENERATORS_NORMAL_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_NORMAL_GENERATOR_H_

#include <cmath>

#include "integer_generator.h"
#include "core/utils/rng.h"

// A number generator that generates random integers according to the normal
// distribution by using the Marsaglia polar method
class NormalGenerator : public IntegerGenerator {
 public:
  // Initializes the generator using the given mean (mu) and
  // standard deviation (sigma)
  NormalGenerator(int mu, unsigned int sigma, RandomNumberGenerator &rng):
  rng_(rng),spare_available_(false),mu_(mu),sigma_(sigma){}

  // Generates and returns the next number
  int64_t next(){
    if(spare_available_){
      spare_available_ = false;
      return mu_ + sigma_*spare_;
    }
    double u,v,s;
    do {
      u = rng_.NextDouble()*2 - 1;
      v = rng_.NextDouble()*2 - 1;
      s = u*u + v*v;
    } while((s >= 1) || (s == 0));

    spare_available_ = true;
    spare_ = v*sqrt(-2.0*log(s)/s);
    return mu_ + sigma_ * u * sqrt(-2.0*log(s)/s);
  }

  // Resets the generator to its start configuration
  void reset(){
    // Nothing...
  }

  // Changes the random number generator of the generator
  void rng(RandomNumberGenerator &rng){
    rng_ = rng;
  }

  // Clones the generator
  Generator* clone() const {
    return new NormalGenerator(*this);
  }

 private:
  // The random number generator used to generate the next value
  RandomNumberGenerator& rng_;

  // Whether there is still a value left from the last generated value pair
  bool spare_available_;

  // A spare value
  double spare_;

  // The mean of the normal distribution
  int mu_;

  // The standard deviation of the normal distribution
  int sigma_;
};

#endif // BENCHMARK_CORE_GENERATORS_NORMAL_GENERATOR_H_

