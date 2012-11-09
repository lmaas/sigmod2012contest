#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "util.h"

// Create a record for the BasicTest
Record *CreateRecordBasic(int32_t attribute_1, int64_t attribute_2, const char* attribute_3, const char* payload){
  Attribute** a = (Attribute**) malloc(3*sizeof(Attribute*));
  
  a[0] = ShortAttribute(attribute_1);
  a[1] = IntAttribute(attribute_2);
  a[2] = VarcharAttribute(attribute_3);

  Record* record = (Record*) malloc(sizeof(Record));
  record->key.value = a;
  record->key.attribute_count = 3;
  SetValue(record->payload,payload);
  
  return record;
}

// Create a record fot the PartialMatchTest
Record *CreateRecordPartialMatch(int32_t attribute_1, int32_t attribute_2, int32_t attribute_3, const char* payload){
  Attribute** a = (Attribute**) malloc(3*sizeof(Attribute*));
  
  a[0] = ShortAttribute(attribute_1);
  a[1] = ShortAttribute(attribute_2);
  a[2] = ShortAttribute(attribute_3);
  
  Record* record = (Record*) malloc(sizeof(Record));
  record->key.value = a;
  record->key.attribute_count = 3;
  SetValue(record->payload,payload);
  
  return record;
}

// Create a new record for the RangeTest
Record *CreateRecordRange(int32_t attribute_1, const char* attribute_2, int64_t attribute_3, const char* payload){
  Attribute** a = (Attribute**) malloc(3*sizeof(Attribute*));
  
  a[0] = ShortAttribute(attribute_1);
  a[1] = VarcharAttribute(attribute_2);
  a[2] = IntAttribute(attribute_3);
  
  Record* record = (Record*) malloc(sizeof(Record));
  record->key.value = a;
  record->key.attribute_count = 3;
  SetValue(record->payload,payload);
  
  return record;
}

// Create a new record for the IsolationTest
Record *CreateRecordIsolation(int64_t attribute_1, const char* payload){
  Attribute** a = (Attribute**) malloc(sizeof(Attribute*));
  
  a[0] = IntAttribute(attribute_1);
  
  Record* record = (Record*) malloc(sizeof(Record));
  record->key.value = a;
  record->key.attribute_count = 1;
  SetValue(record->payload,payload);
  
  return record;
}

// Create a record for the AutoCommitTest
Record *CreateRecordAutoCommit(int32_t attribute_1, const char* payload){
  Attribute** a = (Attribute**) malloc(sizeof(Attribute*));
  
  a[0] = ShortAttribute(attribute_1);
  
  Record* record = (Record*) malloc(sizeof(Record));
  record->key.value = a;
  record->key.attribute_count = 1;
  SetValue(record->payload,payload);
  
  return record;
}

// Create a record for the BulkTest
Record *CreateRecordBulk(int32_t attribute_1, const char* payload){
  return CreateRecordAutoCommit(attribute_1, payload);
}

// Create a record for the ErrorHandlingTest
Record *CreateRecordErrorHandling(int32_t attribute_1, const char* payload){
  return CreateRecordAutoCommit(attribute_1, payload);
}

// Set the value of a Block
void SetValue(Block &block, const char* value){
  int size = strlen(value)+1;
  block.size = size;
  block.data = malloc(size);
  memcpy(block.data, value, size);
}

// Create a new attribute of type SHORT
Attribute *ShortAttribute(int32_t value){
  Attribute* attribute = (Attribute*) malloc(sizeof(Attribute));
  attribute->type = kShort;
  attribute->short_value = value;
  return attribute;
}

// Create a new attribute of type INT
Attribute *IntAttribute(int64_t value){
  Attribute* attribute = (Attribute*) malloc(sizeof(Attribute));
  attribute->type = kInt;
  attribute->int_value = value;
  return attribute;
}

// Create a new attribute of type VARCHAR
Attribute *VarcharAttribute(const char* value){
  Attribute* attribute = (Attribute*) malloc(sizeof(Attribute));
  attribute->type = kVarchar;
  strcpy(attribute->char_value, value);
  return attribute;
}

// Compare two Keys
int KeyCmp(const Key& a, const Key& b){
  int r = a.attribute_count-b.attribute_count;
  if(r != 0)
    return r;
  
  for(int i = 0; i < a.attribute_count; i++){
    assert(a.value[i]);
    assert(b.value[i]);
    switch(a.value[i]->type){
      case kShort:
        if(a.value[i]->short_value < b.value[i]->short_value)
          return -1;
        if(a.value[i]->short_value > b.value[i]->short_value)
          return 1;
        break;
      case kInt:
        if(a.value[i]->int_value < b.value[i]->int_value)
          return -1;
        if(a.value[i]->int_value > b.value[i]->int_value)
          return 1;
        break;
      case kVarchar:
        r = strcmp(a.value[i]->char_value,b.value[i]->char_value);
        if(r != 0)
          return r;
        break;
      default:
        assert(false);
    }
  }
  return 0;
}

// Compare two Blocks
int BlockCmp(const Block& a, const Block& b){
  const int min_size = (a.size < b.size) ? a.size : b.size;
  int r = memcmp(a.data,b.data,min_size);
  if(r == 0){
    if(a.size < b.size)
      return -1;
    if(a.size > b.size)
      return 1;
    return 0;
  }
  return r;  
}

// Compare two Records
int RecordCmp(const Record& a, const Record& b){
  int r = KeyCmp(a.key,b.key);
  if(r == 0){
    return BlockCmp(a.payload,b.payload);
  }
  return r;
}

// Free all resources that are currently used by record
// (this also includes the memory used to store the payload)
void Release(Record* record){
  if(record){
    free(record->payload.data);
    for(int i = 0; i < record->key.attribute_count; i++){
      free(record->key.value[i]);
    }
    free(record->key.value);
  }
}
