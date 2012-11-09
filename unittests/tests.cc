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
 
 Current version: 1.03 (released March 8, 2012)
 
 Version history:
   - 1.03 (March 8, 2012)
     * Updated the bulk test to correctly check for updated records
   - 1.02 (February 14, 2012)
     * Completly revised independent test cases that check for data integrity
   - 1.01 (December 17, 2011)
     * Replaced the CreateRecord functions with their correct implementations
     * Modified tests to match the changes made to the contest's API
   - 1.0 Initial release (December 14, 2011) 
 */

/** @file
 Defines test cases to ensure the correctness of implementations implementing
 the contest interface.
 */

#define __STDC_LIMIT_MACROS
#include <contest_interface.h>
#include <common/macros.h>
#include <pthread.h>

#include "test_util.h"
#include "util.h"

// The names of all indices used by the test cases
#define BASIC_TEST_INDEX "BasicIndex"
#define PARTIAL_MATCH_TEST_INDEX "PartialMatchIndex"
#define RANGE_TEST_INDEX "RangeIndex"
#define ISOLATION_TEST_INDEX "IsolationIndex"
#define AUTO_COMMIT_TEST_INDEX "AutoCommitIndex"
#define BULK_TEST_INDEX "BulkIndex"
#define ERROR_HANDLING_TEST_INDEX "ErrorHandlingIndex"

// The name of an index that will not be created during the test
// (this index is used by the ErrorHandlingTest to ensure that non-existent
// indices cannot be modified)
#define NON_EXISTENT_INDEX "NonExistentIndex"

// The minimum and maximum varchar value
#define MIN_VARCHAR ""
#define MAX_VARCHAR (std::string(MAX_VARCHAR_LENGTH,126)).c_str()

// Test to ensure that the basic behavior of the implementation is correct
// 
// It tests the basic functions InsertRecord(), UpdateRecord() and
// DeleteRecord() and makes sure that simple point queries are handled
// correctly.
// This test also ensures that committed changes are persisted to the data store 
// and transactions are properly aborted so that the changes made during the
// course of the transaction are not committed.
TEST(BasicTest){
  // Create some test records
  Record *a  = CreateRecordBasic(1,2,"a","Record a");
  Record *b  = CreateRecordBasic(1,2,"b","Record b");
  Record *bc = CreateRecordBasic(1,2,"b","Record b");
  Record *c  = CreateRecordBasic(INT32_MIN,INT64_MIN,MIN_VARCHAR,"Record c");
  Record *d  = CreateRecordBasic(INT32_MAX,INT64_MAX,MAX_VARCHAR,"Record d");
  Record *e  = CreateRecordBasic(2,3,"c","Record e");
  Record *f  = CreateRecordBasic(2,3,"c","Record f");
  Record *g  = CreateRecordBasic(3,4,"d","Record g");
  Record *h  = CreateRecordBasic(4,4,"a","Record h");
  Record *i  = CreateRecordBasic(5,1,"a","Record i");
  
  // Create a simple index with keys comprising 3 attributes
  // (one for each attribute type)
  KeyType schema = {kShort, kInt, kVarchar};
  ErrorCode err = CreateIndex(BASIC_TEST_INDEX, COUNT_OF(schema), schema);
  
  ASSERT_EQUALS(err, kOk, "Could not create the new index");
  
  if(err == kOk){
    // Begin a new Transaction
    Transaction *tx;
    ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                  "Could not start a new transaction");
    
    if(err == kOk){
      Index *idx;
      // Open the created index
      ASSERT_EQUALS(err = OpenIndex(BASIC_TEST_INDEX, &idx), kOk,
                    "Could not open the created index");
      
      if(err == kOk){
        // Insert the test records
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,a),
                      "Could not insert record (1,2,'a','Record a')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,b),
                      "Could not insert record (1,2,'b','Record b')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,c),
                      "Could not insert record with minimal key");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,d),
                      "Could not insert record with maximal key");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,e),
                      "Could not insert record (2,3,'c','Record e')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,f),
                      "Could not insert record (2,3,'c','Record f')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,g),
                      "Could not insert record g");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,g),
                      "Could not insert record g (full duplicate)");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,h),
                      "Could not insert record h");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,h),
                      "Could not insert record h (full duplicate)");
        //
        // Ensure that simple point queries are correct
        //
        Iterator *it;
        Record* tmp;
        
        // Query for Record a
        ASSERT_EQUALS(err = GetRecords(tx,idx,a->key,a->key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record a");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*a,*tmp),
                        "The original and the retrieved record a do not match");
            Release(tmp);
          }
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Delete the inserted Record bc (to ensure that the payload
        // is really copied)
        Release(bc);
        
        // Query for record b
        ASSERT_EQUALS(err = GetRecords(tx,idx,b->key,b->key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record b");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*b,*tmp),
                          "Record bc and the retrieved record b do not match");
            Release(tmp);
          }
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Query for the record comprising the minimal attribute values
        ASSERT_EQUALS(err = GetRecords(tx,idx,c->key,c->key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record c");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*c,*tmp),
                        "The original and the retrieved record c do not match");
            Release(tmp);
          }
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Query for the record comprising the maximal attribute values
        ASSERT_EQUALS(err = GetRecords(tx,idx,d->key,d->key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record d");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*d,*tmp),
                        "The original and the retrieved record d do not match");
            Release(tmp);
          }
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Query for a non-existing record
        ASSERT_EQUALS(err = GetRecords(tx,idx,i->key,i->key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                        "The iterator did not report an end of range");
          if(err == kOk){
            Release(tmp);
          }
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Ensure that duplicates work
        ASSERT_EQUALS(err = GetRecords(tx,idx,e->key,e->key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record e/f");
          if(err == kOk){
            int r;
            if((r = RecordCmp(*e,*tmp)) == 0){
              Release(tmp);
              ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                            "Could not retrieve record f (second)");
              if(err == kOk){
                ASSERT_EQUALS(0, RecordCmp(*f,*tmp),
                              "The original and the retrieved record f "
                              "do not match (second)");
                Release(tmp);
              }
            } else if((r = RecordCmp(*f,*tmp)) == 0){
              Release(tmp);
              ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                            "Could not retrieve record e (second)");
              if(err == kOk){
                ASSERT_EQUALS(0, RecordCmp(*e,*tmp),
                              "The original and the retrieved record e "
                              "do not match (second)");
                Release(tmp);
              }
            } else {
              Release(tmp);
            }
            ASSERT_EQUALS(0, r, "The retrieved record neither matches record e "
                                "nor record f");
          }
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Ensure that full duplicate records (duplicate key and duplicate
        // payload) work correctly
        ASSERT_EQUALS(err = GetRecords(tx,idx,g->key,g->key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record g");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*g,*tmp),
                        "The original and the retrieved record g do not match");
            Release(tmp);
            ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                          "Could not retrieve record g (duplicate)");
            if(err == kOk){
              ASSERT_EQUALS(0, RecordCmp(*g,*tmp),
                            "The original and the retrieved record g "
                            "(duplicate) do not match");
              Release(tmp);
            }
          }
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          
          // Ensure that DeleteRecord() deletes exactly one record
          // (when no flags are passed)
          ASSERT_EQUALS(err = DeleteRecord(tx,idx,g,0), kOk,
                        "Could not delete record g");
          
          if(err == kOk){
            ASSERT_EQUALS(err = GetRecords(tx,idx,g->key,g->key, &it), kOk,
                          "Could not open iterator");
            if(err == kOk){
              ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                            "Could not retrieve record g "
                            "(after deleting a full duplicate record)");
              if(err == kOk){
                ASSERT_EQUALS(0, RecordCmp(*g,*tmp),
                              "The original and the retrieved record g do not "
                              "match (after deleting a full duplicate record)");
                Release(tmp);
              }
              ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                            "The iterator did not return an end of range");
            }
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
        }
          
        // Ensure that UpdateRecord() only updates one one record
        // (when no flags are passed)
        Block payload;
        SetValue(payload,"new payload");
        
        ASSERT_EQUALS(err = UpdateRecord(tx,idx,h,&payload,0), kOk,
                      "Could not update record h");
          
        if(err == kOk){
          // Check that the updated record and the duplicate
          // (with the old payload) can be retrieved
          ASSERT_EQUALS(err = GetRecords(tx,idx,h->key,h->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                          "Could not retrieve record h "
                          "(after updating a full duplicate record)");
            if(err == kOk){
              bool r;
              if((r = (RecordCmp(*h,*tmp) == 0))){
                ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                              "Could not retrieve updated version of h");
                if(err == kOk){
                  ASSERT_EQUALS(0, KeyCmp(h->key,tmp->key),
                                "The key of the updated version of h does "
                                "not match the original key");
                  ASSERT_EQUALS(0, BlockCmp(payload,tmp->payload),
                                "The payload of the updated version of h does "
                                "not match the new payload");
                  Release(tmp);
                }
              } else if((r = ((KeyCmp(h->key,tmp->key) == 0)
                             && (BlockCmp(payload,tmp->payload) == 0)))){
                ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                              "Could not retrieve original version of h");
                if(err == kOk){
                  ASSERT_EQUALS(0, RecordCmp(*h,*tmp),
                                "The retrieved record h (second) does not "
                                "match the original record");
                  Release(tmp);
                }
              } else {
                Release(tmp);
              }
              ASSERT_EQUALS(true, r, "The retrieved record neither matches "
                          "record h in its old version nor in its new version");

            }
            ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                          "The iterator did not return an end of range");

            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
        }
        
        // Close the index
        ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
      }
      
      // Commit the transaction
      err = CommitTransaction(&tx);
      
      ASSERT_EQUALS(kOk, err, "Could not commit transaction");
      if(err == kOk){
        // Check that the commited data still exists
        Iterator *it;
        Record* tmp;
        
        // Open the index
        ASSERT_EQUALS(err = OpenIndex(BASIC_TEST_INDEX, &idx), kOk,
                      "Could not open index");
        
        if(err == kOk){
          // Query for Record a
          ASSERT_EQUALS(err = GetRecords(NULL,idx,a->key,a->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                          "Could not retrieve record a (after commit)");
            if(err == kOk){
              ASSERT_EQUALS(0, RecordCmp(*a,*tmp),
                            "The original and the retrieved record a "
                            "do not match (after commit)");
              Release(tmp);
            }
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
          
          // Query for record b
          ASSERT_EQUALS(err = GetRecords(NULL,idx,b->key,b->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                          "Could not retrieve record b (after commit)");
            if(err == kOk){
              ASSERT_EQUALS(0, RecordCmp(*b,*tmp),
                            "The original and the retrieved record b "
                            "do not match (after commit)");
              Release(tmp);
            }
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
          
          // Close the index
          ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
        }
      }
    }
    
    
    // Ensure that transactions are rolled back properly
    ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                  "Could not start a new transaction");
    
    if(err == kOk){
      Index *idx;
      // Open the index
      ASSERT_EQUALS(err = OpenIndex(BASIC_TEST_INDEX, &idx), kOk,
                    "Could not open index");
      
      if(err == kOk){
        //Insert record i (should not be present inside the database)
        ASSERT_EQUALS(InsertRecord(tx,idx,i), kOk, "Could not insert record i");
        
        // Close the index
        ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
      }
      
      // Abort the transaction so record i is not committed
      ASSERT_EQUALS(err = AbortTransaction(&tx), kOk,
                    "Could not abort transaction");
      if(err == kOk){
        Iterator *it;
        Record *tmp;
        
        // Verify that record i cannot be found
        ASSERT_EQUALS(err = OpenIndex(BASIC_TEST_INDEX, &idx), kOk,
                      "Could not open the index");
        if(err == kOk){
          ASSERT_EQUALS(err = GetRecords(NULL,idx,i->key,i->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                          "Record i has been found "
                          "(after the transaction aborted)");
            if(err == kOk){
              Release(tmp);
            }
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
          
          // Close the index
          ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
        }
      }
    }
    
    // Delete the used index
    ASSERT_EQUALS(DeleteIndex(BASIC_TEST_INDEX), kOk,
                  "Could not delete the used index");
  }
  
  // Cleanup
  Release(a);
  Release(b);
  Release(c);
  Release(d);
  Release(e);
  Release(f);
  Release(g);
  Release(h);
  Release(i);
};


// Test to ensure that partial-match queries are handled correctly
//
// This test executes different partial-match queries and makes sure that they
// only return matching records.
TEST(PartialMatchTest){
  // Create some test records
  Record* a = CreateRecordPartialMatch(1,2,3,"Record a");
  Record* b = CreateRecordPartialMatch(1,4,3,"Record b");
  Record* c = CreateRecordPartialMatch(2,2,3,"Record c");
  Record* d = CreateRecordPartialMatch(2,4,3,"Record d");
  Record* e = CreateRecordPartialMatch(2,5,3,"Record e");
  Record* f = CreateRecordPartialMatch(3,2,3,"Record f");
  
  // Create a simple index with keys comprising 3 short attributes
  KeyType schema = {kShort, kShort, kShort};
  ErrorCode err = CreateIndex(PARTIAL_MATCH_TEST_INDEX, COUNT_OF(schema), schema);

  ASSERT_EQUALS(err, kOk, "Could not create the new index");

  if(err == kOk){
    // Begin a new Transaction
    Transaction *tx;
    ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                  "Could not start a new transaction");
    
    if(err == kOk){
      // Open the index
      Index *idx;
      ASSERT_EQUALS(err = OpenIndex(PARTIAL_MATCH_TEST_INDEX, &idx), kOk,
                    "Could not open index");
      if(err == kOk){
        // Insert the test records
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,a),
                      "Could not insert record (1,2,3,'Record a')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,b),
                      "Could not insert record (1,4,3,'Record b')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,c),
                      "Could not insert record (2,2,3,'Record c')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,d),
                      "Could not insert record (2,4,3,'Record d')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,e),
                      "Could not insert record (2,5,3,'Record e')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,f),
                      "Could not insert record (3,2,3,'Record f')");
        
        Key key;
        key.attribute_count = 3;
        key.value = (Attribute**) malloc(3*sizeof(Attribute*));
        
        Iterator *it;
        Record *tmp;
        
        // Partial-match query Nr. 1: (a, b = 2, c = 3)
        key.value[0] = NULL;
        key.value[1] = ShortAttribute(2);
        key.value[2] = ShortAttribute(3);
        
        ASSERT_EQUALS(err = GetRecords(tx, idx, key, key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record a");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*a,*tmp),
                        "The original and the retrieved record a do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record c");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*c,*tmp),
                        "The original and the retrieved record c do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record f");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*f,*tmp),
                        "The original and the retrieved record f do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(GetNext(it,&tmp), kErrorNotFound,
                        "Iterator did not return an end of range");
          
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Partial-match query Nr. 2: (a = 2, b, c = 3)
        free(key.value[0]);
        free(key.value[1]);
        free(key.value[2]);
        
        key.value[0] = ShortAttribute(2);
        key.value[1] = NULL;
        key.value[2] = ShortAttribute(3);
        
        ASSERT_EQUALS(err = GetRecords(tx, idx, key, key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record c");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*c,*tmp),
                        "The original and the retrieved record c do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record d");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*d,*tmp),
                        "The original and the retrieved record d do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record e");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*e,*tmp),
                        "The original and the retrieved record e do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(GetNext(it,&tmp), kErrorNotFound,
                        "Iterator did not return an end of range");
          
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }

        
        // Partial-match query Nr. 3: (a, b = 4, c)
        free(key.value[0]);
        free(key.value[1]);
        free(key.value[2]);

        key.value[0] = NULL;
        key.value[1] = ShortAttribute(4);
        key.value[2] = NULL;
        
        ASSERT_EQUALS(err = GetRecords(tx, idx, key, key, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record b");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*b,*tmp),
                        "The original and the retrieved record b do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record d");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*d,*tmp),
                        "The original and the retrieved record d do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(GetNext(it,&tmp), kErrorNotFound,
                        "Iterator did not return end of range");
          
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }

        // Close the index
        ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
      }
      // Abort the transaction
      ASSERT_EQUALS(AbortTransaction(&tx), kOk, "Could not abort transaction");
    }
    
    // Delete the used index
    ASSERT_EQUALS(DeleteIndex(PARTIAL_MATCH_TEST_INDEX), kOk,
                  "Could not delete the used index");
  }

  // Cleanup
  Release(a);
  Release(b);
  Release(c);
  Release(d);
  Release(e);
  Release(f);
};


// Test to ensure that range queries are handled correctly
//
// It executes some basic range queries as well as some more complex ones
// (e.g. range queries with duplicates as minimum or maximum key
//  or range queries where the upper boundary of a VARCHAR attribute only
//  partially matches an existing record).
TEST(RangeTest){
  // Create some test records
  Record* a = CreateRecordRange(1,"a",2,"Record a");  // -
  Record* b = CreateRecordRange(2,"a",3,"Record b");  // -
  Record* c = CreateRecordRange(2,"b",2,"Record c");  // x
  Record* d = CreateRecordRange(2,"b",2,"Record d");  // x
  Record* e = CreateRecordRange(2,"bb",2,"Record e"); // x
  Record* f = CreateRecordRange(2,"c",3,"Record f");  // x
  Record* g = CreateRecordRange(3,"a",2,"Record g");  // -
  Record* h = CreateRecordRange(3,"c",3,"Record h");  // x
  Record* i = CreateRecordRange(3,"cc",3,"Record i"); // -
  Record* j = CreateRecordRange(4,"b",4,"Record j");  // -

  
  // Create a simple index with keys comprising 3 attributes
  KeyType schema = {kShort, kVarchar, kInt};
  ErrorCode err = CreateIndex(RANGE_TEST_INDEX, COUNT_OF(schema), schema);
  
  ASSERT_EQUALS(err, kOk, "Could not create the new index");
  if(err == kOk) {
    // Begin a new Transaction
    Transaction *tx;
    ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                  "Could not start a new transaction");
    
    if(err == kOk){
      Index *idx;
      // Open the created index
      ASSERT_EQUALS(err = OpenIndex(RANGE_TEST_INDEX, &idx), kOk,
                    "Could not open the created index");
      
      if(err == kOk){
        // Insert the test records
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,a),
                      "Could not insert record (1,'a',2,'Record a')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,b),
                      "Could not insert record (2,'a',3,'Record b')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,c),
                      "Could not insert record (2,'b',2,'Record c')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,d),
                      "Could not insert record (2,'b',2,'Record d')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,e),
                      "Could not insert record (2,'bb',2,'Record e')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,f),
                      "Could not insert record (2,'c',3,'Record f')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,g),
                      "Could not insert record (3,'a',2,'Record g')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,h),
                      "Could not insert record (3,'c',3,'Record h')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,i),
                      "Could not insert record (3,'cc',3,'Record i')");
        ASSERT_EQUALS(kOk, InsertRecord(tx,idx,j),
                      "Could not insert record (4,'b',4,'Record j')");
        
        
        Key key_min;
        key_min.attribute_count = 3;
        key_min.value = (Attribute**) malloc(3*sizeof(Attribute*));
        
        Key key_max;
        key_max.attribute_count = 3;
        key_max.value = (Attribute**) malloc(3*sizeof(Attribute*));
        
        Iterator *it;
        Record *tmp;
        
        // Range query Nr. 1: (2 - 3, 'b' - 'c', 2 - 3)
        key_min.value[0] = ShortAttribute(2);
        key_min.value[1] = VarcharAttribute("b");
        key_min.value[2] = IntAttribute(2);
        
        key_max.value[0] = ShortAttribute(3);
        key_max.value[1] = VarcharAttribute("c");
        key_max.value[2] = IntAttribute(3);
        
        ASSERT_EQUALS(err = GetRecords(tx, idx, key_min, key_max, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record c/d");
          if(err == kOk){
            int r;
            if((r = RecordCmp(*c,*tmp)) == 0){
              Release(tmp);
              ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                            "Could not retrieve record d (second)");
              if(err == kOk){
                ASSERT_EQUALS(0, RecordCmp(*d,*tmp),
                              "The original and the retrieved record d "
                              "do not match (second)");
                Release(tmp);
              }
            } else if((r = RecordCmp(*d,*tmp)) == 0){
              Release(tmp);
              ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                            "Could not retrieve record c (second)");
              if(err == kOk){
                ASSERT_EQUALS(0, RecordCmp(*c,*tmp),
                              "The original and the retrieved record c "
                              "do not match (second)");
                Release(tmp);
              }
            } else {
              Release(tmp);
            }
            ASSERT_EQUALS(0, r, "The retrieved record neither matches record c "
                                "nor record d");
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record e");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*e,*tmp),
                        "The original and the retrieved record e do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record f");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*f,*tmp),
                          "The original and the retrieved record f do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk, "Could not retrieve record h");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*h,*tmp),
                          "The original and the retrieved record h do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                        "Iterator did not report an end of range");
          if(err == kOk){
            Release(tmp);
          }
          
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Range query Nr. 2: (1 - 12, 'a' - 'z', 4 - 10)
        free(key_min.value[0]);
        free(key_min.value[1]);
        free(key_min.value[2]);
        
        free(key_max.value[0]);
        free(key_max.value[1]);
        free(key_max.value[2]);
        
        key_min.value[0] = ShortAttribute(4);
        key_min.value[1] = VarcharAttribute("a");
        key_min.value[2] = IntAttribute(4);
        
        key_max.value[0] = ShortAttribute(12);
        key_max.value[1] = VarcharAttribute("z");
        key_max.value[2] = IntAttribute(10);
        
        ASSERT_EQUALS(err = GetRecords(tx, idx, key_min, key_max, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record j");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*j,*tmp),
                        "The original and the retrieved record j do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                        "Iterator did not report an end of range");
          if(err == kOk){
            Release(tmp);
          }
          
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Range query Nr. 3: (1 - 5, '' - 'a', 0 - 2)
        free(key_min.value[0]);
        free(key_min.value[1]);
        free(key_min.value[2]);
        
        free(key_max.value[0]);
        free(key_max.value[1]);
        free(key_max.value[2]);
        
        key_min.value[0] = ShortAttribute(1);
        key_min.value[1] = VarcharAttribute("");
        key_min.value[2] = IntAttribute(0);
        
        key_max.value[0] = ShortAttribute(5);
        key_max.value[1] = VarcharAttribute("a");
        key_max.value[2] = IntAttribute(2);
        
        ASSERT_EQUALS(err = GetRecords(tx, idx, key_min, key_max, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record a");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*a,*tmp),
                        "The original and the retrieved record a do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                        "Could not retrieve record g");
          if(err == kOk){
            ASSERT_EQUALS(0, RecordCmp(*g,*tmp),
                        "The original and the retrieved record g do not match");
            Release(tmp);
          }
          
          ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                        "Iterator did not report an end of range");
          if(err == kOk){
            Release(tmp);
          }
          
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Range query Nr. 4: (1 - 2, 'b' - 'cc', 4)
        free(key_min.value[0]);
        free(key_min.value[1]);
        free(key_min.value[2]);
        
        free(key_max.value[0]);
        free(key_max.value[1]);
        free(key_max.value[2]);
        
        key_min.value[0] = ShortAttribute(1);
        key_min.value[1] = VarcharAttribute("b");
        key_min.value[2] = IntAttribute(4);
        
        key_max.value[0] = ShortAttribute(2);
        key_max.value[1] = VarcharAttribute("cc");
        key_max.value[2] = IntAttribute(4);
        
        ASSERT_EQUALS(err = GetRecords(tx, idx, key_min, key_max, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                        "Iterator did not report an end of range");
          if(err == kOk){
            Release(tmp);
          }
          
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Close the index
        ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
      }
      // Abort the transaction
      ASSERT_EQUALS(AbortTransaction(&tx), kOk, "Could not abort transaction");
    }

    // Delete the used index
    ASSERT_EQUALS(DeleteIndex(RANGE_TEST_INDEX), kOk,
                  "Could not delete the used index");
  }

  // Cleanup
  Release(a);
  Release(b);
  Release(c);
  Release(d);
  Release(e);
  Release(f);
  Release(g);
  Release(h);
  Release(i);
  Release(j);
};


// Test to ensure that uncommitted changes are not visible to other concurrent
// transactions
//
// This test creates a thread that continuously polls for new records while
// another thread inserts a record (without committing).
// To give the second thread some time to retrieve the new record the main
// thread will wait for one second until it terminates. Hence, this test may
// appear a little slow.
volatile bool run_isolation_test = true;

static void* IsolationTestThread(void*){
  Index* idx;
  ErrorCode err = OpenIndex(ISOLATION_TEST_INDEX, &idx);
  ASSERT_EQUALS(err, kOk,
                "Could not open the IsolationTest index");
  
  Key wildcard;
  wildcard.attribute_count = 1;
  wildcard.value = (Attribute**) malloc(sizeof(Attribute*));
  wildcard.value[0] = NULL;
  
  Transaction *tx;
  Iterator *it;
  Record *tmp;
  if(err == kOk){
    
    while(run_isolation_test){
      // Begin a new Transaction
      ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                    "Could not start a new transaction");
      if(err == kOk){
        ASSERT_EQUALS(err = GetRecords(tx, idx, wildcard, wildcard, &it), kOk,
                      "Could not open iterator");
        if(err == kOk){
          ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                        "Uncommitted data should not be visible.");
          if(err == kOk){
            Release(tmp);
          }
        
          ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
        }
        
        // Commit the transaction
        ASSERT_EQUALS(CommitTransaction(&tx), kOk,
                      "Could not commit transaction");
      }
    }
    
    // Close the index
    ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
  }
  
  return NULL;
}

TEST(IsolationTest){
  pthread_t thread;
  
  // A test records that will be inserted into the index
  Record *a = CreateRecordIsolation(1,"record a");
  
  // Create a simple index with keys comprising 1 short attribute
  KeyType schema = {kInt};
  ErrorCode err = CreateIndex(ISOLATION_TEST_INDEX, COUNT_OF(schema), schema);
  
  ASSERT_EQUALS(err, kOk, "Could not create the new index");
  if(err == kOk) {
    // Start the IsolationTestThread
    int r = pthread_create(&thread, NULL, IsolationTestThread, NULL);
    ASSERT_EQUALS(r, 0, "Could not create a new thread for the isolation test. "
                        "Aborting...");
    
    if(r == 0){
      // Begin a new Transaction
      Transaction *tx;
      ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                    "Could not start a new transaction");
      
      if(err == kOk){
        Index *idx;
        // Open the created index
        ASSERT_EQUALS(err = OpenIndex(ISOLATION_TEST_INDEX, &idx), kOk,
                      "Could not open the created index");
        if(err == kOk){
          
          // Insert the test record
          ASSERT_EQUALS(kOk, InsertRecord(tx,idx,a),
                        "Could not insert record (1,'record a')");
          
          // Close the index
          ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
          
          // Wait for a second
          sleep(1);
        }

        // Abort the transaction
        ASSERT_EQUALS(AbortTransaction(&tx), kOk,"Could not abort transaction");
      }
    }
    
    // Close the IsolationTestThread
    run_isolation_test = false;
    pthread_join(thread, NULL);
    
    // Delete the used index
    ASSERT_EQUALS(DeleteIndex(ISOLATION_TEST_INDEX), kOk,
                  "Could not delete the used index");
  }
  
  // Cleanup
  Release(a);
};


// Test to ensure that auto-commit mode works properly
//
// It tests that records can be modified without providing a valid transaction
// handle and makes sure that all changes are committed immediately.
TEST(AutoCommitTest){
  // Create some test records
  Record *a = CreateRecordAutoCommit(1,"record a");
  Record *b = CreateRecordAutoCommit(2,"record b");
  
  // Create a simple index with keys comprising 1 short attribute
  KeyType schema = {kShort};
  ErrorCode err = CreateIndex(AUTO_COMMIT_TEST_INDEX, COUNT_OF(schema), schema);
  
  ASSERT_EQUALS(err, kOk, "Could not create the new index");
  if(err == kOk) {
    Index *idx;

    // Open the created index
    ASSERT_EQUALS(err = OpenIndex(AUTO_COMMIT_TEST_INDEX, &idx), kOk,
                  "Could not open the created index");
    if(err == kOk){
      Iterator *it;
      Record *tmp;
      
      // Insert the test records
      ASSERT_EQUALS(err = InsertRecord(NULL, idx, a), kOk,
                    "Could not insert record a");
      ASSERT_EQUALS(err = InsertRecord(NULL, idx, b), kOk,
                    "Could not insert record b");
      
      // Verify that the record was successfully inserted
      ASSERT_EQUALS(err = GetRecords(NULL, idx, a->key, a->key, &it), kOk,
                    "Could not open iterator");
      if(err == kOk){
        ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                      "Could not retrieve record a");
        if(err == kOk){
          ASSERT_EQUALS(0, RecordCmp(*a,*tmp),
                        "The original and the retrieved record a do not match");
          Release(tmp);
        }
        
        ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                      "Iterator did not report an end of range");
        if(err == kOk){
          Release(tmp);
        }
        
        ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
      }
      
      // Update record a
      Block payload;
      SetValue(payload, "new payload");
      ASSERT_EQUALS(err = UpdateRecord(NULL, idx, a, &payload, 0), kOk,
                    "Could not update record a");
      a->payload = payload;
      
      // Ensure that the record was successfully updated
      ASSERT_EQUALS(err = GetRecords(NULL, idx, a->key, a->key, &it), kOk,
                    "Could not open iterator");
      if(err == kOk){
        ASSERT_EQUALS(err = GetNext(it,&tmp), kOk,
                      "Could not retrieve record a (after updating)");
        if(err == kOk){
          ASSERT_EQUALS(0, RecordCmp(*a,*tmp),
                     "The the retrieved and the updated record a do not match");
          Release(tmp);
        }
        
        ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                      "Iterator did not report an end of range");
        if(err == kOk){
          Release(tmp);
        }
        
        ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
      }
      
      // Delete record b
      ASSERT_EQUALS(err = DeleteRecord(NULL, idx, b, 0), kOk,
                    "Could not delete record b");
      
      // Ensure that the record was successfully deleted
      ASSERT_EQUALS(err = GetRecords(NULL, idx, b->key, b->key, &it), kOk,
                    "Could not open iterator");
      if(err == kOk){
        ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                      "Iterator did not report an end of range");
        if(err == kOk){
          Release(tmp);
        }
        
        ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
      }
      
      // Close the index
      ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
    }
    
    // Delete the index
    ASSERT_EQUALS(DeleteIndex(AUTO_COMMIT_TEST_INDEX), kOk,
                  "Could not delete the index");
  }
  
  // Cleanup
  Release(a);
  Release(b);
};


// Test to ensure that the flags for UpdateRecord() and DeleteRecord()
// are handled properly
//
// This test attempts to modify the first record matching the given
// key (while ignoring the payload) as well as to modify full duplicates.
// It also makes sure that multiple records with different payloads but equal
// key can be modified at one go.
TEST(BulkTest){
  // Create some test records
  Record *a = CreateRecordBulk(1,"record a");
  Record *b = CreateRecordBulk(1,"record b");
  Record *c = CreateRecordBulk(2,"record c");
  Record *d = CreateRecordBulk(2,"record d");

  // A record that will be used to test kIgnorePayload
  Record *ip = CreateRecordBulk(1,"ignore payload");
  
  // Create a simple index with keys comprising 1 short attribute
  KeyType schema = {kShort};
  ErrorCode err = CreateIndex(BULK_TEST_INDEX, COUNT_OF(schema), schema);
  
  ASSERT_EQUALS(err, kOk, "Could not create the new index");
  if(err == kOk) {
    // Begin a new Transaction
    Transaction *tx;
    ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                  "Could not start a new transaction");
    
    if(err == kOk){
      Index *idx;
      // Open the created index
      ASSERT_EQUALS(err = OpenIndex(BULK_TEST_INDEX, &idx), kOk,
                    "Could not open the created index");
      
      if(err == kOk){
        Iterator *it;
        Record *tmp;
        
        // Insert the test records
        ASSERT_EQUALS(InsertRecord(tx,idx,a), kOk, "Could not insert record a");
        ASSERT_EQUALS(InsertRecord(tx,idx,b), kOk, "Could not insert record b");
        ASSERT_EQUALS(InsertRecord(tx,idx,c), kOk, "Could not insert record c");
        ASSERT_EQUALS(InsertRecord(tx,idx,c), kOk,
                      "Could not insert record c (duplicate)");
        ASSERT_EQUALS(InsertRecord(tx,idx,d), kOk, "Could not insert record d");
        
        // Try to update 1 record with key (1)
        Block payload;
        SetValue(payload, "new payload");
        ASSERT_EQUALS(err = UpdateRecord(tx,idx,ip,&payload,kIgnorePayload), kOk,
                      "Could not update using kIgnorePayload");
        
        // Check that only one record has been updated
        if(err == kOk){
          ASSERT_EQUALS(err = GetRecords(tx, idx, ip->key, ip->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            ASSERT_EQUALS(err = GetNext(it, &tmp), kOk,
                          "Could not retrieve record a/b");
            if(err == kOk){
              if((RecordCmp(*a,*tmp) == 0) || (RecordCmp(*b,*tmp) == 0)){
                Release(tmp);
                
                // The next record should be the updated one
                ASSERT_EQUALS(err = GetNext(it, &tmp), kOk,
                              "Could not retrieve the updated record");
                if(err == kOk){
                  ASSERT_EQUALS(BlockCmp(tmp->payload, payload), 0,
                                "The retrieved record's payload does not match "
                                "the assigned payload (after update)");
                  Release(tmp);
                  
                  ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                                "Iterator did not report an end of range");
                  if(err == kOk){
                    Release(tmp);
                  }
                }

              } else {
                int r;
                ASSERT_EQUALS(r = BlockCmp(tmp->payload, payload), 0,
                              "The retrieved record's payload does not match "
                              "the assigned payload (after update)");
                
                if(r == 0){
                  // The next record should be either record a or record b
                  ASSERT_EQUALS(err = GetNext(it, &tmp), kOk,
                                "Could not retrieve the record a/b");
                  
                  if(err == kOk){
                    ASSERT_EQUALS(((RecordCmp(*a,*tmp) == 0) || (RecordCmp(*b,*tmp) == 0)), true,
                                  "The retrieved record neither matches "
                                  "record a nor record b");
                    Release(tmp);
                  }
                  
                  ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                                "Iterator did not report an end of range");
                  if(err == kOk){
                    Release(tmp);
                  }
                }
              }
            }
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
        }
        
        // Try to update record c (including its duplicate)
        ASSERT_EQUALS(err = UpdateRecord(tx,idx,c,&payload,kMatchDuplicates),
                      kOk, "Could not update using kMatchDuplicates");
        
        // Check that both records have been updated and ensure
        // that record d was not changed (as it is not a full duplicate) 
        if(err == kOk){
          ASSERT_EQUALS(err = GetRecords(tx, idx, c->key, c->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            bool found_d = false;
            for(int i = 0; i < 3; i++){
              ASSERT_EQUALS(err = GetNext(it, &tmp), kOk,
                            "Could not retrieve next record with key (2)");
              if(err != kOk)
                break;
              if(!found_d && (RecordCmp(*d,*tmp) == 0)){
                found_d = true;
              } else {
                ASSERT_EQUALS(BlockCmp(tmp->payload, payload), 0,
                              "The retrieved record's payload does not match "
                              "the assigned payload (after update)");
              }
              Release(tmp);
            }
            
            ASSERT_EQUALS(found_d, true, "Record d has not been found");
            
            if(err == kOk){
              ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                            "Iterator did not report an end of range");
              if(err == kOk){
                Release(tmp);
              }
            }
            
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
        }
        
        // Try to delete one record with key (1)
        ASSERT_EQUALS(err = DeleteRecord(tx,idx,ip,kIgnorePayload), kOk,
                      "Could not delete using kIgnorePayload");
        
        // Verify that exactly one record with key (1) was deleted
        if(err == kOk){
          ASSERT_EQUALS(err = GetRecords(tx, idx, ip->key, ip->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            ASSERT_EQUALS(err = GetNext(it, &tmp), kOk,
                          "Could not retrieve the remaining record "
                          "(after deletion using kIgnorePayload)");
            Release(tmp);
            
            if(err == kOk){
              ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                            "Iterator did not report an end of range");
              if(err == kOk){
                Release(tmp);
              }
            }
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
        }
        
        // Try to delete all records with key (2)
        ASSERT_EQUALS(err = DeleteRecord(tx,idx,d,kIgnorePayload | kMatchDuplicates), kOk,
                  "Could not delete using kIgnorePayload and kMatchDuplicates");
        
        // Verify that all record with key 2 have been deleted
        if(err == kOk){
          ASSERT_EQUALS(err = GetRecords(tx, idx, d->key, d->key, &it), kOk,
                        "Could not open iterator");
          if(err == kOk){
            ASSERT_EQUALS(err = GetNext(it,&tmp), kErrorNotFound,
                          "Iterator did not report an end of range");
            if(err == kOk){
              Release(tmp);
            }
            ASSERT_EQUALS(kOk, CloseIterator(&it), "Could not close iterator");
          }
        }
        
        // Close the index
        ASSERT_EQUALS(CloseIndex(&idx), kOk, "Could not close index");
      }
      // Abort the transaction
      ASSERT_EQUALS(AbortTransaction(&tx), kOk, "Could not abort transaction");
    }
      
    // Delete the index
    ASSERT_EQUALS(DeleteIndex(BULK_TEST_INDEX), kOk, 
                  "Could not delete the index");
  }

  // Cleanup
  Release(a);
  Release(b);
  Release(c);
  Release(d);
};


// Test to ensure the right usage of error codes
//
// This test tests some common situations in which errors occure and
// makes sure that the right error codes are returned.
TEST(ErrorHandlingTest){
  
  // Create a test record
  Record *a = CreateRecordErrorHandling(1,"record a");
  
  // Create a record with an incompatible key
  Record *ik = CreateRecordIsolation(2,"incompatible record");
  
  // Create a simple index with keys comprising 1 short attribute
  KeyType schema = {kShort};
  ErrorCode err = CreateIndex(ERROR_HANDLING_TEST_INDEX, COUNT_OF(schema), schema);
  
  ASSERT_EQUALS(err, kOk, "Could not create the new index");
  
  // Ensure that index names are unique
  ASSERT_EQUALS(CreateIndex(ERROR_HANDLING_TEST_INDEX, COUNT_OF(schema), schema), kErrorIndexExists,
                "CreateIndex() does not return kErrorIndexExists "
                "when attempting to create an index with a name already in use");
  
  if(err == kOk){
    Index* idx = NULL;
    
    // Open an unknown index
    ASSERT_EQUALS(OpenIndex(NON_EXISTENT_INDEX, &idx), kErrorUnknownIndex,
                  "OpenIndex() does not return kErrorUnknownIndex "
                  "when attempting to open an non-existent index");
    
    // Open the test index
    ASSERT_EQUALS(err = OpenIndex(ERROR_HANDLING_TEST_INDEX, &idx), kOk,
                  "Could not open index");
    
    if(err == kOk){
      
      // Start a new transaction
      Transaction *tx;
      ASSERT_EQUALS(err = BeginTransaction(&tx), kOk,
                    "Could not start a new transaction");
      if(err == kOk){
        
        // Insert the test record
        ASSERT_EQUALS(err = InsertRecord(tx, idx, a), kOk,
                      "Could not insert the test record");
        
        if(err == kOk){
          // Make sure that DeleteIndex() returns kErrorOpenTransactions
          // when attempting to delete an index that has been modified
          // by a currently unresolved transaction
          ASSERT_EQUALS(err = DeleteIndex(ERROR_HANDLING_TEST_INDEX),
                        kErrorOpenTransactions,
                        "DeleteIndex() does not return kErrorOpenTransactions "
                        "when attempting to delete an index that has been "
                        "modified by a currently unresolved (neither committed "
                        "nor aborted) transaction");

          if(err == kOk){
            // Stop the current test, as the further course of
            // this test is undefined
            return;
          }
        }
        
        // Close the transaction
        ASSERT_EQUALS(err = AbortTransaction(&tx), kOk,
                      "Could not abort transaction");
        
        if(err == kOk){
          // Try to abort a transaction that has already been closed
          ASSERT_EQUALS(AbortTransaction(&tx), kErrorTransactionClosed,
                        "AbortTransaction() does not return "
                        "kErrorTransactionClosed when attempting to abort "
                        "a transaction that was already closed");
          
          // Try to commit a transaction that has already been closed
          ASSERT_EQUALS(CommitTransaction(&tx), kErrorTransactionClosed,
                        "CommitTransaction() does not return "
                        "kErrorTransactionClosed when attempting to commit "
                        "a transaction that was already closed");
        }
      }
      
      // Ensure that InsertRecord() returns kErrorIncompatibleKey when
      // attempting to insert a record with an incompatible key
      ASSERT_EQUALS(InsertRecord(NULL,idx,ik),kErrorIncompatibleKey,
                    "InsertRecord() does not return kErrorIncompatibleKey "
                    "when attempting to insert an incompatible record");
      
      // Ensure that UpdateRecord() returns kErrorIncompatibleKey when
      // attempting to insert a record with an incompatible key
      Block payload;
      ASSERT_EQUALS(UpdateRecord(NULL,idx,ik,&payload,0),kErrorIncompatibleKey,
                    "UpdateRecord() does not return kErrorIncompatibleKey "
                    "when attempting to update an incompatible record");
      
      // Ensure that DeleteRecord() returns kErrorIncompatibleKey when
      // attempting to delete a record with an incompatible key
      ASSERT_EQUALS(DeleteRecord(NULL,idx,ik,0),kErrorIncompatibleKey,
                    "DeleteRecord() does not return kErrorIncompatibleKey "
                    "when attempting to delete an incompatible record");
      
      // Ensure that GetRecords() returns kErrorIncompatibleKey when
      // attempting to retrieve records using an incompatible key
      Iterator *it;
      ASSERT_EQUALS(GetRecords(NULL,idx,ik->key,ik->key,&it),kErrorIncompatibleKey,
                    "GetRecords() does not return kErrorIncompatibleKey "
                    "when attempting to retrieve records using an "
                    "incompatible key");
      
      // Open a new iterator
      ASSERT_EQUALS(err = GetRecords(NULL,idx,a->key,a->key,&it),kOk,
                    "Could not open iterator");
      
      if(err == kOk){
        // Close the iterator
        ASSERT_EQUALS(err = CloseIterator(&it),kOk, "Could not close iterator");
        
        if(err == kOk){
          // Ensure that GetNext() returns kErrorIteratorClosed
          // when attempting to retrieve a record using an iterator that
          // has already been closed
          Record *tmp;
          ASSERT_EQUALS(GetNext(it,&tmp),kErrorIteratorClosed,
                        "GetNext() does not return kErrorIteratorClosed "
                        "when attempting to retrieve records from an iterator "
                        "that has already been closed");
          
          // Ensure that CloseIterator returns kErrorIteratorClosed
          // when attempting to close an iterator that has already been closed
          ASSERT_EQUALS(CloseIterator(&it),kErrorIteratorClosed,
                        "CloseIterator() does not return kErrorIteratorClosed "
                        "when attempting to close an iterator that has already "
                        "been closed");
        }
      }
      
      // Close the test index
      ASSERT_EQUALS(err = CloseIndex(&idx), kOk, "Could not close index");
      
      if(err == kOk){
        // Close an index that has been already closed
        ASSERT_EQUALS(err = CloseIndex(&idx), kErrorUnknownIndex,
                      "CloseIndex() does not return kErrorUnkownIndex "
                      "when attempting to close an index that has been "
                      "already closed");
        
        // Ensure that InsertRecord() returns kErrorUnknownIndex when
        // called without passing a valid index handle
        ASSERT_EQUALS(InsertRecord(NULL,idx,a),kErrorUnknownIndex,
                      "InsertRecord() does not return kErrorUnknownIndex "
                      "when passing an invalid index handle");
        
        // Ensure that UpdateRecord() returns kErrorUnknownIndex when
        // called without passing a valid index handle
        ASSERT_EQUALS(UpdateRecord(NULL,idx,a,&payload,0),kErrorUnknownIndex,
                      "UpdateRecord() does not return kErrorUnknownIndex "
                      "when passing an invalid index handle");
        
        // Ensure that DeleteRecord() returns kErrorUnknownIndex when
        // called without passing a valid index handle
        ASSERT_EQUALS(DeleteRecord(NULL,idx,a,0),kErrorUnknownIndex,
                      "DeleteRecord() does not return kErrorUnknownIndex "
                      "when passing an invalid index handle");
        
        // Ensure that GetRecords() returns kErrorUnknownIndex when
        // called without passing a valid index handle
        ASSERT_EQUALS(GetRecords(NULL,idx,a->key,a->key,&it),kErrorUnknownIndex,
                      "GetRecords() does not return kErrorUnknownIndex "
                      "when passing an invalid index handle");
      }
    }
    
    // Delete the index
    ASSERT_EQUALS(DeleteIndex(ERROR_HANDLING_TEST_INDEX), kOk,
                  "Could not delete the index");
    
    // Ensure that non-existent indices cannot be deleted
    ASSERT_EQUALS(DeleteIndex(ERROR_HANDLING_TEST_INDEX), kErrorUnknownIndex,
                  "DeleteIndex() does not return kErrorUnkownIndex "
                  "when attempting to delete a non-existent index");
  }
  
  // Cleanup
  Release(a);
}
