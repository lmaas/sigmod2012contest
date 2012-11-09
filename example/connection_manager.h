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

#ifndef _BDBIMPL_CONNECTION_MANAGER_H_
#define _BDBIMPL_CONNECTION_MANAGER_H_

#include <pthread.h>
#include <common/macros.h>

class DbEnv;

// Defines a simple connection manager for Berkeley DB.
//
// ConnectionManager is used to build and manage an open Berkeley DB environment.
// 
// ConnectionManager implements the Singleton Pattern. 
class ConnectionManager{
 	public:
    // Return the singleton instance of ConnectionManager
		static ConnectionManager& getInstance();
		
    // Return the Berkeley DB environment that will be used
		DbEnv *env(){ return env_; }
    
    // Initialize the singleton instance
    static void Initialize();
  
    // Destroy the singleton instance
    static void Destroy();
  
	private:
		// Private constructor (don't allow instanciation from outside)
		ConnectionManager();

		// Destructor
		~ConnectionManager();
		
		// The Berkeley DB environment that will be used
		DbEnv *env_;
    
    // The singleton instance of ConnectionManager
    static ConnectionManager* instance_;
  
    // A pthread once handle to guarantee that the singleton instance is 
    // only initialized once
    static pthread_once_t once_;

    DISALLOW_COPY_AND_ASSIGN(ConnectionManager);
};

#endif // _BDBIMPL_CONNECTION_MANAGER_H_
