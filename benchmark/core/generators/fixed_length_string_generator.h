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

#ifndef BENCHMARK_CORE_GENERATORS_FIXED_LENGTH_STRING_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_FIXED_LENGTH_STRING_GENERATOR_H_

#include <cstring>

#include "core/utils/rngs/fast_rand_number_generator.h"
#include "string_generator.h"

// Defines a string generator that returns strings of a fixed length
class FixedLengthStringGenerator: public StringGenerator {
 public:
  // Creates a new fixed length string generator that generates strings of
  // length <length>
  FixedLengthStringGenerator(unsigned int length, RandomNumberGenerator &rng):
  length_(length),seed_(rng.Next()){
    rng_ = new FastRandNumberGenerator(seed_);
    current_.resize(length_);
  }

  // Destructor
  ~FixedLengthStringGenerator(){
    delete rng_;
  }

  // Resets the generator to its start configuration
  void reset(){
    delete rng_;
    rng_ = new FastRandNumberGenerator(seed_);
  }

  // Generates and returns the next string
  std::string next(){
    // Restrict the characters set to contain only readable characters
    static const char text[]="abcdefghijklmnopqrstuvwxyz"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "0123456789*+-=";
    //"(){}[]|/\\"
    //"?!#_.:,;";

    // Generate the string
    for(unsigned int i = 0; i < length_;i++){
      current_[i] = text[rng_->Next()%strlen(text)];

    }

    return current_;
  }

  // Clones the generator
  Generator* clone() const {return new FixedLengthStringGenerator(*this);}

 private:
  // The length of each generated string
  unsigned int length_;

  // The RNG to be used
  RandomNumberGenerator *rng_;

  // The seed used to reset the generator
  int64_t seed_;
};

#endif // BENCHMARK_CORE_GENERATORS_FIXED_LENGTH_STRING_GENERATOR_H_
