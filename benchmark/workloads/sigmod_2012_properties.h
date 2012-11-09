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

#ifndef BENCHMARK_WORKLOADS_SIGMOD_2012_PROPERTIES_H_
#define BENCHMARK_WORKLOADS_SIGMOD_2012_PROPERTIES_H_

#include <vector>
#include <cassert>

#include "contest_interface.h"
#include "core/cached_key_store.h"
#include "core/logger.h"
#include "core/utils/lexical_cast.h"

class SIGMOD2012IndexProperties;
class Generator;

// Defines properties for the SIGMOD 2012 Programming Contest workload
class SIGMOD2012Properties{
 public:
  // Creates a new properties object for the SIGMOD 2012 Programming Contest
  // workload
  SIGMOD2012Properties():range_portion_(0),point_portion_(0),update_portion_(0),
  insert_portion_(0),delete_portion_(0),extensive_stats_(false){}
  
  // Loads the properties from a file
  static SIGMOD2012Properties *LoadFromFile(Logger &logger,
                                            std::string filename,
                                            unsigned int seed);
  
  // Adds a new index to the properties object
  void AddIndex(SIGMOD2012IndexProperties *index){indices_.push_back(index);}
  
  // Returns the index at the given index
  SIGMOD2012IndexProperties &GetIndex(unsigned int index){
    return *indices_[index];
  }
  
  // Returns the portion of range queries
  int range_portion() const {return range_portion_;}
  
  // Returns the portion of point queries
  int point_portion() const {return point_portion_;}
  
  // Returns the portion of update operations
  int update_portion() const {return update_portion_;}
  
  // Returns the portion of insert operations
  int insert_portion() const {return insert_portion_;}
  
  // Returns the portion of delete operations
  int delete_portion() const {return delete_portion_;}
  
  // Returns whether extensive stats are enabled
  bool extensive_stats() const{return extensive_stats_;}
  
  // Returns the number of indices inside this property object
  size_t index_count() const {return indices_.size();}
  
  // Sets the portion of range queries
  void range_portion(int range_portion){range_portion_=range_portion;}
  
  // Sets the portion of point queries
  void point_portion(int point_portion){point_portion_=point_portion;}
  
  // Sets the portion of update operations
  void update_portion(int update_portion){update_portion_=update_portion;}
  
  // Sets the portion of insert operations
  void insert_portion(int insert_portion){insert_portion_=insert_portion;}
  
  // Sets the portion of delete operations
  void delete_portion(int delete_portion){delete_portion_=delete_portion;}
  
  // Sets whether extensive stats should be used
  void extensive_stats(bool extensive_stats){extensive_stats_=extensive_stats;}
  
 private:
  // A list of all indices to be used by the benchmark
  std::vector<SIGMOD2012IndexProperties*> indices_;
  
  // The portion of range queries
  int range_portion_;
  
  // The portion of point queries
  int point_portion_;
  
  // The portion of update operations
  int update_portion_;
  
  // The portion of insert operations
  int insert_portion_;
  
  // The portion of delete operations
  int delete_portion_;
  
  // Whether extensive statistics are enabled
  bool extensive_stats_;
};

// Defines properties for indices used by the SIGMOD 2012 Programming Contest
// workload
class SIGMOD2012IndexProperties{
 public:
  // Creates a new index properties object
  SIGMOD2012IndexProperties(const char* name, unsigned int dimensions,
                            AttributeType* types, Generator** generators,
                            unsigned int size, unsigned int payload_size,
                            float capacity_factor):
    name_(name),dimensions_(dimensions),types_(types),size_(size),
    payload_size_(payload_size),generators_(generators){
    
    // Get the record size
    unsigned int key_size = 0;
    for(unsigned int i = 0; i < dimensions_; i++){
      switch(types_[i]){
        case kShort:
          key_size += sizeof(int32_t);
          break;
        case kInt:
          key_size += sizeof(int64_t);
          break;
        case kVarchar:
          key_size += MAX_VARCHAR_LENGTH;
          break;
        default:
          assert(false);
      }
    }
    key_size_ = key_size;
    record_size_ = key_size_+payload_size_;
    populate_count_ = (size_/record_size_);
    keystore_capacity_ = (capacity_factor*populate_count_);
    keystore_ = new CachedKeyStore<char*>(keystore_capacity_);
  }
  
  // Destroys an index properties object
  ~SIGMOD2012IndexProperties(){
    delete keystore_;
  };
  
  // Returns the name of the index
  const char* name() const {return name_;}
  
  // Returns the attribute types of the index
  AttributeType* types() const {return types_;}
  
  // Returns the number of dimensions of the index
  unsigned int dimensions() const {return dimensions_;}
  
  // Returns the size of the index in byte
  unsigned int size() const {return size_;}
  
  // Returns the payload size used for this index
  unsigned int payload_size() const {return payload_size_;}
  
  // Returns the array of generators used to generator records for this index
  Generator **generators() const{return generators_;}
  
  // Returns the keystore for this index
  CachedKeyStore<char*> &keystore() const {return *keystore_;}
  
  // Returns the size of a record for this index in byte
  unsigned int record_size() const {return record_size_;}
  
  // Returns the number of records used to populate this index
  unsigned int populate_count() const {return populate_count_;}
  
  // Returns the max. number of records that can be stored inside the keystore
  unsigned int keystore_capacity() const {return keystore_capacity_;}
  
  // Returns the size of a key for this index in byte
  unsigned int key_size() const {return key_size_;}
  
 private:
  // The name of the index
  const char* name_;
  
  // The number of dimensions of the index
  unsigned int dimensions_;

  // The attribute types of the index
  AttributeType* types_;
    
  // The size of the index in byte
  unsigned int size_;
  
  // The payload size used for this index
  unsigned int payload_size_;
  
  // An array of generators used to generator records for this index
  Generator **generators_;
  
  // The size of a key for this index in byte
  unsigned int key_size_;
  
  // The size of a record for this index in byte
  unsigned int record_size_;
  
  // The number of records used to populate this index
  unsigned int populate_count_;
  
  // The max. number of records that can be stored inside the keystore
  unsigned int keystore_capacity_;
  
  // The keystore for this index
  CachedKeyStore<char*> *keystore_;
};

#endif // BENCHMARK_WORKLOADS_SIGMOD_2012_PROPERTIES_H_
