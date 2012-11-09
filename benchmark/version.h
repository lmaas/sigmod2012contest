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

#ifndef BENCHMARK_VERSION_H_
#define BENCHMARK_VERSION_H_

// The name and filename of the benchmark
#define PRODUCT_NAME "ACM SIGMOD 2012 Programming Contest Benchmark"
#define PRODUCT_FILENAME "sigmod-benchmark"

// The copyright notice
#define COPYRIGHT "(C) 2012 Database Technology Group Dresden"

// The contest website
#define CONTEST_URL "http://wwwdb.inf.tu-dresden.de/sigmod2012contest/"

// The benchmark version
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_REVISION 2

// The API version
#define API_VERSION "1.03"

// A helper function to build the version string
#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define STRINGIFY_HELPER(x) #x

// A string representation of the benchmark version
#define VERSION_STRING STRINGIFY(VERSION_MAJOR) "." \
                       STRINGIFY(VERSION_MINOR) "." \
                       STRINGIFY(VERSION_REVISION)

#endif // BENCHMARK_VERSION_H_
