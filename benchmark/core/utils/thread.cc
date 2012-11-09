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

#include <errno.h>
#include <stdexcept>
#include <stdio.h>

#include "thread.h"

// A helper function used to run the thread's main functions
static void *_thread_func(void *arg){
	Thread *thread = static_cast<Thread *>(arg);
  thread->Run();

  // Exit the thread
  pthread_exit(NULL);

  return NULL;
}

// Constructs a new thread
Thread::Thread(){thread_ = NULL;};

// Starts the thread
void Thread::Start(){
  if (thread_ != NULL)
    return;

  thread_ = new pthread_t;

  int r = pthread_create(thread_, NULL, *_thread_func, this);
  if (r != 0)
    throw std::runtime_error("pthread_create failed while starting thread");
};

// Blocks the calling thread until the thread terminates
void Thread::Join(){
  if (thread_) {
    pthread_join(*thread_, NULL);

    delete thread_;
    thread_ = NULL;
  }
};
