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

#ifndef BENCHMARK_CORE_GENERATORS_STRING_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_STRING_GENERATOR_H_

#include "core/generator.h"

// Defines the interface for string generators
class StringGenerator: public Generator {
 public:
  // Destructor
  virtual ~StringGenerator() {}
  
  // Generates and returns the next string
  virtual std::string next() = 0;
  
  // Returns the last generated string
  std::string &current() {return current_;}
  
  // Returns the type of the generator
  GeneratorType type() const {return kStringGenerator;}
  
 protected:
  // The last recently generated string
  std::string current_;
};

#endif // BENCHMARK_CORE_GENERATORS_STRING_GENERATOR_H_

