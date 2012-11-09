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

#ifndef BENCHMARK_CORE_GENERATORS_CONSTANT_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_CONSTANT_GENERATOR_H_

#include "integer_generator.h"

// Defines a number generator that constanly returns the same integer number
class ConstantGenerator: public IntegerGenerator {
 public:
  // Constructor
  explicit ConstantGenerator(int64_t constant){this->current_=constant;}
  
  // Generates and returns the next number
  int64_t next(){return this->current_;}
  
  // Resets the generator to its start configuration
  void reset(){};
  
  // Clones the generator
  Generator* clone() const {return new ConstantGenerator(*this);}
};

#endif // BENCHMARK_CORE_GENERATORS_CONSTANT_GENERATOR_H_
