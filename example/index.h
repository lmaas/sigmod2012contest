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

#ifndef _BDBIMPL_INDEX_H_
#define _BDBIMPL_INDEX_H_

#include <map>
#include <set>
#include <string>

#include <contest_interface.h>
#include <common/macros.h>

#include "mutex.h"

class Db;
class Dbt;
class Dbc;
class IndexSchema;
class DbTxn;
class DbEnv;

// Class representing an index handle
class Index{
 public:    
  // Destructor
  ~Index();
  
  // Opens an index
  static ErrorCode Open(const char* name, Index** index);
  
  // Close this index
  void Close();
  
  // Create a cursor to access the data inside this index
  Dbc* Cursor(Transaction* tx);
  
  // Converts the given Dbt to a key of this index
  Key GetKey(const Dbt *bdb_key);
  
  // Converts the given Key of this index into a Dbt object
  Dbt* GetBDBKey(Key key, bool max = false);
  
  // Insert the given record into the index
  ErrorCode Insert(Transaction *tx, Record *record);
  
  // Update the given record with the given payload
  ErrorCode Update(Transaction *tx, Record *record, Block *payload, uint8_t flags);
  
  // Delete the given record
  ErrorCode Delete(Transaction *tx, Record *record, uint8_t flags);
  
  // Checks whether the given record is compatible with this index
  bool Compatible(Record *record);

  // Checks whether the given key is compatible with this index
  bool Compatible(Key &key);
  
  // Register a new iterator handle
  bool RegisterIterator(Iterator* iterator);
  
  // Unregister an iterator
  void UnregisterIterator(Iterator* iterator);
  
  // Return the name of this index
  const char* name() const { return name_; };
  
  // Return whether the index has been closed
  bool closed () const { return closed_; };
  
  // Get the schema of the referenced index
  IndexSchema* schema() { return schema_; };

 private:
  // Constructor
  Index(const char* name);
  
  // The Berkeley DB database handle
  Db *db_;
  
  // The Berkeley DB environment to use
  DbEnv *env_;

  // The name of this index
  const char* name_;

  // The structure of this index
  IndexSchema* schema_;

  // Whether the index has been closed
  bool closed_;

  // A set of all open iterators that use this index handle
  std::set<Iterator*> iterators_;
  
  // A mutex for protecting the insert and read operations on the iterator set
  Mutex mutex_;
  
  DISALLOW_COPY_AND_ASSIGN(Index);
};

// Represents a single or multicolumn index
class IndexSchema{
  public:
  // Constructor
  IndexSchema(uint8_t attribute_count, KeyType type);
  
  // Destructor
  ~IndexSchema();
  
  // Create a new index schema
  static void Create(const char* name, uint8_t column_count, KeyType types);
  
  // Convert the given Dbt to a key of this index
  Key GetKey(const Dbt *bdb_key);
  
  // Convert the given Key of this index into a Dbt object
  Dbt *GetBDBKey(Key key, bool max = false);
  
  // Checks whether the given key is compatible with this schema
  bool Compatible(Key &key);
  
  // Register a new index handle
  void RegisterHandle(Index* handle);
  
  // Unregister an index handle
  void UnregisterHandle(Index* handle);
  
  // Close all registered handles
  void CloseHandles();
  
  // Start a new modifying transaction on this index
  bool BeginTransaction(DbTxn *tx);

  // End a modifying transaction on this index
  void EndTransaction(DbTxn *tx);

  // Try to make this index read-only
  bool MakeReadOnly();

  uint8_t attribute_count() const { return attribute_count_; };
  AttributeType* type(){ return type_; };
  size_t size(){return size_;};
 
 private:
  // The number of attributes that form a key of this index
  uint8_t attribute_count_;
  
  // An array of attribute types
  AttributeType* type_;
  
  // The size of a key of this index in byte
  size_t size_;

  // Whether the index is readonly
  bool read_only_;

  // A set of all open handles of this index structure
  std::set<Index*> handles_;

  // A set of open transactions that have modified this index
  std::set<DbTxn*> transactions_;
  
  // A mutex for protecting the insert and read operations on the handle set
  Mutex mutex_;

  // A mutex for protecting the insert and read operations on the transaction set
  Mutex transaction_mutex_;

  DISALLOW_COPY_AND_ASSIGN(IndexSchema);
};


// Defines a simple index manager.
// 
// It is used to manage the schemas of the created indices.
// 
// IndexManager implements the Singleton Pattern. 
class IndexManager{
 public:
  // Return the singleton instance of IndexManager
  static IndexManager& getInstance();
  
  // Returns the index schema with the given name
  IndexSchema *Find(std::string name);    
  
  // Insert a index schema (may overwrite existing entry)
  void Insert(std::string name, IndexSchema* structure);

  // Search and delete the index structure with the given name
  ErrorCode Remove(std::string name);

  // Initialize the singleton instance
  static void Initialize();

  // Destroy the singleton instance 
  static void Destroy();

 private:
  // Private constructor (don't allow instanciation from outside)
  IndexManager(){};

  // Destructor
  ~IndexManager(){};

  // A map holding the structures of all indices
  std::map<std::string,IndexSchema*> indices_;

  // A mutex for protecting the insert and read operations
  // Note: It would be better to use some type of S/X locks
  // as multiple threads may read the map concurrently without
  // affecting each other (this map however is not performance-critical)
  Mutex mutex_;
  
  // The singleton instance of IndexManager
  static IndexManager* instance_;
  
  // A pthread once handle to guarantee that the singleton instance is 
  // only initialized once
  static pthread_once_t once_;

  DISALLOW_COPY_AND_ASSIGN(IndexManager);
};

#endif // _BDBIMPL_INDEX_H_
