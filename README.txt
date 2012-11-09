ACM SIGMOD 2012 Programming Contest
© 2011-2012 TU Dresden - Database Technology Group
Lukas M. Maas <Lukas_Michael.Maas@mailbox.tu-dresden.de>
T. Kissinger <Thomas.Kissinger@tu-dresden.de>

All of this product's content was created by the authors of this file.


=============================================

ACM SIGMOD 2012 Programming Contest

=============================================

This readme file contains information about using the contents of this archive
to build and test your contest submission.

The detailed task description of the contest can be found at:
http://wwwdb.inf.tu-dresden.de/sigmod2012contest/

----------------------------------------------
-                  Contents                  -
----------------------------------------------
Included in this release are the following files and directories:
  - README.txt                    This document
  - LICENSE.txt                   Software license
  - CHANGES.txt                   Changes since the last release
  - BasicImpl.c                   An empty implementation that just prints out
                                  the requested method
  - Makefile                      Makefile to compile the provided source code
  - include/contest_interface.h   The API that participants of the contest must
                                  implement
  - common/                       Contains source code that is used by multiple
                                  build targets
  - benchmark/                    Contains the benchmark source code
  - unittests/                    Contains source code that is only used to
                                  execute unit tests for the API defined in
                                  include/contest_interface.h
  - example/                      Contains the example implementation which uses
                                  Oracle's Berkeley DB
  - workloads/                    Contains some workload definitions that may
                                  be used with the benchmark


----------------------------------------------
-               Getting started              -
----------------------------------------------
The interface that participants of this year's SIGMOD Programming Contest must
implement is defined inside include/contest_interface.h

The example implementation uses Oracle's Berkeley DB
(http://www.oracle.com/technology/products/berkeley-db).
This means that in order to compile the example implementation you must first
have Berkeley DB installed. We have tested the code with version 5.1
(Under Debian you may use the following package: libdb5.1++) and assumed that
the Berkeley DB installation location is /usr/. If you choose to install it
somewhere else, then you will need to change the BDBBASE inside the makefile.

To build the example implementation on a Mac OS X machine you may also need to
modify the installation location of Berkeley DB inside the makefile (see the
makefile for details).


The name of the library that will be created using the provided makefile is
  
  libcontest.so

If you prefer another name you can change it by editing the LIBRARY variable
inside the makefile.


All code will compile and run on most UNIX-compatible systems using the GNU
toolchain (including Linux and OS X).

For a list of all build targets available use the 'make help' command.


----------------------------------------------
-         Testing your implementation        -
----------------------------------------------
We provide a set of correctness tests that may be used to ensure that your
implementation handles common tasks in the right way.

You can compile the unit tests by using the unittest build target:

  make unittest

Typing:

  make run-unittest

will build and also run the unit tests.

A configurable benchmark can be build using the following command:

  make benchmark

You can run the benchmark by typing:
  
  sigmod-benchmark <workload-file>

For more information about how to use or configure the benchmark, please use
the integrated help command:
  
  sigmod-benchmark --help


The workload that drives the leaderboard is defined inside the
'base.workload' file that is located inside the workloads/ directory.


You may use
  
  make run-benchmark

to build and also run the benchmark using the base configuration
(described above).

----------------------------------------------
-               Workload files               -
----------------------------------------------

The benchmark uses a json-based file format to specify the workload that will
be executed.

The following generator types (and parameter) are supported:

  For SHORT and INT:
    NORMAL:
      "mean"               - The mean of the normal distributon
      "standard deviation" - The distribution's standard deviation

    UNIFORM:
      "min"                - The lower bound of the range of generated values
      "max"                - The upper bound of the range of generated values

    CONSTANT:
      "value"              - The constant value to be generated

    ZIPF:
      "min"                - The lower bound of the range of generated values
      "max"                - The upper bound of the range of generated values
      "theta"              - The zipfian constant

    SEQUENCE:
      "min"                - The start value of the sequence
      "step"               - The step of the sequence


  For VARCHAR:
    CONSTANT:
      "value"              - The constant value to be generated

    FIXED_LENGTH:
      "length"             - The length of each generated string


Workload files may contain C++-style comments (i.e. single-line comments that
start with '//').

For more information please refer to the workload files located in workloads/.
