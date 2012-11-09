# Copyright (c) 2012 TU Dresden - Database Technology Group
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Authors: Lukas M. Maas <Lukas_Michael.Maas@mailbox.tu-dresden.de>,
#          T. Kissinger <thomas.kissinger@tu-dresden.de>
#
# Current version: 1.01 (released February 21, 2012)
#
# Version history:
#   - 1.01 (February 21, 2012)
#     * Added options for Berkeley DB
#   - 1.0 Initial release (December 14, 2011).

# The reference implementation is based on Oracle's Berkeley DB.
# Under most linux distributions Berkeley DB is installed by default in /usr
# Therefore the following defines are not needed:
#BDBBASE =
#BDBLIB  =
#BDBINC  =
# Under OS X you may use the following defines:
#BDBBASE = /usr/local/BerkeleyDB.5.3
#BDBLIB  = -L$(BDBBASE)/lib
#BDBINC  = -I$(BDBBASE)/include

# The objects files that will be created for the reference implementation
OBJECTS = example/BDBImpl.o example/connection_manager.o example/index.o \
          example/iterator.o example/util.o example/transaction.o

# You may use the following defines to add custom include folders and libraries
IMPL=$(OBJECTS)
ADDINC=$(BDBINC)
ADDLIB=$(BDBLIB) -ldb_cxx

# Compiler flags
CC  = gcc
CXX = g++
CFLAGS=-O3 -fPIC -Wall -g -I. -I./include $(ADDINC) -D__STDC_LIMIT_MACROS
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lpthread $(ADDLIB)

# The programs that will be built
PROGRAMS=unittest sigmod-benchmark

# The name of the library that will be built
LIBRARY=contest

# The name of the library that will be produced by the 'basic' build target
BASIC=null

# Build all programs
all: $(PROGRAMS)

# Build the shared library
lib: $(IMPL)
	$(CXX) $(CXXFLAGS) -shared -Wl $(LDFLAGS) -o lib$(LIBRARY).so $(IMPL)

# Build targets for unittest and benchmark
UNITTESTO=unittests/main.o unittests/test_runner.o unittests/test_util.o unittests/tests.o unittests/util.o
BENCHMARKSRC=benchmark/main.cc benchmark/core/benchmark.cc benchmark/core/loggers/console_logger.cc benchmark/core/utils/thread.cc benchmark/core/utils/timer.cc benchmark/core/utils/argument_parser.cc benchmark/workloads/sigmod_2012_basic_workload.cc benchmark/workloads/sigmod_2012_properties.cc benchmark/core/importers/json_importer.cc


unittest: lib $(UNITTESTO)
	$(CXX) $(CXXFLAGS) -o unittest $(COMMON) $(UNITTESTO) ./lib$(LIBRARY).so

benchmark: lib
	$(CXX) $(CXXFLAGS) -I./benchmark -o sigmod-benchmark $(BENCHMARKSRC) -lpthread ./lib$(LIBRARY).so

basic: BasicImpl.o
	$(CXX) $(CXXFLAGS) -shared -W1 -o lib$(BASIC).so BasicImpl.o

# The following targets may be used to run the created executables
run-benchmark: benchmark
	./sigmod-benchmark workloads/base.workload

run-unittest: unittest
	./unittest

run: run-unittests run-benchmark

# Cleanup all files created during the build process
clean:
	$(RM) -R $(PROGRAMS) lib$(LIBRARY).so lib$(BASIC).so
	find . -name '*.o' -print | xargs rm -f

# Display a help page
help:
	@echo
	@echo "Build SIGMOD 2012 Programming Contest"
	@echo "============================================"
	@echo "build targets:"
	@echo "  basic             Build the basic (empty) implementation"
	@echo "  benchmark         Build the basic benchmark"
	@echo "  clean             Delete all files created during the build process"
	@echo "  help              Display this help"
	@echo "  lib               Build the library"
	@echo "  run-benchmark     Build and run the benchmark using the base workload"
	@echo "  run-unittest      Build and run the unit tests"
	@echo "  unittest          Build the unit tests"
	@echo

