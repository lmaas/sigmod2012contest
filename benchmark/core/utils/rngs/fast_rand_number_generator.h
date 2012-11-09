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

#ifndef BENCHMARK_CORE_UTILS_RNGS_FAST_RAND_NUMBER_GENERATOR_H_
#define BENCHMARK_CORE_UTILS_RNGS_FAST_RAND_NUMBER_GENERATOR_H_

#include "core/utils/rng.h"

// Implements a random number generator based on the fast rand routine
// It has the same output value range as the C lib random number generator
class FastRandNumberGenerator : public RandomNumberGenerator {
public:
  // Constructs a new fast rand number generator and seeds it with the default
  // seed
  FastRandNumberGenerator(){this->SeedDefault();}

  // Constructs a new fast rand number generator and seeds it with the given
  // seed
  FastRandNumberGenerator(unsigned int seed){this->Seed(seed);}

  // Seeds the RNG with the given seed
  void Seed(unsigned int seed){seed_ = seed;}

  // Seeds the RNG with the default seed
  void SeedDefault(){seed_ = 88172645463325252LL;}

  // Returns a random number between min() and max()
  uint64_t Next(){
    seed_ = (214013*seed_+2531011);
    return (seed_>>16)&0x7FFF;
  }

  // Returns the boundaries
  uint64_t max() const {return UINT64_MAX;}
  uint64_t min() const {return 0;}

private:
  // The current state of the fast rand generator
  uint64_t seed_;
};

#endif // BENCHMARK_CORE_UTILS_RNGS_FAST_RAND_NUMBER_GENERATOR_H_

