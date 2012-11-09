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

#include <iostream>
#include "timer.h"

// Creates a new timer
Timer::Timer(){
  started_ = false;
}

// Starts the time measurement
void Timer::Start(){
  started_ = true;
  gettimeofday(&start_, 0);
}

// Stops the time measurement
void Timer::Stop(){
  gettimeofday(&stop_, 0);
}

// Returns the length of the interval Stop()-Start() in milliseconds
unsigned long Timer::milliseconds(){
  return started_?((stop_.tv_sec-start_.tv_sec)*1000
                   + (stop_.tv_usec-start_.tv_usec)/1000):0;
}

// Returns the length of the interval Stop()-Start() in seconds
unsigned long Timer::seconds(){
  return started_?(stop_.tv_sec-start_.tv_sec):0;
}
