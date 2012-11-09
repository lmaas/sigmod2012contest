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

// Parses JSON strings with some small modifications
// (C++-Style comments; multiline support)

#ifndef BENCHMARK_CORE_IMPORTER_JSON_IMPORTER_H_
#define BENCHMARK_CORE_IMPORTER_JSON_IMPORTER_H_

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

class JSONTokenizer;

// A custom Exception class that represents errors related to processing
// the json document (e.g. parsing errors)
class JSONException: public std::runtime_error {
 public:
  JSONException(const std::string& msg = "") : std::runtime_error(msg) {}
};

// An enumeration that contains all different types of JSON values
typedef enum {
  kJSONNull,
  kJSONBoolean,
  kJSONNumber,
  kJSONString,
  kJSONArray,
  kJSONObject
} JSONType;

// An abstract base class representing a JSON Value
class JSONValue {
 public:
  // Constructor
  JSONValue(){};
  
  // Returns the type of this JSON Value
  virtual JSONType type() = 0;
};

// A JSON null value
class JSONNull : public JSONValue {
 public:
  // Constructs a new JSON null value
  JSONNull(){};
  
  // Constructs a new JSON null value from the given JSON tokenizer
  JSONNull(JSONTokenizer &tokenizer);
  
  // Constructs a new JSON null value from the given input string
  JSONNull(const std::string &source);
  
  // Returns the type of this JSON null value
  JSONType type() {return kJSONNull;}
};

// A JSON boolean value
class JSONBoolean : public JSONValue {
 public:
  // Constructs a new JSON boolean value from the given JSON tokenizer
  JSONBoolean(JSONTokenizer &tokenizer);
  
  // Constructs a new JSON boolean value from the given input string
  JSONBoolean(const std::string &source);
  
  // Sets the boolean value
  void value(bool value) {value_=value;}
  
  // Gets the boolean value
  bool value() const {return value_;}
  
  // Returns the type of this JSON boolean
  JSONType type(){return kJSONBoolean;}
  
 private:
  // The value of this boolean JSON value
  bool value_;
};

// A JSON number (double)
class JSONNumber : public JSONValue {
 public:
  // Constructs a new JSON number from the given JSON tokenizer
  JSONNumber(JSONTokenizer &tokenizer);
  
  // Sets the JSON number value
  void value(double value){value_=value;}
  
  // Returns the JSON number value
  double value() const {return value_;}
  
  // Returns the type of this JSON number
  JSONType type(){return kJSONNumber;}
  
 private:
  // Whether the given number contains any decimal places
  bool is_double;
  
  // The value of this number
  double value_;
};


// JSON String
class JSONString : public JSONValue{
 public:
  // Constructs a new JSON string with the given value
  JSONString(std::string value):value_(value){}
  
  // Constructs a new JSON string from the given JSON tokenizer
  JSONString(JSONTokenizer &tokenizer);
  
  // Sets the value of this string
  void value(const std::string &value){value_=value;}
  
  // Gets the value of this string
  std::string value() const {return value_;}
  
  // Returns the type of this JSON string
  JSONType type(){return kJSONString;}
  
 private:
  // The value of this string
  std::string value_;
};

// JSON Object
// 
// Note:
//   JSON objects are interpreted as a map (i.e. duplicates are not supported)
class JSONObject : public JSONValue {
 public:
  // Constructs an empty JSON Object
  JSONObject(){};
  
  // Constructs an JSON Object from a JSON source string
  JSONObject(std::string source);
  
  // Constructs an JSON Object from a JSON tokenizer
  JSONObject(JSONTokenizer &tokenizer);
  
  // Adds a new key/value pair to the object
  void Put(std::string key, JSONValue* value);
  void Put(std::string key, std::string value);
  
  // Returns the JSON value stored under the given key
  JSONValue* Get(const std::string key) const{
    std::map<std::string, JSONValue*>::const_iterator it = values.find(key);
    if(it == values.end())
      return NULL;
    return it->second;
  };
  
  // Returns whether the JSON object contains a key/value pair
  // with the given key
  bool Contains(const std::string key){
    return !(values.find(key) == values.end());
  }
  
  // Returns the type of this JSON object
  JSONType type() {return kJSONObject;}
  
  // TODO: Add an iterator to allow iteration over JSON objects
  
 private:
  // The key/value pairs stored inside this JSONobject
  std::map<std::string,JSONValue*> values;
  
  // Initializes the JSONobject form the given Tokenizer
  void Init(JSONTokenizer &tokenizer);
};

// JSON Array
class JSONArray : public JSONValue {
 public:
  // Constructs an empty JSON Array
  JSONArray(){};
  
  // Constructs an JSON Array from a given JSON tokenizer
  JSONArray(JSONTokenizer &tokenizer);
  
  // Returns the number of elements inside the array
  size_t size(){return values.size();}
  
  // Returns the JSONValue stored at the given index
  JSONValue *Get(unsigned int index){return values[index];}
  
  // Returns the type of this array
  JSONType type() {return kJSONArray;}
  
  // TODO: Add an iterator to allow iteration over JSON arrays
  
 private:  
  // The array's content
  std::vector<JSONValue*> values;
};

// A tokenizer that takes a input stream and extracts characters and tokens
// from it.
//
// JSONTokenizer objects are used to share a single input stream between
// multiple JSONValue objects.
class JSONTokenizer {
 public:
  // Constructs a new JSON tokenizer
  JSONTokenizer(std::istream &stream);
  
  // Gets the next character
  char next();
  
  // Gets the next character while skipping characters and comments
  char next_clean();
  
  // Backs up one character
  void back();
  
  // Extracts characters and discards them until the delimiter is found
  void SkipTo(char delimiter);
  
  // Returns the next character without extracting it
  char peek() const {return stream_.peek();}
  
  // Extracts the next JSONValue
  JSONValue *NextValue();
  
  // Returns true if the End Of File has been reached
  bool eof() const {return stream_.eof();}
  
 private:
  // The input stream used to extract the tokens
  std::istream &stream_;
};

#endif // BENCHMARK_CORE_IMPORTER_JSON_IMPORTER_H_

