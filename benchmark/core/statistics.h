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

#ifndef BENCHMARK_CORE_STATISTICS_H_
#define BENCHMARK_CORE_STATISTICS_H_

#include <vector>

// Defines a single statistics element which is consists of a metric and a
// corresponding value
class StatElement{
public:
  // Constructs an empty statistcs element
  StatElement():metric_(""),value_(""){}
  
  // Constructs an statistics element with the given metric and value
  StatElement(const std::string &metric, const std::string &value):
    metric_(metric),value_(value){}
  
  // Returns the value of this statistics element
  std::string value() const {return value_;}
  
  // Returns the metric of this statistics element
  std::string metric() const {return metric_;}
  
  // Sets the value of this statistics element
  void value(const std::string& value){ value_=value;}
  
  // Sets the metric of this statistics element
  void metric(const std::string& metric){ metric_=metric;}
  
private:
  // The metric of this statistics element
  std::string metric_;
  
  // The value of this statistics element
  std::string value_;
};

// Defines a group of statistics elements
class StatGroup{
public: 
  // Constructs an empty statistics element group
  StatGroup():name_(""){}
  
  // Constructs an statistics element group with the given name
  explicit StatGroup(std::string name):name_(name){}
  
  // Adds a new statistics element with the given metric and value to the group
  void Add(const std::string &metric, const std::string &value){
    this->Add(StatElement(metric,value));
  }
  
  // Adds the given statistics element to the group
  void Add(const StatElement& element){elements_.push_back(element);}
  
  // Inserts a new statistics element with the given metric and value at the
  // front of the group
  void AddFront(const std::string &metric, const std::string &value){
    this->Add(StatElement(metric,value));
  }
  
  // Inserts the given statistics element at the front of the group
  void AddFront(const StatElement &element){
    elements_.insert(elements_.begin(),element);
  }
  
  // Returns the name of the group
  std::string name() const {return name_;}
  
  // Sets the name of the group
  void name(const std::string& name){name_=name;}
  
  // Returns a list of all statistics elements inside this group
  const std::vector<StatElement>& elements() const {return elements_;}
  
private:
  // The name of the group
  std::string name_;
  
  // A list of all statistics elements inside this group
  std::vector<StatElement> elements_;
};

// Defines a set of statistics.
//
// Statistics consist of groups of statistics elements, as well as of single
// statistics elements (metric/value pairs)
class Statistics {
public:
  // Adds the given statistics element
  void Add(const StatElement &element){elements_.push_back(element);}
  
  // Inserts a new statistics element with the given metric and value
  void Add(const std::string &metric, const std::string &value){
    this->Add(StatElement(metric,value));
  }
  
  // Inserts the given statistics element at the front
  void AddFront(const StatElement &element){
    elements_.insert(elements_.begin(),element);
  }
  
  // Inserts a new statistics element with the given metric and value at the front
  void AddFront(const std::string &metric, const std::string &value){
    this->Add(StatElement(metric,value));
  }
  
  // Inserts the given statistics group
  void AddGroup(const StatGroup& group){groups_.push_back(group);}
  
  // Returns a list of all top level statistics elements
  // (i.e. statistics elements that are not members of any group)
  const std::vector<StatElement>& elements() const {return elements_;}
  
  // Returns a list of all statistics groups
  const std::vector<StatGroup>& groups() const {return groups_;}
private:
  // A list of all statistics elements
  std::vector<StatElement> elements_;
  
  // A list of all statistics groups
  std::vector<StatGroup> groups_;
};

#endif // BENCHMARK_CORE_STATISTICS_H_
