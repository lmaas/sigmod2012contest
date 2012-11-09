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

#ifndef BENCHMARK_CORE_UTILS_RANDOM_NUMBER_GENERATOR_H_
#define BENCHMARK_CORE_UTILS_RANDOM_NUMBER_GENERATOR_H_

#include <stdint.h>

// Represents an generic interface for random number generators (RNGs)
class RandomNumberGenerator{
 public:
  // Seeds the RNG with the given seed
  virtual void Seed(unsigned int seed) = 0;

  // Seeds the RNG with the default seed
  virtual void SeedDefault() = 0;

  // Returns a random number between min() and max()
  virtual uint64_t Next() = 0;

  // Returns a random float in the range (0,1)
  virtual float NextFloat(){
    return (float) Next()/max();
  };

  // Returns a random double in the range (0,1)
  virtual float NextDouble(){
    return (double) Next()/max();
  };

  // Returns the boundaries
  virtual uint64_t max() const = 0;
  virtual uint64_t min() const = 0;
};

#endif // BENCHMARK_CORE_UTILS_RANDOM_NUMBER_GENERATOR_H_
