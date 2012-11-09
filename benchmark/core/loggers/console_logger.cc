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

#include "console_logger.h"

// Adds a new section with the given name and description
void ConsoleLogger::AddSection(const std::string &name,
                               const std::string &description,
                               bool verbose){
  if(quiet())
    return;
  
  if(verbose){
    if(!this->verbose())
      return;
    verbose_count_++;
  }
  
  if(section_compact_)
    std::cout << std::endl;
  
  indent();
  if(indent_>0)std::cout<<"- ";
  std::cout << description << "..." << std::flush;
  section_compact_ = true;
  indent_++;
};

// Closes the last opened section given a return state (success or failure)
void ConsoleLogger::CloseSection(bool success, std::string details){
  if(quiet() || (indent_==0))
    return;
  if(verbose_count_ > 0){
    verbose_count_--;
    if(verbose())
      return;
  }
  bool new_line = false;
  if(section_compact_){
    std::cout << (success?"done":"failed");
    new_line = true;
  } else if(!success){
    std::cout << std::endl <<"failed";
    new_line = true;
  }
  if(!details.empty()){
    if((!section_compact_) && success)
      std::cout << "done";
    std::cout << (" ("+details+")");
    new_line=true;
  }
  if(new_line)
    std::cout<<std::endl;
  else
    std::cout << std::flush;
  section_compact_ = false;
  indent_--;
};

// Prints the given statistics
void ConsoleLogger::PrintStatistics(const class Statistics& stats){
  this->Info("-------------------------------");
  // Print the single stats
  for(unsigned int i = 0; i < stats.elements().size(); i++){
    this->Info((stats.elements()[i].metric())+": "+(stats.elements()[i].value()));
  }
  // Print the groups
  for(unsigned int i = 0; i < stats.groups().size(); i++){
    this->Info("");
    this->Info(" "+(stats.groups()[i].name())+":");
    this->Info(" --------------------");
    const std::vector<StatElement>& e = stats.groups()[i].elements();
    for(unsigned int i = 0; i < e.size(); i++){
      this->Info("  "+(e[i].metric())+": "+(e[i].value()));
    }
  }
  
  this->Info("");
};

// Logs a message using the given LogLevel.
// When newline is set, message will be printed in a new line
void ConsoleLogger::Log(LogLevel level, const std::string &message, bool newline){
  std::string out = (section_compact_?"\n":"");
  
  // If necessary indent the output
  out += std::string(indentation_width_*indent_,' ');
  
  switch (level) {
    case kLogLevelFatal:
      out += "FATAL ERROR: ";
      break;
    case kLogLevelError:
      out += "ERROR: ";
      break;
    case kLogLevelWarning:
      if(quiet())
        return;
      out += "WARNING: ";
      break;
    case kLogLevelDebug:
      if(quiet())
        return;
      out += "DEBUG: ";
      break;
    case kLogLevelVerbose:
      if(!verbose())
        return;
    default:
      if(quiet())
        return;
      
      if(indent_>0)
        out += "- ";
      break;
  }
  
  out+=message;
  out+=(newline?"\n":"");
  std::cout << out;
  
  section_compact_=false;
};

// Indents the current line
void ConsoleLogger::indent(){
  std::cout << std::string(indentation_width_*indent_,' ');
}
