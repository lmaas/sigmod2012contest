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

#ifndef _BDBIMPL_ITERATOR_H_
#define _BDBIMPL_ITERATOR_H_

#include "index.h"

class Dbc;
class Dbt;

// Represents an iterator
class Iterator {
 public:
  // Constructor
  Iterator();
  
  // Destructor;
  ~Iterator();
  
  // Initialize the iterator
  void Init(Transaction* tx, Index* idx, Key min_keys, Key max_keys);
  
  // Close the iterator
  void Close();

  // Move the iterator to the next record
  bool Next();

  // Return whether the iterator has been closed
  bool closed() const { return closed_; };

  // Return whether the iterator has exceeded its range
  bool end() const { return end_; };

  // Return the record to which the iterator refers
  Record* value();
    
 private:
  // Close the Berkeley DB Cursor
  void CloseCursor();
  
  // Mark the iterator as ended
  void SetEnded();

  // The current key to which the iterator refers
  Dbt *key_;

  // The current value to which the iterator refers
  Dbt *value_;

  // The maximum key that limits the range of this iterator
  Dbt *max_key_;

  // The minimum key for this iterator
  Dbt *min_key_;
  
  // The index which is iterated over
  Index *index_;
  
  // The index schema of that index
  IndexSchema *is_;

  // The used Berkeley DB cursor
  Dbc *cursor_;

  // Whether the iterator has been closed
  bool closed_;

  // Whether the iterator has exceeded its range
  bool end_;

  // Whether the iterator is initialized
  bool initialized_;
  
  //
  bool key_set_;

  DISALLOW_COPY_AND_ASSIGN(Iterator);
};

#endif // _BDBIMPL_ITERATOR_H_