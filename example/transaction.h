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

#ifndef _BDBIMPL_TRANSACTION_H_
#define _BDBIMPL_TRANSACTION_H_

#include <set>
#include <common/macros.h>

#include "index.h"

// Represents a transaction
class Transaction {
 public:
  // Constructor
  Transaction();
  
  // Destructor
  ~Transaction();
  
  // Begin this transaction
  void Begin();
  
  // Abort this transaction
  void Abort();
  
  // Commit this transaction
  void Commit();
  
  // Use a given index schema with this transaction
  bool UseIndex(IndexSchema *structure);
  
 private:
  // Close the transaction
  void CloseTransaction();
  
  // A set of indices that have been modified using this transaction
  std::set<IndexSchema*> indices_;
  
  // The Berkeley DB transaction handle
  DbTxn *tid;
  
  // Whether the transaction has been resolved
  bool finished_;
  
  friend class Index;
  
  DISALLOW_COPY_AND_ASSIGN(Transaction);
};

#endif // _BDBIMPL_TRANSACTION_H_