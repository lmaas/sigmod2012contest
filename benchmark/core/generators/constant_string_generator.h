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

#ifndef BENCHMARK_CORE_GENERATORS_CONSTANT_STRING_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_CONSTANT_STRING_GENERATOR_H_

#include "string_generator.h"

// Defines a string generator that constanly returns the same value
class ConstantStringGenerator: public StringGenerator {
public:
  // Constructor
  ConstantStringGenerator(const std::string& constant){
    current_=constant;
  }
  
  // Generates and returns the next string
  std::string next(){
    return current_;
  }
  
  // Resets the generator to its start configuration
  void reset(){
    // Nothing...
  }
  
  // Clones the generator
  Generator* clone() const {return new ConstantStringGenerator(*this);}
};

#endif // BENCHMARK_CORE_GENERATORS_CONSTANT_STRING_GENERATOR_H_

