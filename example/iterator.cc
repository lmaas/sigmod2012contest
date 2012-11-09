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

#include "iterator.h"
#include "util.h"

#include <db_cxx.h>

#include <stdint.h>
#include <cstdlib>
#include <string.h>


// Constructor
Iterator::Iterator(){
  index_ = NULL;
  closed_ = true;
  end_ = false;
  initialized_ = false;
  cursor_ = NULL;
  min_key_ = NULL;
  max_key_ = NULL;
  key_ = NULL;
  value_ = NULL;
  key_set_ = false;
}

// Destructor
Iterator::~Iterator(){
  Close();
  //std::cerr<<"destructor"<<std::endl;
  if(key_set_){
    free(key_->get_data());
  }
  delete key_;
  delete value_;
}


//Initialize the iterator to iterate over a given index.
void Iterator::Init(Transaction* tx, Index* idx, Key min_keys, Key max_keys){
  //std::cerr<<"init";
  if(!closed_)
    Close();
  
  index_ = idx;
  closed_ = false;
  end_ = false;
  initialized_ = false;
  is_ = index_->schema();
    
  // Initialize the min_key_
  min_key_ = index_->GetBDBKey(min_keys);
  
  // Initialize the max_key_
  max_key_ = index_->GetBDBKey(max_keys,true);

  // Initialize the cursor
  cursor_ = index_->Cursor(tx);
  
  // Start with the min_key and an empty value
  key_ = index_->GetBDBKey(min_keys);
  key_set_ = true;
  value_ = new Dbt();
  value_->set_size(0);


  // Register the new iterator
  index_->RegisterIterator(this);
}

// Close the iterator
void Iterator::Close(){
  // Close the cursor
  CloseCursor();
  
  if(closed_)
    return;
  
	closed_ = true;
  
  // Cleanup
  delete [] (char*) min_key_->get_data();
  delete min_key_;
  
  delete [] (char*) max_key_->get_data();
  delete max_key_;
  
  //std::cerr<<"free"<<"("<<this<<")";
  //delete key_;
  //delete value_;
  
  // Unregister the iterator
  index_->UnregisterIterator(this);

}

//
// Retrieves the next value from the iterator
//
// It essentially applies post-filtering to the result set, in order to find 
// the next valid key/value pair
// (i.e., it reads the next key/value pairs until a valid one has been found 
//  or the given range is exceeded).
//
// This approach can be very inefficient (especially for partial-match queries
// that do not restrict the dimension of the first key attribute)
//
bool Iterator::Next(){
  //std::cerr<<"Next"<<"("<<this<<")";
  int err;
  if(end_)
    return true;

  if(!initialized_){
    // Get the first key/value pair in the range of this iterator
    err = cursor_->get(key_, value_, DB_SET_RANGE);
    initialized_ = true;
  } else {
    // Move the cursor to the next key
    err = cursor_->get(key_, value_, DB_NEXT);
  }
  key_set_ = false;
  while(true){
    if(err == 0){
      
        // As the records are ordered starting with the first key attribute
        // we have exceeded our key range when the key of the retrieved
        // key is greater than the first attribute of the maximum key
        if(KeyCmp(is_, key_, max_key_) > 0){
          
          // Mark the iterator as ended
          SetEnded();
          
          return true;
        } else if((KeyCmp(is_,min_key_,key_,true) == 0) &&
                  (KeyCmp(is_,key_,max_key_,true) == 0)){

          // We've found a record
          return true;
        }
        // Move the cursor to the next key
        err = cursor_->get(key_, value_, DB_NEXT);
    } else {
      key_set_ = false;
      // Mark the iterator as ended because no new record could be fetched
      SetEnded();
      
      // If no record was found, than no error occured
      if(err != DB_NOTFOUND){
        // Otherwise, some error occured while fetching the record

        // Close the iterator
        Close();

        return false;
      }
      return true;
    }
  }
  return true;
}

// Return the record to which the iterator refers
Record* Iterator::value(){
  //std::cerr<<"Value"<<"("<<this<<")";
  Record *record = NULL;
  
  // If the iterator has already ended, don't return a record
  if(!end_){
    record = (Record*) malloc(sizeof(Record));
    // Set the key
    record->key = index_->GetKey(key_);
  
    // Set the payload
    record->payload.data = malloc(value_->get_size());
    memcpy(record->payload.data,value_->get_data(),value_->get_size());
    record->payload.size = value_->get_size();
  }
  return record;
}

// Close the Berkeley DB Cursor
//
// This function is needed to prevent closing of cursors that are already closed
void Iterator::CloseCursor(){
  //    std::cerr<<"closeCursor"<<"("<<this<<")";
  if(cursor_!=NULL){
    cursor_->close();
    cursor_ = NULL;
  }
}

// Mark the iterator as ended
void Iterator::SetEnded(){
  end_=true;
    
  // Close the current cursor, so that read locks are released
  CloseCursor();
}
