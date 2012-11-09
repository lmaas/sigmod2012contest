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

#include <string.h>
#include <cstdlib>

#include "util.h"

// Compares two Berkeley DB keys (used for the b-tree)
int keycmp(Db *db, const Dbt *a,  const Dbt *b){
  // Retrieve the index schema from key a
  IndexSchema *is;
  memcpy(&is, a->get_data(), sizeof(is));
    
  // Compare the keys
  return KeyCmp(is,a,b);
}

// Compares two Berkeley DB keys using the given schema
//
// If full is true the result will be 0 if every attribute of key a
// is smaller than or equal to key b (1 otherwise)
int KeyCmp(IndexSchema *is, const Dbt *a, const Dbt *b, bool full){
  
  // This union mimics the unnamed union defined inside the Attribute
  // struct in contest_interface.h
  typedef union {
    int32_t short_value;
    int64_t int_value;
    char char_value[MAX_VARCHAR_LENGTH+1];
  } Value;
  
  // Skip the first 64bit of each key (they are used to save the index schema)
  Value *av, *bv;
  char* ak = ((char*) a->get_data())+8;
  char* bk = ((char*) b->get_data())+8;
  
  // Compare the keys
  int result = 0;
  for(int i = 0; i < is->attribute_count(); i++){
    av = (Value*) (ak);
    bv = (Value*) (bk);
    result = 0;
    switch(is->type()[i]){
      case kShort:
        if( av->short_value< bv->short_value )
          result = -1;
        else if (av->short_value > bv->short_value )
          result = 1;
        ak+=4;
        bk+=4;
        break;
      case kInt:
        if(av->int_value < bv->int_value )
          result = -1;
        else if (av->int_value > bv->int_value )
          result = 1;
        ak+=8;
        bk+=8;
        break;
      case kVarchar:
        result = strcmp(av->char_value, bv->char_value);
        ak+=MAX_VARCHAR_LENGTH+1;
        bk+=MAX_VARCHAR_LENGTH+1;
      }
    if(!full && (result != 0)){
      return result;
    } else if(full && (result > 0)){
      return 1;
    }
  }

  return 0;
}