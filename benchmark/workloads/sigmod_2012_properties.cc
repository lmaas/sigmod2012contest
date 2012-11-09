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
#include <fstream>
#include <iostream>
#include <sstream>

#include "contest_interface.h"

#include "core/generator.h"
#include "core/generators/uniform_generator.h"
#include "core/generators/normal_generator.h"
#include "core/generators/constant_generator.h"
#include "core/generators/zipf_generator.h"
#include "core/generators/sequence_generator.h"
#include "core/generators/constant_string_generator.h"
#include "core/generators/fixed_length_string_generator.h"
#include "core/importers/json_importer.h"
#include "core/utils/rngs/xor_shift_number_generator.h"
#include "core/utils/size.h"

#include "sigmod_2012_properties.h"

#define KEYSTORE_CAPACITY 1.5

// Converts the given string representation of a filesize into its integer
// representation (in byte)
// Returns -1 if the given string could not be
int GetSize(const std::string &string){
  std::stringstream str;
  int value = 1;
  for(unsigned int i = 0; i < string.size(); i++){
    switch(string[i]){
      case ' ':
        break;
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
        str << string[i];
        break;
      default:
        if(str.str().size() < 1)
          return -1;

        if(i != 'B'){
          if(i<string.size()-1){
             switch(string[i]){
              case 'K':
                // next character needs to be B
                i++;
                if(string[i]=='B'){
                  value = 1 KB;
                } else {
                  return -1;
                }
                break;
              case 'M':
                // Next character needs to be B
                i++;
                if(string[i]=='B'){
                  value = 1 MB;
                } else {
                  return -1;
                }
                break;
              case 'G':
                // Next character needs to be B
                i++;
                if(string[i]=='B'){
                  value = 1 GB;
                } else {
                  return -1;
                }
                break;
              default:
                return -1;
            }
          } else {
            return -1;
          }
        }
    }
  }
  return std::atoi(str.str().c_str())*value;
}

// Loads the properties from a file
SIGMOD2012Properties *SIGMOD2012Properties::LoadFromFile(Logger &logger,
                                                         std::string filename,
                                                         unsigned int seed){
  SIGMOD2012Properties *result = new SIGMOD2012Properties();

  // Open the file
  std::ifstream file (filename.c_str());
  if (!file.is_open()){
    logger.Error("Could not open workload file: "+filename);
    return NULL;
  }

  // Read the configuration from the file
  try{
    JSONTokenizer tokenizer(file);
    JSONObject json(tokenizer);

    // Get basic settings
    JSONValue *value;

    if((value=json.Get("extensive statistics"))&&(value->type()==kJSONBoolean))
      result->extensive_stats(((JSONBoolean*)value)->value());

    // Range portion
    if((value=json.Get("range portion")) && (value->type()==kJSONNumber))
      result->range_portion(((JSONNumber*)value)->value());

    // Point portion
    if((value=json.Get("point portion")) && (value->type()==kJSONNumber))
      result->point_portion(((JSONNumber*)value)->value());

    // Update portion
    if((value=json.Get("update portion")) && (value->type()==kJSONNumber))
      result->update_portion(((JSONNumber*)value)->value());

    // Insert portion
    if((value=json.Get("insert portion")) && (value->type()==kJSONNumber))
      result->insert_portion(((JSONNumber*)value)->value());

    // Delete portion
   if((value=json.Get("delete portion")) && (value->type()==kJSONNumber))
      result->delete_portion(((JSONNumber*)value)->value());



#ifdef DEBUG_
    logger.Debug("range portion is "+lexical_cast(result->range_portion()));
    logger.Debug("point portion is "+lexical_cast(result->point_portion()));
    logger.Debug("update portion is "+lexical_cast(result->update_portion()));
    logger.Debug("insert portion is "+lexical_cast(result->insert_portion()));
    logger.Debug("delete portion is "+lexical_cast(result->delete_portion()));
    logger.Debug("extensive statistics are "+
                 lexical_cast((result->extensive_stats()?"ON":"OFF")));
#endif

    // Get the index definitions
    if((value=json.Get("indices")) && value->type()==kJSONArray){
      JSONArray* array = (JSONArray*)value;

      int seed_offset = 0;
      for(unsigned int i = 0; i < array->size(); i++){
        if((value=array->Get(i)) && (value->type()==kJSONObject)){
          JSONObject* object = (JSONObject*)value;

          // Get the name
          std::string name;
          if((value=object->Get("name"))&&(value->type()==kJSONString)){
            name = ((JSONString*)value)->value();
          } else {
            // TODO: handle random names!
            logger.Error("Index definitions need to contain a String "
                         "called \"name\"");
            return NULL;
          }

          // Get the index size
          std::string string;
          int size;
          if((value=object->Get("size"))&&(value->type()==kJSONString)){
            string = ((JSONString*)value)->value();
            size = GetSize(string);
            if(size == -1){
              logger.Error("The provided index size is not valid");
              return NULL;
            }
          } else if(value&&(value->type()==kJSONNumber)){
            size = ((JSONNumber*)value)->value();
          } else {
            logger.Error("Index definitions have to contain the index size "
                         "(\"size\"))");
            return NULL;
          }

          // Get the payload size
          int payload_size;
          if((value=object->Get("payload size"))&&(value->type()==kJSONString)){
            string = ((JSONString*)value)->value();
            payload_size = GetSize(string);
            if(payload_size == -1){
              logger.Error("The provided payload size is not valid");
              return NULL;
            }
          } else if(value&&(value->type()==kJSONNumber)){
            payload_size = ((JSONNumber*)value)->value();
          } else {
            logger.Error("Index definitions need to contain the payload size "
                         "(\"payload size\")");
            return NULL;
          }

          // Get the keystore capacity factor
          double keystore_capacity = KEYSTORE_CAPACITY;
          if((value=object->Get("keystore capacity")) &&
             (value->type()==kJSONNumber)){
            keystore_capacity = ((JSONNumber*)value)->value();
          };

          // Get the attributes
          int dimensions = 0;
          if((value = object->Get("attributes")) &&
             (value->type()==kJSONArray)){
            JSONArray *attributes = (JSONArray*) value;

            if((dimensions = attributes->size()) < 1){
              logger.Error("Index definitions need to contain "
                           "at least 1 attribute");
              return NULL;
            }

            AttributeType* types = new AttributeType[dimensions];
            Generator** generators = new Generator*[dimensions];

            for(unsigned int j = 0; j < attributes->size(); j++){
              if((value = attributes->Get(j)) &&(value->type() == kJSONObject)){
                JSONObject* attribute = (JSONObject*) value;

                // Initialize the rng for this attribute
                RandomNumberGenerator *rng =
                  new XORShiftNumberGenerator(seed+(seed_offset++));

                // Get the attribute type
                if((value=attribute->Get("type")) &&
                   (value->type() == kJSONString)){
                  if(((JSONString*)value)->value()=="SHORT"){
                    types[j] = kShort;
                  } else if(((JSONString*)value)->value()=="INT"){
                    types[j] = kInt;
                  } else if(((JSONString*)value)->value()=="VARCHAR"){
                    types[j] = kVarchar;
                  } else {
                    logger.Error("The attribute type needs to be either "
                                 "\"SHORT\", \"INT\" or \"VARCHAR\"");
                    return NULL;
                  }
                } else {
                  logger.Error("Attributes need to contain their \"type\"");
                  return NULL;
                }

                // Get the attribute generator
                if((value=attribute->Get("generator")) &&
                   (value->type() == kJSONObject)){
                  JSONObject *generator = (JSONObject*) value;

                  // Get the Generator type
                  if((value=generator->Get("type")) &&
                     (value->type() == kJSONString)){
                    if((types[j] == kInt) || (types[j] == kShort)){
                      if(((JSONString*)value)->value()=="NORMAL"){
                        // Get the mean
                        int mu;
                        if((value=generator->Get("mean")) &&
                           (value->type() == kJSONNumber)){
                          mu = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"NORMAL\" need to contain the "
                                       "\"mean\"");
                          return NULL;
                        }

                        // Get the standard deviation
                        unsigned int sigma;
                        if((value=generator->Get("standard deviation"))
                           && (value->type() == kJSONNumber)){
                          sigma = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"NORMAL\" need to contain the "
                                       "\"standard deviation\"");
                          return NULL;
                        }

                        generators[j] = new NormalGenerator(mu, sigma, *rng);
                      } else if(((JSONString*)value)->value()=="UNIFORM"){
                        // Get the lower bound
                        int min;
                        if((value=generator->Get("min")) &&
                           (value->type() == kJSONNumber)){
                          min = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"UNIFORM\" need to contain the "
                                       "lower bound (\"min\")");
                          return NULL;
                        }
                        // Get the upper bound
                        int max;
                        if((value=generator->Get("max")) &&
                           (value->type() == kJSONNumber)){
                          max = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"UNIFORM\" need to contain the "
                                       "lower bound (\"max\")");
                          return NULL;
                        }
                        generators[j] = new UniformGenerator(min, max, *rng);
                      } else if(((JSONString*)value)->value()=="CONSTANT"){
                        // Get the constant value
                        int const_value;
                        if((value=generator->Get("value")) &&
                           (value->type() == kJSONNumber)){
                          const_value = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"CONSTANT\" need to contain the "
                                       "constant value (\"value\")");
                          return NULL;
                        }
                        generators[j] = new ConstantGenerator(const_value);
                      } else if(((JSONString*)value)->value()=="ZIPF"){
                        // Get min value
                        int min;
                        if((value=generator->Get("min")) &&
                           (value->type() == kJSONNumber)){
                          min = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"ZIPF\" need to contain the "
                                       "lower bound (\"min\")");
                          return NULL;
                        }

                        // Get max value
                        int max;
                        if((value=generator->Get("max")) &&
                           (value->type() == kJSONNumber)){
                          max = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"ZIPF\" need to contain the "
                                       "upper bound (\"max\")");
                          return NULL;
                        }

                        // Get theta
                        double theta;
                        if((value=generator->Get("theta")) &&
                           (value->type() == kJSONNumber)){
                          theta = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"ZIPF\" need to contain the "
                                       "zipfian constant (\"theta\")");
                          return NULL;
                        }

                        generators[j] = new ZipfGenerator(min, max, theta,*rng);
                      } else if(((JSONString*)value)->value()=="SEQUENCE"){
                        // Get min value
                        int min;
                        if((value=generator->Get("min")) &&
                           (value->type() == kJSONNumber)){
                          min = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"SEQUENCE\" need to contain the "
                                       "lower bound (\"min\")");
                          return NULL;
                        }

                        // Get step size
                        int step;
                        if((value=generator->Get("step")) &&
                           (value->type() == kJSONNumber)){
                          step = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"SEQUENCE\" need to contain the "
                                       "\"step\"");
                          return NULL;
                        }

                        generators[j] = new SequenceGenerator(min,step);
                      } else {
                        logger.Error("The generator type needs to be either "
                                     "\"NORMAL\", \"UNIFORM\", \"CONSTANT\", "
                                     "\"ZIPF\" or \"SEQUENCE\"");
                        return NULL;
                      }
                    } else {
                      if(((JSONString*)value)->value()=="CONSTANT"){
                        // Get constant value
                        std::string const_value;
                        if((value=generator->Get("value")) &&
                           (value->type() == kJSONString)){
                          const_value = ((JSONString*)value)->value();
                        } else {
                          logger.Error("The string generator type needs to be "
                                       "\"CONSTANT\"");
                          return NULL;
                        }
                        generators[j] = new ConstantStringGenerator(const_value);
                      } else if(((JSONString*)value)->value()=="FIXED_LENGTH"){
                        // Get length
                        int length = 0;
                        if((value=generator->Get("length")) &&
                           (value->type() == kJSONNumber)){
                          length = ((JSONNumber*)value)->value();
                        } else {
                          logger.Error("Generator specifications of type "
                                       "\"FIXED_LENGTH\" need to contain the "
                                       "string length (\"length\")");
                          return NULL;
                        }

                        generators[j] = new FixedLengthStringGenerator(length,*rng);
                      } else {
                        logger.Error("The string generator type needs to be "
                                     "\"CONSTANT\"");
                        return NULL;
                      }
                    }
                  } else {
                    logger.Error("Generators need to contain their \"type\"");
                    return NULL;
                  }
                } else {
                  logger.Error("Generators need to be specified as JSONObjects");
                  return NULL;
                }
              } else {
                logger.Error("Attributes need to be specified as JSONObjects");
                return NULL;
              }
            }

            // Create the index properties
            SIGMOD2012IndexProperties *index =
              new SIGMOD2012IndexProperties(name.c_str(),dimensions, types,
                                            generators,size, payload_size,
                                            keystore_capacity);
            result->AddIndex(index);
          } else {
            logger.Error("Index definitions need to contain an array of "
                         "attributes (\"attributes\")");
            return NULL;
          }

        } else {
          logger.Error("The provided index definition is not valid");
          return NULL;
        }
      }
    } else {
      logger.Error("The provided workloadfile needs to provide at least 1 "
                   "index definition");
      return NULL;
    }

  } catch (JSONException &exp){
    logger.Error("Error while parsing workload file: "+lexical_cast(exp.what()));
    return NULL;
  }

  // Close the workload file
  file.close();

  return result;
}
