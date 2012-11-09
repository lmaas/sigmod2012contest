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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

#include "json_importer.h"

// Constructs a new JSON null value from the given JSON tokenizer
JSONNull::JSONNull(JSONTokenizer &tokenizer){
  std::stringstream str;
  char c = tokenizer.next_clean();
  while((!tokenizer.eof()) && c >= 'a' && c <= 'z'){
    str << c;
    c = tokenizer.next();
  }
  tokenizer.back();

  if(str.str() != "null"){
    throw JSONException("A JSONNull can only be 'null'.");
  }
};

// Constructs a new JSON null value from the given input string
JSONNull::JSONNull(const std::string &source){
  if(source != "null"){
    throw JSONException("A JSONNull can only be 'null'.");
  }
};

// Constructs a new JSON boolean value from the given JSON tokenizer
// TODO: Is there any better way to scan?
JSONBoolean::JSONBoolean(JSONTokenizer &tokenizer){

  std::stringstream str;
  char c = tokenizer.next_clean();
  while((!tokenizer.eof()) && c >= 'a' && c <= 'z'){
    str << c;
    c = tokenizer.next();
  }
  tokenizer.back();

  if(str.str() == "true"){
    value_ = true;
  } else if(str.str() == "false"){
    value_ = false;
  } else {
    throw JSONException("A JSONBoolean must be either 'true' or 'false'");
  }
}

// Constructs a new JSON boolean value from the given input string
JSONBoolean::JSONBoolean(const std::string &source){
  if(source == "true"){
    value_ = true;
  } else if(source == "false"){
    value_ = false;
  } else {
    throw JSONException("A JSONBoolean must be either 'true' or 'false'");
  }
}

// Constructs a new JSON number from the given JSON tokenizer
JSONNumber::JSONNumber(JSONTokenizer &tokenizer){
  char c = tokenizer.next_clean();
  std::stringstream str;
  is_double = false;
  if ((c >= '0' && c <= '9') || (c == '.') || (c == '-') || (c == '+')) {
    while(!tokenizer.eof()){
      bool success = true;
      str << c;
      switch(c){
        case '.':
          if(is_double){
            success = false;
          } else {
            is_double = true;
          }
          break;
        case 'e':
        case 'E':
          is_double = true;
          char c = tokenizer.next();
          str << c;
          if(!((c >= '0' && c <= '9') || c == '-' || c == '+'))
            success = false;
          break;
      }

      if(!success)
        throw JSONException("The given string is not a valid JSON number");

      c = tokenizer.next();
      if(!((c >= '0' && c <= '9') || (c == '.') || (c=='e') || (c=='E'))){
        break;
      }
    }
    tokenizer.back();
    value_ = std::atof(str.str().c_str());
  } else {
    throw JSONException("The given string is not a valid JSON number");
  }
}

// Constructs a new JSON string from the given JSON tokenizer
JSONString::JSONString(JSONTokenizer &tokenizer){

  if(tokenizer.next_clean() != '"'){
    throw JSONException("A JSONString must begin with '\"'");
  }

  std::stringstream str;
  char c;
  while(true){
    switch(c = tokenizer.next()){
      case '"':
        value_ = str.str();
        return;
      case '\0':
        throw JSONException("A JSONString must end with '\"'");
      case '\n':
        throw JSONException("A JSONString must not contain linebreaks");
        return;
      case '\\':
        switch(c = tokenizer.next()){
          case '"':
            str << '"';
            break;
          case '\\':
            str << '\\';
            break;
          case '/':
            str << '/';
            break;
          case 'b':
            str << '\b';
            break;
          case 'f':
            str << '\f';
            break;
          case 'n':
            str << '\n';
            break;
          case 'r':
            str << '\r';
            break;
          case 't':
            str << '\t';
            break;
            // case 'u':
            //  NOTE: Not handled yet
          default:
            str << '\\' << c;
        }
        break;
      default:
        str << c;
    }
  }
}

// Constructs an JSON Object from a JSON source string
JSONObject::JSONObject(std::string source){
  std::istringstream str(source);
  JSONTokenizer tokenizer(str);
  Init(tokenizer);
}

// Constructs an JSON Object from a JSON tokenizer
JSONObject::JSONObject(JSONTokenizer &tokenizer){
  Init(tokenizer);
}

// Initializes the JSONobject form the given Tokenizer
void JSONObject::Init(JSONTokenizer &tokenizer){
  char c;
  // Find the start
  if((c=tokenizer.next_clean()) != '{'){
    throw JSONException("A JSONObject must begin with '{'");
  }

  while(true){
    switch(tokenizer.next_clean()){
      case '}':
        return;
      case '\0':
        throw JSONException("A JSONObject must be closed with '}'");
    }
    tokenizer.back();

    // A JSON object contains a list of keys...
    JSONString* key = new JSONString(tokenizer);

    // ... followed by a colon...
    if(tokenizer.next_clean() != ':'){
      throw JSONException("Expected a ':' after a key inside a JSONObject");
    }

    // ...and the value
    values.insert(std::pair<std::string,JSONValue*>(key->value(),tokenizer.NextValue()));

    delete key;

    // Key/Value pairs are seperated commas
    switch(tokenizer.next_clean()){
      case '}':
        return;
      case ',':
        break;
      default:
        throw JSONException("Key/value pairs in JSONObjects have to be seperated by ','");
        return;
    }
  }
}

// Adds a new key/value pair to the object
void JSONObject::Put(std::string key, JSONValue* value){
  values.insert(std::pair<std::string,JSONValue*>(key,value));
}

// Adds a new key/value pair to the object
void JSONObject::Put(std::string key, std::string value){
  Put(key, new JSONString(value));
}

// Constructs an JSON Array from a given JSON tokenizer
JSONArray::JSONArray(JSONTokenizer &tokenizer){
  if(tokenizer.next_clean() != '['){
    throw JSONException("A JSONArray must begin with '['");
  }

  while(true){
    switch(tokenizer.next_clean()){
      case ']':
        return;
      case '\0':
        throw JSONException("A JSONArray must end with ']'");
      default:
        tokenizer.back();
        values.push_back(tokenizer.NextValue());
    }

    switch(tokenizer.next_clean()){
      case ']':
        return;
      case ',':
        break;
      default:
        throw JSONException("Values inside arrays need to be seperated by ','");
    }
  }
}

// Constructs a new JSON tokenizer
JSONTokenizer::JSONTokenizer(std::istream &stream):stream_(stream){
}

// Backs up one character
void JSONTokenizer::back(){
  stream_.unget();
}

// Gets the next character
char JSONTokenizer::next(){
  return stream_.get();
}

// Extracts characters and discards them until the delimiter is found
void JSONTokenizer::SkipTo(char delimiter){
  char c = next();
  while(!eof() && c!=delimiter){
    c=next();
  }
}

// Gets the next character while skipping characters and comments
char JSONTokenizer::next_clean(){
  while(!eof()){
    char c = next();
    if(c == '/'){
      c = next();
      if(c=='/'){
        SkipTo('\n');
      }else{
        back();
        return '/';
      }
    }
    else if((c > ' ')||(c == '\0')){
      return c;
    }
  }
  return '\0';
}

// Extracts the next JSONValue
JSONValue *JSONTokenizer::NextValue(){
  switch(next_clean()){
    case '"':
      back();
      return new JSONString(*this);
      break;
    case '{':
      back();
      return new JSONObject(*this);
      break;
    case '[':
      back();
      return new JSONArray(*this);
    case 't':
    case 'f':
      back();
      return new JSONBoolean(*this);
    case 'n':
      back();
      return new JSONNull(*this);
    case '+':
    case '-':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      back();
      return new JSONNumber(*this);
      // TODO: Look for a better way to decide
    default:
      throw JSONException("The given string is not a valid JSON value.");
  }
  return NULL;
}
