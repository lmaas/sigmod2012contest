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

/** @file
 An abstraction layer for mutexes (to make them easier to use and to make the
 implementation more independent of the actual threading library)
*/

#ifndef _BDBIMPL_MUTEX_H_
#define _BDBIMPL_MUTEX_H_

#include <pthread.h>

#define lock(x) if(Lock _lock_=x){}else

class Mutex{
	public:
		Mutex(){
			pthread_mutex_init(&mutex_, 0);
		};
		
		~Mutex(){
			pthread_mutex_destroy(&mutex_);
		};
		
		friend class Lock;
		
	private:
		pthread_mutex_t mutex_;
		
		void Lock(){
			pthread_mutex_lock(&mutex_);
		};
		
		void Unlock(){
			pthread_mutex_unlock(&mutex_);
		};
};

class Lock{
	public:
		Lock(Mutex& mutex):mutex_(mutex){mutex_.Lock();};
		~Lock(){mutex_.Unlock();};
		
		operator bool() const {
			return false;
		}
		
	private:
		Mutex& mutex_;
};

#endif // _BDBIMPL_MUTEX_H_
