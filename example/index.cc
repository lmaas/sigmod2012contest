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

#include <stdint.h>
#include <cstdlib>
#include <string.h>
#include <db_cxx.h>

#include "connection_manager.h"
#include "index.h"
#include "iterator.h"
#include "transaction.h"
#include "util.h"

// Constructor fopr Index
Index::Index(const char* name){
  name_ = name;
  closed_ = true;
}

// Destructor for Index
Index::~Index(){
  Close();
}

// Opens an index
ErrorCode Index::Open(const char* name, Index** index){
  *index = new Index(name);
  
  // Try to get the structure of the requested index
  if(!((*index)->schema_ = IndexManager::getInstance().Find(name)))
    return kErrorUnknownIndex;
  
  // Creat a new db handle
  (*index)->env_ = ConnectionManager::getInstance().env();
  (*index)->db_ = new Db((*index)->env_, 0);
  
  // Initialize the structure
  (*index)->schema_->RegisterHandle(*index);
  
  
  // Allow duplicates for this db instance
  (*index)->db_->set_flags(DB_DUP);
  
  // Set the compare function for the b-tree
  (*index)->db_->set_bt_compare(&keycmp);
  
  // And finally open the index
  (*index)->db_->open(NULL,                       // Transaction pointer
                      NULL, 					            // File name (NULL, because we
                                                  // want to load from main memory)
                      (*index)->name_,				 	  // Logical db name (i.e. the index name)
                      DB_BTREE,				            // Database type (we use b-tree)
                      DB_THREAD | DB_AUTO_COMMIT, // Open flags
                      0                           // File mode (defaults)
                      );
  
  // The index was successfully opened
  (*index)->closed_ = false;
  
  return kOk;
}

// Close the index
void Index::Close(){
  lock(mutex_){
    if(!closed_)
      closed_ = true;
    else
      return;
  }
  
  // Close all iterators that use this index handle
  std::set<Iterator*>::iterator it;
  while((it=iterators_.begin()) != iterators_.end()){
    (*it)->Close();
  }
  
  // Close the database handle
  if(db_ != NULL){
    db_->close(0);
    db_ = NULL;
  }
  
  if(schema_ != NULL){
    schema_->UnregisterHandle(this);
  }
  
  closed_ = true;
}

// Create a cursor to access the data inside this index
Dbc* Index::Cursor(Transaction* tx){
  Dbc* cursor;
  
  // Create a new cursor with isolation level read committed
  db_->cursor((tx?tx->tid:NULL), &cursor, DB_READ_COMMITTED);
  
  return cursor;
}

// Converts the given Dbt to a key of this index
Key Index::GetKey(const Dbt *bdb_key){
  return schema_->GetKey(bdb_key);
}

// Converts the given Key of this index into a Dbt object
Dbt* Index::GetBDBKey(Key key, bool max){
  return schema_->GetBDBKey(key, max);
}

// Insert the given record into the index
ErrorCode Index::Insert(Transaction *tx, Record *record){
  // Convert the payload
  Dbt value;
  value.set_data(record->payload.data);
  value.set_size(record->payload.size);
  value.set_flags(0);
	
  // If the insert occured inside a larger transaction, then add
  // the parent transaction to the set of open transactions
  if(tx != NULL){
    if(!tx->UseIndex(schema_))
      return kErrorUnknownIndex;
  }
  
  // Perform the database put
  Dbt* bdbkey = GetBDBKey(record->key);
  bdbkey->set_flags(0);
  ErrorCode res = kOk;
  if (db_->put((tx?tx->tid:NULL), bdbkey, &value, 0) != 0){
	  res = kErrorGenericFailure;
  }
  
  free(bdbkey->get_data());
  free(bdbkey);
  return res;
}

// Update the given record with the given payload
ErrorCode Index::Update(Transaction *tx, Record *record, Block *payload, uint8_t flags){
  ErrorCode result = kOk;
  DbTxn* tid;
  Dbc* cursor;
  
  bool ignore_payload = (flags & kIgnorePayload);
  
  // Convert the record 
  Dbt key = *GetBDBKey(record->key);
  Dbt okey = key;
  void* pkey = key.get_data();
  
  Dbt value;
  // If necessary set the value to match
  if(!ignore_payload){
    value.set_data(record->payload.data);
    value.set_size(record->payload.size);
  }
  
  Dbt original_value = value;
  
  // Convert the new payload
  Dbt new_value;
  new_value.set_data(payload->data);
  new_value.set_size(payload->size);
  
  // Create a serializable nested transaction (to prevent the transaction
  // from seeing data that has been inserted after the transaction begun)
  env_->txn_begin((tx?tx->tid:NULL), &tid,  0 );
  
  // Start writing on the index
  if(schema_->BeginTransaction(tid)){
    
    // Create a cursor for this index
    db_->cursor(tid, &cursor, 0);
    
    // Retrieve the first record
    int err = 0;
    if(ignore_payload){
      err = cursor->get(&key, &value, DB_SET_RANGE);
      
      // Check that the found record has a valid key
      if(KeyCmp(schema_,&okey,&key))
        err = DB_NOTFOUND;
    } else {
      // Try an exact match
      err = cursor->get(&key, &value, DB_GET_BOTH);
    }
    if(err == 0){
      // Update the record using the new payload
      if((err = cursor->put(&key, &new_value, DB_CURRENT)) == 0){
        // If the update occured inside a larger transaction, then add
        // the parent transaction to the set of open transactions
        if(tx != NULL){
          tx->UseIndex(schema_);
        }
        
        // If necessary update duplicates
        if(flags & kMatchDuplicates){
          // Find next duplicate
          while((err = cursor->get(&key, &value, DB_NEXT_DUP)) == 0){
            if(ignore_payload || ((value.get_size() == record->payload.size)
                                  && (memcmp(value.get_data(),
                                             record->payload.data,
                                             record->payload.size) == 0))){
              // And update it
              if ((err = cursor->put(&key, &new_value, DB_CURRENT)) != 0)
                break;
            }
          }
          
          if(err != DB_NOTFOUND)
            result = kErrorNotFound;
        }
      } else {
        if(err == DB_NOTFOUND)
          result = kErrorNotFound;
        else
          result = kErrorGenericFailure;
      }
    } else {
      if(err == DB_NOTFOUND)
        result = kErrorNotFound;
      else
        result = kErrorGenericFailure;
    }
  } else {
    // Abort the new transaction
    delete [] (char*) pkey;
    tid->abort();
    return kErrorUnknownIndex;
  }
  
  // Commit the nested transaction
  tid->commit(0);
  delete [] (char*) pkey;
  // We finished writing on the index
  schema_->EndTransaction(tid);
  
  return result;
}

// Delete the given record
ErrorCode Index::Delete(Transaction *tx, Record *record, uint8_t flags){
  ErrorCode result = kOk;
  DbTxn* tid;
  Dbc* cursor;
  
  bool ignore_payload = (flags & kIgnorePayload);
  
  // Convert the record 
  Dbt key = *GetBDBKey(record->key);
  Dbt okey = key;
  void* pkey = key.get_data();
  
  Dbt value;
  // If necessary set the value to match
  if(!ignore_payload){
    value.set_data(record->payload.data);
    value.set_size(record->payload.size);
  }
  
  // Create a serializable nested transaction (to prevent the transaction
  // from seeing data that has been inserted after the transaction begun)
  env_->txn_begin((tx?tx->tid:NULL), &tid,  0 );
  
  // Start writing on the index
  if(schema_->BeginTransaction(tid)){
    
    // Create a cursor for this index
    db_->cursor(tid, &cursor, DB_READ_COMMITTED);
    
    // Retrieve the first record
    int err = 0;
    if(ignore_payload){
      err = cursor->get(&key, &value, DB_SET_RANGE);
      
      // Check that the found record has a valid key
      if(KeyCmp(schema_,&okey,&key))
        err = DB_NOTFOUND;
      
    } else {
      // Try an exact match
      err = cursor->get(&key, &value, DB_GET_BOTH);
    }
    
    if(err == 0){
      // Delete the record
      if((err = cursor->del(0)) == 0){
        // If the deletion occured inside a larger transaction, then add
        // the parent transaction to the set of open transactions
        if(tx != NULL){
          tx->UseIndex(schema_);
        }
        
        // If necessary delete duplicates
        if(flags & kMatchDuplicates){
          // Find next duplicate
          while((err = cursor->get(&key, &value, DB_NEXT_DUP)) == 0){
            if(ignore_payload || ((value.get_size() == record->payload.size)
                                  && (memcmp(value.get_data(),
                                             record->payload.data,
                                             record->payload.size) == 0))){
              // And delete it
              if ((err = cursor->del(0)) != 0)
                break;
            }
          }
          
          if(err != DB_NOTFOUND)
            result = kErrorGenericFailure;
        }
      } else {
        if(err == DB_NOTFOUND)
          result = kErrorNotFound;
        else
          result = kErrorGenericFailure;
      }
    } else {
      if(err == DB_NOTFOUND)
        result = kErrorNotFound;
      else
        result = kErrorGenericFailure;
    }
  } else {
    // Abort the new transaction
    delete [] (char*) pkey;
    tid->abort();
    
    return kErrorUnknownIndex;
  }
  
  // Commit the nested transaction
  delete [] (char*) pkey;
  tid->commit(0);
  
  // We finished writing on the index
  schema_->EndTransaction(tid);
  
  return result;
}

// Checks whether the given record is compatible with this index
bool Index::Compatible(Record *record){
  if((record == NULL) || closed_)
    return false;
  
  return Compatible(record->key);
}

// Checks whether the given key is compatible with this index
bool Index::Compatible(Key &key){
  if(closed_)
    return false;
  
  return schema_->Compatible(key);
}

// Register a new iterator handle
bool Index::RegisterIterator(Iterator* iterator){
  lock(mutex_){
    if(closed_)
      return false;
    if(iterator != NULL)
      iterators_.insert(iterator);
  }
  return true;
}

// Unregister an iterator
void Index::UnregisterIterator(Iterator* iterator){
  lock(mutex_){
    iterators_.erase(iterator);
  }
}

// Constructor for IndexSchema
IndexSchema::IndexSchema(uint8_t attribute_count, KeyType type){
  attribute_count_ = attribute_count;
  type_ = (AttributeType*) malloc(attribute_count*sizeof(AttributeType));
  size_ = 0;
  read_only_=false;
  
  // Build the size and copy the type array
  for(int i = 0; i < attribute_count; i++){
    if(type[i] == kShort)
      size_ += 4;
    else if(type[i] == kInt)
      size_ += 8;
    else
      size_ += MAX_VARCHAR_LENGTH+1;
    
    type_[i] = type[i];
  }
}

// Destructor for IndexSchema
IndexSchema::~IndexSchema(){
  // Close all open Handles of this structure
  CloseHandles();
  delete[] type_;
}

// Create a new index schema
void IndexSchema::Create(const char* name, uint8_t column_count, KeyType types){
  Db db(ConnectionManager::getInstance().env(),0);
  
  // Allow duplicates for this index
  db.set_flags(DB_DUP);
  
  // Create the new index
  db.open(NULL, 		        // Transaction pointer
          NULL, 					      // File name (NULL, because we want the index to be in-memory)
          name,					        // Logical db name
          DB_BTREE,				      // Database type (we use b-tree)
          DB_CREATE | DB_EXCL,  // Open flags
          0                     // File mode (defaults)
          );
  
  // Insert new Index into the index map
  IndexManager::getInstance().Insert(name,new IndexSchema(column_count, types));
}

// Convert the given Dbt to a key of this index
Key IndexSchema::GetKey(const Dbt *bdb_key){
  // Create the new key object
  Key key;
  key.value = (Attribute**) malloc(attribute_count_*sizeof(Attribute*));
  key.attribute_count = attribute_count_;
  
  int offset = 8, size = 0;
  for(int i = 0; i < attribute_count_; i++){
    // Determine the attribute size
    if(type_[i] == kShort)
      size = 4;
    else if(type_[i] == kInt)
      size = 8;
    else
      size = MAX_VARCHAR_LENGTH+1;
    
    // Create the attribute
    key.value[i] = (Attribute*) malloc(sizeof(Attribute));
    key.value[i]->type = type_[i];
    memcpy(&(key.value[i]->char_value),((char*)bdb_key->get_data())+offset,size);
    
    // Set the offset to the first byte after the attribute
    offset+=size;
  }
  
  return key;
}

// Convert the given Key of this index into a Dbt object
Dbt* IndexSchema::GetBDBKey(Key key, bool max){
  // Allocate the necessary memory
  char* data = new char[size_+8];
  
  IndexSchema* is = this;
  memcpy(data,&is,sizeof(this));
  
  int offset = 8, size = 0;
  
  // If the key value is NULL we have to set a wildcard
  // depending on if it is a maximum or minimum key
  int32_t short_wildcard = (max?INT32_MAX:INT32_MIN);
  int64_t int_wildcard = (max?INT64_MAX:INT64_MIN);

  for(int i = 0; i < attribute_count_; i++){
    // Determine the attribute size
    if(type_[i] == kShort){
      size = 4;
      if(key.value[i] == NULL)
        memcpy(data+offset,&short_wildcard,size);
    }else if(type_[i] == kInt){
      size = 8;
      if(key.value[i] == NULL)
        memcpy(data+offset,&int_wildcard,size);

    }else{
      size = MAX_VARCHAR_LENGTH+1;
      if(key.value[i] == NULL){
         memset(data+offset,127,MAX_VARCHAR_LENGTH);
         if(max)
          memset(data+offset+MAX_VARCHAR_LENGTH,'\0',1);
         else
          memset(data+offset,'\0',1);
      }
    }
    if(key.value[i] != NULL){
      // Copy the data into the buffer
      memcpy(data+offset,&(key.value[i]->char_value),size);
    }
    // Set the offset to the first byte after the attribute
    offset += size;
  }

  // Return the newly created Dbt object
  Dbt* rt = new Dbt(data,size_+8);
  return rt;
}

// Checks whether the given key is compatible with this schema
bool IndexSchema::Compatible(Key &key){
  if(key.attribute_count != attribute_count_)
    return false;
  
  for(int i = 0; i < key.attribute_count; i++){
    if(key.value[i]){
      if(key.value[i]->type != type_[i])
        return false;
    }
  }

  return true;
}

// Register a new index handle
void IndexSchema::RegisterHandle(Index* handle){
  lock(mutex_){
    handles_.insert(handle);
  }
}

// Unregister an index handle
void IndexSchema::UnregisterHandle(Index* handle){
  lock(mutex_){
    handles_.erase(handle);
  }
}

// Close all registered handles
void IndexSchema::CloseHandles(){
  std::set<Index*>::iterator it;
  it=handles_.begin();
  while(it != handles_.end()){
    // Closs Index handle
    if(*it != NULL)
      (*it)->Close();
    it++;
    
  }
}

// Start a new modifying transaction on this index
// This function returns false if the index is read-only (otherwise, true)
bool IndexSchema::BeginTransaction(DbTxn *tx){
  lock(transaction_mutex_){
    if(read_only_)
      return false;
    
    transactions_.insert(tx);
  }
  return true;
}

// End a modifying transaction on this index
void IndexSchema::EndTransaction(DbTxn *tx){
  lock(transaction_mutex_){
    transactions_.erase(tx);
  }
}

// Try to make this index read-only
//
// This function will return false if currently unresolved transactions have
// written to this index.
bool IndexSchema::MakeReadOnly(){
  lock(transaction_mutex_){
    if(transactions_.size() > 0){
      return false;
    }
    read_only_ = true;
  }
  return true;
}

pthread_once_t IndexManager::once_ = PTHREAD_ONCE_INIT;
IndexManager* IndexManager::instance_;

// Return the singleton instance of IndexManager
IndexManager& IndexManager::getInstance(){
  pthread_once(&once_, &Initialize);
  return *instance_;
}

// Returns the index schema with the given name
IndexSchema* IndexManager::Find(std::string name){
  lock(mutex_){
    std::map<std::string,IndexSchema*>::iterator it = indices_.find(name);
    
    if( it != indices_.end())
    {
      return it->second;
    }
  }
  return NULL;
}

// Insert a index schema (may overwrite existing entry)
void IndexManager::Insert(std::string name, IndexSchema* structure){
  lock(mutex_){
    indices_[name] = structure;
  }
}

// Search and delete the index structure with the given name
ErrorCode IndexManager::Remove(std::string name){
  lock(mutex_){
    std::map<std::string,IndexSchema*>::iterator it = indices_.find(name);

    if( it != indices_.end()){
      // Try to delete the index structure
      if(it->second->MakeReadOnly()){
        delete it->second;
        indices_.erase(it);
      } else {
        //std::cout<<"kErrorOpenTransaction"<<std::flush;
        return kErrorOpenTransactions;
      }
    } else {
      //std::cout<<"kErrorUnknownIndex"<<std::flush;
      return kErrorUnknownIndex;
    }
  }
  return kOk;
}

// Initialize the singleton instance of IndexManager
void IndexManager::Initialize(){
  instance_ = new IndexManager();
  atexit(&Destroy);
}

// Destroy the singleton instance of IndexManager
void IndexManager::Destroy(){
  delete instance_;
  instance_ = 0;
}
