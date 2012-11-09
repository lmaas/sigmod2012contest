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

#include <iostream>
#include <set>
#include <cstring>
#include <stdlib.h>

#include "core/generators/normal_generator.h"
#include "core/generators/uniform_generator.h"
#include "core/generators/discrete_generator.h"
#include "core/generators/fixed_length_string_generator.h"
#include "core/utils/rngs/fast_rand_number_generator.h"
#include "core/utils/lexical_cast.h"

#include "sigmod_2012_basic_workload.h"
#include "sigmod_2012_properties.h"


// Change these values to configure the behavior different transaction types
#define POINT_QUERIES_PER_TXN 20
#define UPDATES_PER_TXN 5
#define INSERTS_PER_TXN 5
#define DELETES_PER_TXN 5


// Runs the workload and return a statistics object
Statistics * SIGMOD2012BasicWorkload::Run(){
  // If necessary start benchmark threads
  if(!warmed_up_){
    for(unsigned int i = 0; i < thread_count_; i++){
      threads[i]->Start();
    }
  }

  // Run the measurement
  for(unsigned int i=0; i < thread_count_; i++){
    threads[i]->EnableMeasurement();
  }

  sleep(measurement_time_);

  for(unsigned int i = 0; i < thread_count_; i++){
    threads[i]->Stop();
  }

  for(unsigned int i = 0; i < thread_count_; i++){
    threads[i]->Join();
  }

  // Gather statistics
  unsigned int deadlock_count = 0;
  unsigned int tx_count = 0;
  unsigned int tx_success_count = 0;
  unsigned int op_count = 0;
  for(unsigned int i = 0; i < thread_count_; i++){
    deadlock_count += threads[i]->deadlock_count();
    tx_count += threads[i]->tx_count();
    tx_success_count += threads[i]->tx_success_count();
    op_count += threads[i]->op_count();
  }

  Statistics* statistics = new Statistics();
  statistics->Add("Operations/Second",
                  lexical_cast<float>(op_count/measurement_time_));
  statistics->Add("Number of Deadlocks",lexical_cast(deadlock_count));
  if(tx_count > 0){
    unsigned int failed = tx_count - tx_success_count;
    statistics->Add("Failed Transactions",
                    lexical_cast(failed)+" ("+
                    lexical_cast  (100*failed/tx_count)+"%)");
  } else {
    logger_.Error(lexical_cast(tx_count)+" transactions executed");
  }

  if(properties_->extensive_stats()){
    for(unsigned int i =0; i < thread_count_; i++){
      StatGroup group("Thread "+lexical_cast(i+1)+" ("+
                      threads[i]->index_name()+")");
      float avg_iterations = 0;
      if(threads[i]->range_queries() > 0)
        avg_iterations = (threads[i]->range_count()/threads[i]->range_queries());
      group.Add("Range Queries",lexical_cast(threads[i]->range_queries())+
                " (Avg. iterations: "+lexical_cast(avg_iterations)+")");
      group.Add("Point Queries",lexical_cast(threads[i]->point_count()));
      group.Add("Inserts",lexical_cast(threads[i]->insert_count()));
      group.Add("Updates",lexical_cast(threads[i]->update_count()));
      group.Add("Deletes",lexical_cast(threads[i]->delete_count()));
      group.Add("Number of Deadlocks",lexical_cast(threads[i]->deadlock_count()));
      tx_count = threads[i]->tx_count();
      if(tx_count > 0){
        unsigned int failed = tx_count - threads[i]->tx_success_count();
        group.Add("Failed Transactions",lexical_cast(failed)+" ("+
                  lexical_cast(100*failed/tx_count)+"%)");
      } else {
        group.Add("Failed Transactions","0 (100%)");
      }
      statistics->AddGroup(group);
    }
  }

  return statistics;
}

// Initalizes the workload using the given properties.
// The properties need to contain the path to a valid workload file
// ("workload-file")
bool SIGMOD2012BasicWorkload::Init(BenchmarkProperties &properties){
  // Seed the RNG
  logger_.AddSection("RNG Initialization","Seeding the RNG with seed "+
                     lexical_cast<int>(properties.seed()));
  rng_ = new FastRandNumberGenerator(properties.seed());
  logger_.CloseSection(true);

  // Setup the sample indices
  properties_ = SIGMOD2012Properties::LoadFromFile(logger_,
                                            properties.Get("workload-file",""),
                                            properties.seed());
  logger_.CloseSection(true);

  // Update statistics settings
  if(properties.Get("extensive-stats","") == "true")
    properties_->extensive_stats(true);
  else if(properties.Get("extensive-stats","") == "false")
    properties_->extensive_stats(false);

  if(!properties_)
    return false;

  // Create the indices
  if(!CreateIndices())
    return false;

  // Populate the indices
  if(!PopulateIndices())
    return false;

  // Initialize the benchmark threads
  threads = new SIGMOD2012BenchmarkThread*[properties.thread_count()];
  for(unsigned int i = 0; i < properties.thread_count(); i++){
    threads[i] = new SIGMOD2012BenchmarkThread(logger_,*properties_,
                     properties_->GetIndex(i%properties_->index_count()),
                                               properties.seed()+i);
  }

  thread_count_ = properties.thread_count();
  measurement_time_ = properties.measurement_time();
  warmup_time_=properties.warmup_time();

  return true;
}

// Creates the indices used by the benchmark
bool SIGMOD2012BasicWorkload::CreateIndices(){
  if(!properties_)
    return false;

  logger_.AddSection("Index creation","Creating indices");

  for(unsigned int i = 0; i < properties_->index_count(); i++){
    SIGMOD2012IndexProperties &ip = properties_->GetIndex(i);
    if(kOk != CreateIndex(ip.name(),ip.dimensions(),ip.types())){
      logger_.Error("Could not create index '"+std::string(ip.name())+"'\n");
      return false;
    }
  }
  logger_.CloseSection(true);
  return true;

};

// Runs the workload without gathering statistics (warm up)
void SIGMOD2012BasicWorkload::Warmup(){
  for(unsigned int i = 0; i < thread_count_; i++){
    threads[i]->Start();
  }
  sleep(warmup_time_);
  warmed_up_ = true;
}


// Populates the indices used by the benchmark
// (NOTE: Uses one Thread per index)
bool SIGMOD2012BasicWorkload::PopulateIndices(){
  Timer populate_timer;
  bool success = true;
  PopulateThread *threads[properties_->index_count()];

  logger_.AddSection("Index population","Populating indices");

  populate_timer.Start();

  for(unsigned int i = 0; i < properties_->index_count(); i++){
    threads[i] = new PopulateThread(logger_,properties_->GetIndex(i),rng_->Next());
    threads[i]->Start();
  }

  for(unsigned int i = 0; i < properties_->index_count(); i++){
    threads[i]->Join();
    if(!threads[i]->success())
      success = false;
  }
  populate_timer.Stop();
  logger_.CloseSection(success,lexical_cast(populate_timer.milliseconds())+" ms");
  return success;
}

// Initializes a new population thread that populates the given index
SIGMOD2012BasicWorkload::PopulateThread::PopulateThread(Logger &logger,
                          SIGMOD2012IndexProperties &index, unsigned int seed):
                          Thread(),index_(index),logger_(logger),success_(false){

  // Initialize the random number generator
  rng_ = new XORShiftNumberGenerator(seed);

  // Copy the attribute generators
  generators_ = new Generator*[index_.dimensions()];

  for(unsigned int i=0; i < index_.dimensions(); i++){
    generators_[i] = index_.generators()[i]->clone();
    generators_[i]->rng(*rng_);
    generators_[i]->reset();
  }
}

// Destructor for PopulateThread
SIGMOD2012BasicWorkload::PopulateThread::~PopulateThread(){
  delete rng_;

  // Delete the attribute generators
  for(unsigned int i=0; i < index_.dimensions(); i++){
    delete generators_[i];
  }

  delete[] generators_;
}

// Run the population thread
void SIGMOD2012BasicWorkload::PopulateThread::Run(){
#ifdef DEBUG_
  std::cout << "#" <<std::flush;
#endif
  Index *idx;
  // Open the index
  if(kOk != OpenIndex(index_.name(),&idx)){
    logger_.Error("Could not open index '" + lexical_cast(index_.name())+"'");
    return;
  }

  // Allocate all necessary memory
  Record *record = (Record*) malloc(sizeof(Record));
  record->key.attribute_count = index_.dimensions();
  record->key.value = (Attribute**) malloc(index_.dimensions()*sizeof(Attribute*));
  for(unsigned int i = 0; i < index_.dimensions(); i++){
    record->key.value[i] = (Attribute*) malloc(sizeof(Attribute));
    record->key.value[i]->type = index_.types()[i];
  }
  record->payload.size = index_.payload_size();
  record->payload.data = malloc(index_.payload_size());
  // Initialize the payload generator
  FixedLengthStringGenerator payload_generator(index_.payload_size(),*rng_);

  // Insert the records
  for(unsigned int i = 0; i < index_.populate_count(); i++){
    char* data = new char[index_.key_size()];
    size_t offset = 0;

    // Set the new key and insert it into the keystore of the respective index
    for(unsigned int j = 0; j < index_.dimensions(); j++){
      switch(index_.types()[j]){
        case kShort:
          record->key.value[j]->short_value = ((IntegerGenerator*)generators_[j])->next();
          *((int32_t*)(data+offset)) = record->key.value[j]->short_value;
          offset+=sizeof(int32_t);
          break;
        case kInt:
          record->key.value[j]->int_value = ((IntegerGenerator*)generators_[j])->next();
          *((int64_t*)(data+offset)) = record->key.value[j]->int_value;
          offset+=sizeof(int64_t);
          break;
        case kVarchar:
          strcpy(record->key.value[j]->char_value,((StringGenerator*)generators_[j])->next().c_str());
          strcpy(data+offset,record->key.value[j]->char_value);
          offset+=MAX_VARCHAR_LENGTH;
          break;
        default:
          assert(0);
      }
    }

    // Insert the key into the keystore
    index_.keystore().Push(data);

    // Set the new Payload
    memcpy(record->payload.data,payload_generator.next().c_str(),index_.payload_size());

    // Insert the record
    if(kOk != InsertRecord(0, idx, record)){
      logger_.Error("Could not insert record");
      return;
    }
  }

  // Close the Index
  if(kOk != CloseIndex(&idx)){
    logger_.Error("Could not close index '" +lexical_cast(index_.name())+"'");
    return;
  }

  success_ = true;
#ifdef DEBUG_
  std::cout << "!" <<std::flush;
#endif
};

// The available transaction type probabilities
enum Probabilities{
  kRangeProb = 0,
  kPointProb,
  kUpdateProb,
  kInsertProb,
  kDeleteProb
};

// Create a new benchmark thread that works on the given index
SIGMOD2012BasicWorkload::SIGMOD2012BenchmarkThread::SIGMOD2012BenchmarkThread(
 Logger &logger,SIGMOD2012Properties &properties, SIGMOD2012IndexProperties &index,
                                                            unsigned int seed):
 BenchmarkThread(),index_(index),logger_(logger){
  std::vector<int> probs;

  // Range queries
  probs.push_back(properties.range_portion());
  // Point queries
  probs.push_back(properties.point_portion());
  // Updates
  probs.push_back(properties.update_portion());
  // Inserts
  probs.push_back(properties.insert_portion());
  // Deletes
  probs.push_back(properties.delete_portion());

  // Initialize random number generator
  rng_ = new XORShiftNumberGenerator(seed);

  // Initialize the generator
  op_select_ = new DiscreteGenerator(probs,*rng_);

  // Copy the attribute generators
  generators_ = new Generator*[index_.dimensions()];
  for(unsigned int i=0; i < index_.dimensions(); i++){
    generators_[i] = index_.generators()[i]->clone();
    generators_[i]->rng(*rng_);
    generators_[i]->reset();
  }

  ResetStatistics();
};

// Destructor for BenchmarkThread
SIGMOD2012BasicWorkload::SIGMOD2012BenchmarkThread::~SIGMOD2012BenchmarkThread(){
  delete rng_;

  // Delete the attribute generators
  for(unsigned int i=0; i < index_.dimensions(); i++){
    delete generators_[i];
  }

  delete[] generators_;
}

// Runs the  benchmark thread
void SIGMOD2012BasicWorkload::SIGMOD2012BenchmarkThread::Run(){
#ifdef DEBUG_
  std::cout<<"#"<<std::flush;
#endif
  ResetStatistics();
  run_ = true;

  // Open the index for this thread
  Index *idx;

  if(kOk != OpenIndex(index_.name(),&idx)){
    logger_.Error("Could not open index'"+lexical_cast(index_.name())+"'");
    return;
  }

  // Allocate the minimum and maximum key
  Key min, max;
  min.attribute_count = index_.dimensions();
  max.attribute_count = index_.dimensions();
  min.value = (Attribute**) malloc(index_.dimensions()*sizeof(Attribute*));
  max.value = (Attribute**) malloc(index_.dimensions()*sizeof(Attribute*));

  for(unsigned int i = 0; i < index_.dimensions(); i++){
    min.value[i] = (Attribute*) malloc(sizeof(Attribute));
    min.value[i]->type = index_.types()[i];
    max.value[i] = (Attribute*) malloc(sizeof(Attribute));
    max.value[i]->type = index_.types()[i];
  }
  
  // Initialize the payload generator
  FixedLengthStringGenerator payload_generator(index_.payload_size(),*rng_);

  int value;
  int range_queries = 0;
  int tx_ops;
  Record *retrieved;

  // Allocate a record to use for all operations
  Record* a = (Record*) malloc(sizeof(Record));
  a->key.attribute_count = index_.dimensions();
  a->key.value = (Attribute**) malloc(index_.dimensions()*sizeof(Attribute*));
  a->payload.size = index_.payload_size();
  a->payload.data = malloc(a->payload.size);;
  for(int i = 0; i < a->key.attribute_count; i++){
    a->key.value[i] = (Attribute*) malloc(sizeof(Attribute));
    a->key.value[i]->type = index_.types()[i];
  }

  // Main loop
  while(run_){
    // Select a random operation
    value = op_select_->next();

    // Begin a new transaction
    Transaction* tx;
    if(kOk != BeginTransaction(&tx)){
      logger_.Error("Could not begin a new transaction");
      run_ = false;
      break;
    }
    tx_ops = 0;
    tx_count_++;

    // Execute the transaction
    switch(value){
      case kRangeProb:{
        // Generate a random range
        for(unsigned int i = 0; i < index_.dimensions(); i++){
          switch(index_.types()[i]){
            case kShort:{
              int32_t min_val = ((IntegerGenerator*)generators_[i])->next();
              int32_t max_val = ((IntegerGenerator*)generators_[i])->next();
              if(min_val<max_val){
                min.value[i]->short_value = min_val;
                max.value[i]->short_value = max_val;
              } else {
                min.value[i]->short_value = max_val;
                max.value[i]->short_value = min_val;
              }

              break;
            }
            case kInt:{
              int64_t min_val = ((IntegerGenerator*)generators_[i])->next();
              int64_t max_val = ((IntegerGenerator*)generators_[i])->next();
              if(min_val<max_val){
                min.value[i]->int_value = min_val;
                max.value[i]->int_value = max_val;
              } else {
                min.value[i]->int_value = max_val;
                max.value[i]->int_value = min_val;
              }
              break;
            }
            case kVarchar:{
              const char* min_val = ((StringGenerator*)generators_[i])->next().c_str();
              const char* max_val = ((StringGenerator*)generators_[i])->next().c_str();
              if(strcmp(min_val,max_val)<0){
                strcpy(min.value[i]->char_value,min_val);
                strcpy(max.value[i]->char_value,max_val);
              } else {
                strcpy(min.value[i]->char_value,max_val);
                strcpy(max.value[i]->char_value,min_val);
              }
              break;
            }
          }
        }

        Iterator *it;

        // Get the records
        ErrorCode r = GetRecords(tx,idx,min,max,&it);
        if(r == kOk){
          increment(range_queries);
          for(int i = 0; i < 200; i++){
            increment(tx_ops);
            if((r = GetNext(it,&retrieved)) != kOk ){
              CloseIterator(&it);
              if(r == kErrorDeadlock)
                deadlock_count_++;
              break;
            }
            ReleaseRecord(retrieved);
          }
          CloseIterator(&it);
        }

        break;
      }
      case kPointProb:{
        //logger_.Debug("POINT");
        for(int i = 0; i < POINT_QUERIES_PER_TXN; i++){
          Iterator *it;

          // Get a random key
          char* ptr;
          if(!index_.keystore().Get(rng_->Next(),ptr)){
            break;
          };

          SetKey(ptr, a->key);

          // Get the record
          ErrorCode r = GetRecords(tx,idx,a->key,a->key,&it);

          if(r == kOk){
            if((r = GetNext(it,&retrieved)) != kOk ){
              if(r == kErrorDeadlock)
                deadlock_count_++;
              CloseIterator(&it);
              break;
            }
            increment(tx_ops);
            ReleaseRecord(retrieved);
            r = CloseIterator(&it);
          }
        }

        break;
      }
      case kUpdateProb:{
        //logger_.Debug("UPDATE");
        for(int i = 0; i < UPDATES_PER_TXN; i++){
          // Get a random key
          char* ptr;
          if(!index_.keystore().Get(rng_->Next(),ptr)){
            break;
          };

          SetKey(ptr, a->key);

          // Set a new payload
          memcpy((char*)a->payload.data,payload_generator.next().c_str(),index_.payload_size());

          // Update the record
          ErrorCode r = UpdateRecord(tx,idx,a,&(a->payload),kIgnorePayload);

          if(r != kOk){
            if(r == kErrorDeadlock)
              deadlock_count_++;
            break;
          }
          increment(tx_ops);
        }

        break;
      }
      case kInsertProb:{
        //logger_.Debug("INSERT");
        for(int i = 0; i < INSERTS_PER_TXN; i++){
          char* data = new char[index_.key_size()];
          size_t offset = 0;
          // Set the new key and insert it into the keystore of the respective index
          for(unsigned int j = 0; j < index_.dimensions(); j++){
            switch(index_.types()[j]){
              case kShort:
                a->key.value[j]->short_value = ((IntegerGenerator*)generators_[j])->next();
                *((int32_t*)(data+offset)) = a->key.value[j]->short_value;
                offset+=sizeof(int32_t);
                break;
              case kInt:
                a->key.value[j]->int_value = ((IntegerGenerator*)generators_[j])->next();
                *((int64_t*)(data+offset)) = a->key.value[j]->int_value;
                offset+=sizeof(int64_t);
                break;
              case kVarchar:
                strcpy(a->key.value[j]->char_value,((StringGenerator*)generators_[j])->next().c_str());
                strcpy(data+offset,a->key.value[j]->char_value);
                offset+=MAX_VARCHAR_LENGTH;
                break;
              default:
                assert(0);
            }
          }


          // Insert the key into the keystore
          index_.keystore().Push(data);

          // Create a random payload
          if(!a->payload.data)
            a->payload.data = malloc(a->payload.size);
          memcpy((char*)a->payload.data,payload_generator.next().c_str(),index_.payload_size());

          // Insert the record
          ErrorCode r = InsertRecord(tx, idx, a);
          if(r == kOk){
            increment(tx_ops);
          } else {
            if(r == kErrorDeadlock)
              deadlock_count_++;
            break;
          }
        }

        break;
      }
      case kDeleteProb:{
        //logger_.Debug("DELETE");
        for(int i = 0; i < DELETES_PER_TXN; i++){
          // Get the last inserted key
          char* ptr;
          if(!index_.keystore().Pop(ptr)){
            break;
          };

          SetKey(ptr, a->key);
          delete[] ptr;

          // Delete the record
          ErrorCode r = DeleteRecord(tx,idx,a,kIgnorePayload);

          if(r != kOk){
            if(r == kErrorDeadlock)
              deadlock_count_++;
            break;
          } else {
            increment(tx_ops);
          }
        }
        break;
      }
      default:
        assert(false);
    }
    // Commit the transaction
    ErrorCode r = CommitTransaction(&tx);
    switch(r){
      case kOk:
        switch(value){
          case kRangeProb:
            range_count_+=tx_ops;
            range_queries_+=range_queries;
            range_queries = 0;
            break;
          case kPointProb:
            point_count_+=tx_ops;
            break;
          case kInsertProb:
            insert_count_+=tx_ops;
            break;
          case kDeleteProb:
            delete_count_+=tx_ops;
            break;
          case kUpdateProb:
            update_count_+=tx_ops;
            break;
          default:
            assert(false);
        }
        tx_success_count_++;
        break;
      case kErrorDeadlock:
        deadlock_count_++;
      default:
        break;
    }

  }


  if(kOk != CloseIndex(&idx)){
    logger_.Warning("Could not close index'"+lexical_cast(index_.name())+"'");
  }

  // Cleanup
  ReleaseRecord(a);

#ifdef DEBUG_
  std::cout<<"!"<<std::flush;
#endif
}

// Resets all statistical values to its default values
void SIGMOD2012BasicWorkload::SIGMOD2012BenchmarkThread::ResetStatistics(){
  tx_count_ = 0;
  tx_success_count_ = 0;
  deadlock_count_ = 0;
  op_count_ = 0;

  range_queries_ = 0;
  range_count_ = 0;
  point_count_ = 0;
  insert_count_ = 0;
  delete_count_ = 0;
  update_count_ = 0;
}

// Serializes the given key and stores it at the given destination
// (NOTE: SetKey does not allocate the necessary memory to store the
//        serialized key.)
void SIGMOD2012BasicWorkload::SIGMOD2012BenchmarkThread::SetKey(char* serialized, Key &key){
  size_t offset = 0;
  // Deserialize the key
  for(unsigned int i = 0; i < key.attribute_count; i++){
    switch(key.value[i]->type){
      case kShort:
        key.value[i]->short_value = *((int32_t*)(serialized+offset));
        offset+=sizeof(int32_t);
        break;
      case kInt:
        key.value[i]->int_value = *((int64_t*)(serialized+offset));
        offset+=sizeof(int64_t);
        break;
      case kVarchar:
        strcpy(key.value[i]->char_value,serialized+offset);
        offset+=MAX_VARCHAR_LENGTH;
        break;
      default:
        assert(0);
    }
  }
}

// Frees all memory used by the given record
void SIGMOD2012BasicWorkload::SIGMOD2012BenchmarkThread::ReleaseRecord(Record *record){
  free(record->payload.data);
  for(int i = 0; i < record->key.attribute_count; i++){
    free(record->key.value[i]);
  }
  free(record->key.value);
  free(record);
}
