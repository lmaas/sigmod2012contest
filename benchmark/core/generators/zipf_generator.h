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

#ifndef BENCHMARK_CORE_GENERATORS_ZIPF_GENERATOR_H_
#define BENCHMARK_CORE_GENERATORS_ZIPF_GENERATOR_H_

#include <cassert>

#include "integer_generator.h"

// A Generator that generates integers according to a Zipf distribution
// (sometimes also refered to as zeta distribution)
//
// It follows the algortihm described in
// "Quickly Generating Billion-Record Synthetic Databases"
class ZipfGenerator: public IntegerGenerator {
 public:
  // Creates a new zipfian generator that generates values in the interval
  // [min, max] with the given zipfian constant theta
  ZipfGenerator(int64_t min, int64_t max, double theta,
                RandomNumberGenerator &rng):
                min_(min),max_(max),theta_(theta),rng_(rng){
    assert(theta<1);
    assert(theta>0);
    unsigned int item_count = (max_-min_)+1;
    alpha_ = 1 / (1 - theta_);
    zetan_ = zeta(item_count,theta_);
    eta_ = (1 - pow(2.0f / item_count, 1 - theta_)) /
           (1 - zeta(theta_, 2) / zetan_);
  }

  // Generates and returns the next number
  int64_t next() {
    double u = rng_.NextFloat();
    double uz = u * zetan_;
    if (uz < 1) return (current_=min_);
    if (uz < 1 + pow(0.5f, theta_)) return (current_=min_+1);

    return (current_= (
                       min_ +
                       (long) (((max_-min_)+1) * pow(eta_*u - eta_ + 1, alpha_))
                      )
           );
  }

  // Resets the generator to its start configuration
  void reset(){
    // Nothing...
  }

  // Changes the random number generator of the generator.
  void rng(RandomNumberGenerator &rng){rng_ = rng;}

  // Clones the generator
  Generator* clone() const {return new ZipfGenerator(*this);}

 private:
  // The lower bound of the interval
  const int64_t min_;

  // The upper bound of the interval
  const int64_t max_;


  const double theta_;
  double zetan_;
  double alpha_;
  double eta_;

  // The RNG to be used
  RandomNumberGenerator &rng_;

  // Compute the zeta constant
  double zeta(double n,double theta){
    double res = 0.0f;
    for (int i=0;i<n;i++){
      res += 1/(pow(i+1,theta));
    }
    return res;
  }
};

#endif // BENCHMARK_CORE_GENERATORS_ZIPF_GENERATOR_H_

