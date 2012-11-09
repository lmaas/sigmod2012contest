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

#include <cstdlib>
#include <iostream>

#include "core/benchmark.h"
#include "core/loggers/console_logger.h"
#include "core/utils/argument_parser.h"

#include "version.h"
#include "workloads/sigmod_2012_basic_workload.h"

// The default seed that will be used if no specific seed has been specified
#define DEFAULT_SEED 1

// The default number of threads that will be used if the thread count has not
// been specified
#define DEFAULT_THREAD_COUNT (sysconf(_SC_NPROCESSORS_ONLN))

// The default amount of seconds to warmup
#define WARMUP "10"

// The default amount of seconds to measure
#define DURATION "30"

// Print the program version
static void printVersion(){
  std::cout << "Version: " VERSION_STRING " "
               "(API Version: " API_VERSION ")"
            << std::endl;
}

int main (int argc, const char * argv[]) {
  // Create and configure a new command line argument parser
  ArgumentParser parser;
  parser.prog(argv[0]).description(PRODUCT_NAME "\n" COPYRIGHT)
        .epilog("For more information visit: "  CONTEST_URL);
	parser.add_argument("<filename>").required(true)
        .help("The workload specification file");
  parser.add_argument("--version").help("Display version information");
  parser.add_argument("-v","--verbose").help("Enable verbose output");
  parser.add_argument("--quiet").help("Disable all logging");
  parser.add_argument("--thread-count").nargs(1).metavar("<value>")
        .default_value(lexical_cast(DEFAULT_THREAD_COUNT))
        .help("The number of threads to be used to run the benchmark");
  parser.add_argument("-s","--seed").nargs(1).metavar("<value>")
        .default_value(lexical_cast(DEFAULT_SEED))
        .help("The seed used to initialize the random number generator");
  parser.add_argument("--warm-up").nargs(1).metavar("<seconds>")
        .default_value(WARMUP).help("The duration of the warm-up period");
  parser.add_argument("--duration").nargs(1).metavar("<seconds>")
        .default_value(DURATION).help("The duration of the measurement period");
  parser.add_argument("--extensive-stats").help("Display detailed statistics");
  parser.add_argument("--configuration-details")
        .help("Display configuration details before running the benchmark");

  // Parse the command line parameters
  if(!parser.parse_args(argc, argv))
    return 1;

  // Display version information
  if(parser.is_set("--version")){
    printVersion();
    return 1;
  }

  if(!parser.valid()){
    parser.print_usage();
    return 1;
  }

  // Configure logger
  ConsoleLogger logger;
  logger.verbose(parser.is_set("--verbose"));
  logger.quiet(parser.is_set("--quiet"));

  // Print heading
  logger.Info(PRODUCT_NAME);
  logger.Info("---------------------------------------------");

  // Initialize the workload
  Workload *workload = new SIGMOD2012BasicWorkload(logger);

  // Create the benchmark
  Benchmark benchmark(*workload,logger);

  // Build the properties set
  BenchmarkProperties props;
  props.seed( parser.get_value("--seed")->toInt());
  props.warmup_time(parser.get_value("--warm-up")->toInt());
  props.measurement_time(parser.get_value("--duration")->toInt());
  props.thread_count(parser.get_value("--thread-count")->toInt());
  props.Set("workload-file", parser.get_value("<filename>")->get());
  props.Set("extensive-stats",
            parser.is_set("--extensive-stats")?"true":"false");

  // If necessary, print configuration details
  if(parser.is_set("--configuration-details")){
    logger.AddSection("Configuration","Printing configuration details");
    logger.Info("Seed         :\t"+lexical_cast(props.seed()));
    logger.Info("Warm-up      :\t"+lexical_cast(props.warmup_time()));
    logger.Info("Duration     :\t"+lexical_cast(props.measurement_time()));
    logger.Info("Thread Count :\t"+lexical_cast(props.thread_count()));
    logger.CloseSection();
  }

  // Run the benchmark
  benchmark.Init(props);
  benchmark.WarmupAndRun();
  benchmark.Close();

  return 0;
}
