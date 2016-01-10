//-----------------------------------------------------------------------------------
//
//   Torque Network Library
//   Copyright (C) 2004 GarageGames.com, Inc.
//   For more information see http://www.opentnl.org
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   For use in products that are not compatible with the terms of the GNU 
//   General Public License, alternative licensing options are available 
//   from GarageGames.com.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//------------------------------------------------------------------------------------

#include "tnlThread.h"
#include "tnlLog.h"

namespace TNL
{

#ifdef TNL_OS_WIN32
Semaphore::Semaphore(U32 initialCount, U32 maximumCount)
{
   mSemaphore = CreateSemaphore(NULL, initialCount, maximumCount, NULL);
}

Semaphore::~Semaphore()
{
   CloseHandle(mSemaphore);
}

void Semaphore::wait()
{
   WaitForSingleObject(mSemaphore, INFINITE);
}

void Semaphore::increment(U32 count)
{
   ReleaseSemaphore(mSemaphore, count, NULL);
}

Mutex::Mutex()
{
   InitializeCriticalSection(&mLock);
}

Mutex::~Mutex()
{
   DeleteCriticalSection(&mLock);
}

void Mutex::lock()
{
   EnterCriticalSection(&mLock);
}

void Mutex::unlock()
{
   LeaveCriticalSection(&mLock);
}

bool Mutex::tryLock()
{
   return false;//   return TryEnterCriticalSection(&mLock);
}

ThreadStorage::ThreadStorage()
{
   mTlsIndex = TlsAlloc();
}

ThreadStorage::~ThreadStorage()
{
   TlsFree(mTlsIndex);
}

void *ThreadStorage::get()
{
   return TlsGetValue(mTlsIndex);
}

void ThreadStorage::set(void *value)
{
   TlsSetValue(mTlsIndex, value);
}

DWORD WINAPI ThreadProc( LPVOID lpParameter )
{
   return ((Thread *) lpParameter)->run();
}

U32 Thread::run()
{
   return 0;
}

void Thread::start()
{
   mThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
   mReturnValue = 0;
}

Thread::Thread()
{
}

Thread::~Thread()
{
   CloseHandle(mThread);
}

#else

Semaphore::Semaphore(U32 initialCount, U32 maximumCount)
{
#ifdef TNL_OS_MAC_OSX
	mSemaphore = dispatch_semaphore_create(initialCount);
#else
	sem_init(&mSemaphore, 0, initialCount);
#endif
}

Semaphore::~Semaphore()
{
#ifdef TNL_OS_MAC_OSX
	dispatch_release(mSemaphore);
#else
   sem_destroy(&mSemaphore);
#endif
}

void Semaphore::wait()
{
#ifdef TNL_OS_MAC_OSX
	dispatch_semaphore_wait(mSemaphore, DISPATCH_TIME_FOREVER);
#else
   sem_wait(&mSemaphore);
#endif
}

void Semaphore::increment(U32 count)
{
   for(U32 i = 0; i < count; i++)
#ifdef TNL_OS_MAC_OSX
	   dispatch_semaphore_signal(mSemaphore);
#else
      sem_post(&mSemaphore);
#endif
}

Mutex::Mutex()
{
   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
#ifdef TNL_OS_LINUX
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif
   pthread_mutex_init(&mMutex, &attr);
   pthread_mutexattr_destroy(&attr);
}

Mutex::~Mutex()
{
   pthread_mutex_destroy(&mMutex);
}

void Mutex::lock()
{
   pthread_mutex_lock(&mMutex);
}

void Mutex::unlock()
{
   pthread_mutex_unlock(&mMutex);
}

bool Mutex::tryLock()
{
   return false;//   return TryEnterCriticalSection(&mLock);
}

ThreadStorage::ThreadStorage()
{
   pthread_key_create(&mThreadKey, NULL);
}

ThreadStorage::~ThreadStorage()
{
   pthread_key_delete(mThreadKey);
}

void *ThreadStorage::get()
{
   return pthread_getspecific(mThreadKey);
}

void ThreadStorage::set(void *value)
{
   pthread_setspecific(mThreadKey, value);
}

void *ThreadProc(void *lpParameter)
{
   return (void *) ((Thread *) lpParameter)->run();
}

Thread::Thread()
{
}

void Thread::start()
{
   size_t val = pthread_create(&mThread, NULL, ThreadProc, this);
   mReturnValue = 0;
}

Thread::~Thread()
{
}

#endif

ThreadQueue::ThreadQueueThread::ThreadQueueThread(ThreadQueue *q)
{
   mThreadQueue = q;
}

size_t ThreadQueue::ThreadQueueThread::run()
{
   mThreadQueue->threadStart();

   mThreadQueue->lock();
   ThreadStorage &sto = mThreadQueue->getStorage();
   sto.set((void *) 0);
   mThreadQueue->unlock();

   for(;;)
      mThreadQueue->dispatchNextCall();
   return 0;
}

ThreadQueue::ThreadQueue(U32 threadCount)
{
   mStorage.set((void *) 1);
   for(U32 i = 0; i < threadCount; i++)
   {
      Thread *theThread = new ThreadQueueThread(this);
      mThreads.push_back(theThread);
      theThread->start();
   }
}

ThreadQueue::~ThreadQueue()
{
}

void ThreadQueue::dispatchNextCall()
{
   mSemaphore.wait();
   lock();
   if(mThreadCalls.size() == 0)
   {
      unlock();
      return;
   }
   Functor *c = mThreadCalls.first();
   mThreadCalls.pop_front();
   unlock();
   c->dispatch(this);
   delete c;
}

void ThreadQueue::postCall(Functor *theCall)
{
   lock();
   if(isMainThread())
   {
      mThreadCalls.push_back(theCall);
      unlock();
      mSemaphore.increment();
   }
   else
   {
      mResponseCalls.push_back(theCall);
      unlock();
   }
}

void ThreadQueue::dispatchResponseCalls()
{
   lock();
   for(S32 i = 0; i < mResponseCalls.size(); i++)
   {
      Functor *c = mResponseCalls[i];
      c->dispatch(this);
      delete c;
   }
   mResponseCalls.clear();
   unlock();
}

};
