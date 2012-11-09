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

#ifndef BENCHMARK_WORKLOADS_SIGMOD_2012_BASIC_WORKLOAD_H_
#define BENCHMARK_WORKLOADS_SIGMOD_2012_BASIC_WORKLOAD_H_

// Uncomment to enable debug output
//#define DEBUG_

#include <iostream>
#include <cstring>
#include <cassert>

#include "contest_interface.h"

#include "core/benchmark.h"
#include "core/workload.h"
#include "core/generators/integer_generator.h"
#include "core/generators/fixed_length_string_generator.h"
#include "core/utils/lexical_cast.h"
#include "core/utils/size.h"
#include "core/utils/rng.h"
#include "core/utils/thread.h"
#include "core/utils/rngs/xor_shift_number_generator.h"

#include "sigmod_2012_properties.h"

#include <common/macros.h>


// The main workload used to benchmark implemantions for the
// SIGMOD 2012 Programming Contest
class SIGMOD2012BasicWorkload : public Workload {
 public:

  // Constructs a new Workload that uses the given logger
  SIGMOD2012BasicWorkload(Logger &logger):
    Workload(logger),initialized_(false),warmed_up_(false){};

  // Destructor
  ~SIGMOD2012BasicWorkload(){
    Cleanup();
  }

  // Initalizes the workload using the given properties.
  // The properties need to contain the path to a valid workload file
  // ("workload-file")
  bool Init(BenchmarkProperties &properties);

  // Runs the workload and return a statistics object
  Statistics * Run();

  // Cleans up and frees all allocated memory
  bool Cleanup(){
    if(initialized_){
      delete rng_;
      initialized_=false;
    }
    return true;
  }

 private:
  // Runs the workload without gathering statistics (warm up)
  void Warmup();

  // Creates the indices used by the benchmark
  bool CreateIndices();

  // Populates the indices used by the benchmark
  bool PopulateIndices();

  // The random number generator to be used
  RandomNumberGenerator *rng_;

  // The properties file to be used
  SIGMOD2012Properties *properties_;

  // The number of threads that will run the benchmark
  unsigned int thread_count_;

  // The duration of the measurement
  unsigned int measurement_time_;

  // The duration of the warmup period
  unsigned int warmup_time_;

  // Whether the benchmark has already been initialized
  bool initialized_;

  // Whether the benchmark has already been warmed up
  bool warmed_up_;

  // A single thread that is used to populate a given index
  class PopulateThread: public Thread{
   public:
    // Initializes a new population thread that populates the given index
    PopulateThread(Logger &logger,
                   SIGMOD2012IndexProperties &index, unsigned int seed);

    // Destructor
    ~PopulateThread();

    // The main function
    void Run();

    // Returns whether the thread completed successfuly
    bool success(){return success_;};

   private:
    // The random number generator used by this thread
    RandomNumberGenerator* rng_;

    // The index that will be populated
    SIGMOD2012IndexProperties &index_;

    // The private generators used to create new keys
    Generator** generators_;

    // The logger that will be used
    Logger &logger_;

    // Whether the thread completed successfuly
    bool success_;

    DISALLOW_COPY_AND_ASSIGN(PopulateThread);
  };

  // A single benchmark main thread
  class SIGMOD2012BenchmarkThread: public BenchmarkThread{
   public:
    // Creates a new benchmark thread that works on the given index
    SIGMOD2012BenchmarkThread(Logger &logger, SIGMOD2012Properties &properties, SIGMOD2012IndexProperties &index,unsigned int seed);

    // Destructor
    ~SIGMOD2012BenchmarkThread();

    // The main function
    void Run();

    // Stops the benchmark thread
    void Stop(){run_=false;}

    // Returns the number of transactions that have been executed by this thread
    unsigned int tx_count() const { return tx_count_; };

    // Returns the number of successful transaction that have been executed by
    // this thread
    unsigned int tx_success_count() const { return tx_success_count_; };

    // Returns the number of deadlocks that occured while executing this
    // benchmark thread
    unsigned int deadlock_count() const { return deadlock_count_; };

    // Returns the number of operations executed by this thread
    unsigned int op_count() const { return range_count_+point_count_+insert_count_+update_count_+delete_count_;};

    // Returns the number of range queries (not operations) executed by this thread
    unsigned int range_queries() const {return range_queries_;}

    // Returns the number of range operations (including GetNext() calls)
    // executed by this thread
    unsigned int range_count() const {return range_count_;}

    // Returns the number of point queries executed by this thread
    unsigned int point_count() const {return point_count_;}

    // Returns the number of insert operations executed by this thread
    unsigned int insert_count() const {return insert_count_;}

    // Returns the number of update operations executed by this thread
    unsigned int update_count() const {return update_count_;}

    // Returns the number of delete operations executed by this thread
    unsigned int delete_count() const {return delete_count_;}

    // Returns the name of the index used by this thread
    std::string index_name() const {return index_.name();}
  private:
    // Resets all statistical values to its default values
    void ResetStatistics();

    // Frees all memory used by the given record
    static void ReleaseRecord(Record *record);

    // Serializes the given key and stores it at the given destination
    // (NOTE: SetKey does not allocate the necessary memory to store the
    //        serialized key.)
    static void SetKey(char* serialized, Key &destination);

    // The index to be used by this thread
    SIGMOD2012IndexProperties &index_;

    // The random number generator used by this thread
    RandomNumberGenerator *rng_;

    // A generator used select the type of each transaction
    IntegerGenerator* op_select_;

    // The private generators used to create new keys
    Generator** generators_;

    // The logger used by the thread
    Logger &logger_;

    // Whether the thread is running
    bool run_;

    // The number of transactions that have been executed by this thread
    unsigned int tx_count_;

    // The number of successful transaction that have been executed by this
    // thread
    unsigned int tx_success_count_;

    // The number of deadlocks that occured while executing this benchmark
    // thread
    unsigned int deadlock_count_;

    // The number of operations executed by this thread
    unsigned int op_count_;

    // The number of range queries (not operations) executed by this thread
    unsigned int range_queries_;

    // The number of range operations (including GetNext() calls) executed by
    // this thread
    unsigned int range_count_;

    // The number of point queries executed by this thread
    unsigned int point_count_;

    // The number of insert operations executed by this thread
    unsigned int insert_count_;

    // The number of update operations executed by this thread
    unsigned int update_count_;

    // The number of delete operations executed by this thread
    unsigned int delete_count_;

    DISALLOW_COPY_AND_ASSIGN(SIGMOD2012BenchmarkThread);
  };

  // A set of benchmark threads to be executed
  SIGMOD2012BenchmarkThread **threads;

  DISALLOW_COPY_AND_ASSIGN(SIGMOD2012BasicWorkload);
};

#endif // BENCHMARK_WORKLOADS_SIGMOD_2012_BASIC_WORKLOAD_H_
