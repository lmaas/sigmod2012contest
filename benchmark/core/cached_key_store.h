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

#ifndef BENCHMARK_CORE_CACHED_KEY_STORE_H_
#define BENCHMARK_CORE_CACHED_KEY_STORE_H_

#include <iostream>

#include "utils/mutex.h"

// A key store to cache a certain amount of keys
template<class Key>
class CachedKeyStore {
 public:
  // Constructs a new key store with the given capacity
  CachedKeyStore(unsigned int capacity):
    capacity_(capacity),size_(0){data_ = new Key[capacity_];}

  // Inserts a key into the key store.
  // Returns false if the current number of keys inside the key store
  // exceeds its capacity
  bool Push(Key &key){
    lock(mutex_){
      if(size_>=capacity_)
        return false;
      data_[size_] = key;
      size_++;
    }
    return true;
  }

  // Removes and returns the last keystore element.
  // Returns false if the key store is empty
  bool Pop(Key &key){
    lock(mutex_){
      if(size_<1)
        return false;
      size_--;
      key = data_[size_];
    }
    return true;
  }

  // Retrieves the key at the given index.
  // Returns false if the key store is empty
  bool Get(const int index, Key &key){
    lock(mutex_){
      if(size_<1)
        return false;
      key = data_[index%size_];
    }
    return true;
  }

  // Returns the capacity of this key store
  unsigned int capacity() const {return capacity_;}

  // Returns the current number of keys inside the keystore
  unsigned int size() const {return size_;}

 private:
  // The capacity of this key store
  const unsigned int capacity_;

  // The current number of keys inside the keystore
  unsigned int size_;

  // A mutex to lock the keystore
  Mutex mutex_;

  // The key list itself
  Key* data_;
};

#endif // BENCHMARK_CORE_CACHED_KEY_STORE_H_
