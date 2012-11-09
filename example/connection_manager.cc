/*
 Copyright (c) 2012 TU Dresden - Database Technology Group
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 Author: Lukas M. Maas <Lukas_Michael.Maas@mailbox.tu-dresden.de>
 
 Current version: 1.0 (released February 21, 2012)
 
 Version history:
 - 1.0 Initial release (February 21, 2012) 
 */

#include <db_cxx.h>
#include <stdio.h>
#include <cstdlib>
#include "connection_manager.h"

pthread_once_t ConnectionManager::once_ = PTHREAD_ONCE_INIT;
ConnectionManager* ConnectionManager::instance_;


// Constructur for Connectionmanager
ConnectionManager::ConnectionManager(){
  // Open flags of the environment
  int envFlags =
    DB_CREATE     |  // Create the environment if it does not exist
    DB_RECOVER    |  // Run normal recovery.
    DB_INIT_LOCK  |  // Initialize the locking subsystem
    DB_INIT_LOG   |  // Initialize the logging subsystem
    DB_INIT_TXN   |  // Initialize the transactional subsystem. This
                     // also turns on logging.
    DB_INIT_MPOOL |  // Initialize the memory pool (in-memory cache)
    DB_PRIVATE    |  // Region files are not backed by the filesystem.
                     // Instead, they are backed by heap memory.
   	DB_AUTO_COMMIT | // Enable auto-commit mode, when no transaction is specified
    DB_THREAD;       // Cause the environment to be free-threaded
  
	// Create the environment
	env_ = new DbEnv(0);
  
	// Specify in-memory logging
	env_->log_set_config(DB_LOG_IN_MEMORY, 1);
  
  // Specify the size of the in-memory log buffer.
  env_->set_lg_bsize(((size_t) 1 << 20) * 25 );
  
  // Specify the size of the in-memory cache
  env_->set_cachesize(4, 0, 1);

  // Indicate that we want the db to internally perform deadlock
  // detection. Also indicate that the transaction with
  // the fewest number of write locks will receive the
  // deadlock notification in the event of a deadlock.
  env_->set_lk_detect(DB_LOCK_MINWRITE);
  
  // Specify a log file to output error messages
  env_->set_errfile(fopen ("bdb.log" , "w"));

  // Open the environment
  env_->open(NULL, envFlags, 0);
}

// Destructor for ConnectionManager
ConnectionManager::~ConnectionManager(){
  try {
    // Close our environment if it was opened.
    if (env_ != NULL)
      env_->close(0);
  } catch(DbException &e) {
    std::cerr << "Error closing database environment." << std::endl;
    std::cerr << e.what() << std::endl;
	}
}

// Return the singleton instance of ConnectionManager
ConnectionManager& ConnectionManager::getInstance(){
  pthread_once(&once_, &Initialize);
  return *instance_;
}

// Initialize the singleton instance
void ConnectionManager::Initialize(){
  instance_ = new ConnectionManager();
  atexit(&Destroy);
}

// Destroy the singleton instance
void ConnectionManager::Destroy(){
  delete instance_;
  instance_ = 0;
}