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

#ifndef BENCHMARK_CORE_GENERATOR_H_
#define BENCHMARK_CORE_GENERATOR_H_

#include "utils/rng.h"

// There are two basic generator types
typedef enum {kIntegerGenerator, kStringGenerator} GeneratorType;

// Defines an abstract interface for data generators
class Generator{
 public:
  // Destructor
  virtual ~Generator() {};
  
  // Resets the generator to its start configuration
  virtual void reset() = 0;
  
  // Returns the type of the generator
  virtual GeneratorType type() const = 0;
  
  // Changes the random number generator of the generator.
  // 
  // Generators must be functional without calling this method
  // (i.e. any RNG that is used by the generator needs to be initialized
  //  inside the generator's constructor)
  //
  // (TODO: It might be better to add an additional abstraction level,
  //        as some generators do not need an RNG at all)
  virtual void rng(RandomNumberGenerator &rng) {};
  
  // Clones the generator
  virtual Generator* clone() const = 0;
};

#endif // BENCHMARK_CORE_GENERATOR_H_
