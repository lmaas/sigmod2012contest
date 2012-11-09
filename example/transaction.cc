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

#include "connection_manager.h"
#include "transaction.h"

// Constructor
Transaction::Transaction(){
  finished_ = false;
  tid = NULL;
}

// Destructor
Transaction::~Transaction(){
  if(!finished_){
    CloseTransaction();
  }
}

// Begin the transaction
void Transaction::Begin(){
  // Start the new transaction with isolation level read committed
  ConnectionManager::getInstance().env()->txn_begin(NULL, &tid,
                                                    DB_READ_COMMITTED );
}

// Abort the transaction
void Transaction::Abort(){
  tid->abort();
  CloseTransaction();
}

// Commit the transaction
void Transaction::Commit(){
  tid->commit(0);
  CloseTransaction();
}

// Use a given index schema with this transaction
bool Transaction::UseIndex(IndexSchema *structure){
  std::pair<std::set<IndexSchema*>::iterator,bool> r = indices_.insert(structure);
  if(r.second)
    return structure->BeginTransaction(tid);
  return true;
}

// Close the transaction
//
// This unregisters this transaction on all used index schemas
void Transaction::CloseTransaction(){
  std::set<IndexSchema*>::iterator it;
  for(it=indices_.begin();it!=indices_.end();it++){
    (*it)->EndTransaction(tid);
  }
  finished_ = true;
}