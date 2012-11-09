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
 
 Current version: 1.0 (released February 14, 2011)
 
 Version history:
 - 1.0 Initial release (February 14, 2011) 
*/

/** @file
 Provides a set of helper functions used by the test cases defined in tests.cc
 */

#include <common/macros.h>
#include <contest_interface.h>

#ifndef _UNITTESTS_UTIL_H_
#define _UNITTESTS_UTIL_H_

// Create a new record for the BasicTest
Record *CreateRecordBasic(int32_t attribute_1, int64_t attribute_2, const char* attribute_3, const char* payload);

// Create a new record for the PartialMatchTest
Record *CreateRecordPartialMatch(int32_t attribute_1, int32_t attribute_2, int32_t attribute_3, const char* payload);

// Create a new record for the RangeTest
Record *CreateRecordRange(int32_t attribute_1, const char* attribute_2, int64_t attribute_3, const char* payload);

// Create a new record for the IsolationTest
Record *CreateRecordIsolation(int64_t attribute_1, const char* payload);

// Create a record for the AutoCommitTest
Record *CreateRecordAutoCommit(int32_t attribute_1, const char* payload);

// Create a record for the BulkTest
Record *CreateRecordBulk(int32_t attribute_1, const char* payload);

// Create a record for the ErrorHandlingTest
Record *CreateRecordErrorHandling(int32_t attribute_1, const char* payload);

// Set the value of a Block
void SetValue(Block &block, const char* value);

// Create a new attribute of type SHORT
Attribute *ShortAttribute(int32_t value);

// Create a new attribute of type INT
Attribute *IntAttribute(int64_t value);

// Create a new attribute of type VARCHAR
Attribute *VarcharAttribute(const char* value);

// Compare two Keys
int KeyCmp(const Key& a, const Key& b);

// Compare two Blocks
int BlockCmp(const Block& a, const Block& b);

// Compare two Records
int RecordCmp(const Record& a, const Record& b);

// Free all resources that are currently used by record
// (this also includes the memory used to store the payload)
void Release(Record* record);

#endif // _UNITTESTS_UTIL_H_
