/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "Platform/Thread.h"

#ifndef __DAVAENGINE_WIN32__
#include <time.h>
#include <errno.h>
#endif

namespace DAVA
{

Set<Thread *> Thread::threadList;
Mutex Thread::threadListMutex;
Map<Thread::Handle, Thread::Id> Thread::threadIdList;
Mutex Thread::threadIdListMutex;

Thread::Id Thread::mainThreadId = 0;

ConditionalVariable::ConditionalVariable()
{
    int32 ret = pthread_cond_init(&cv, 0);
    if(ret)
        Logger::FrameworkDebug("[ConditionalVariable::ConditionalVariable()]: pthread_cond_init error code %d", ret);
    ret = pthread_mutex_init(&exMutex, 0);
    if(ret)
        Logger::FrameworkDebug("[ConditionalVariable::ConditionalVariable()]: pthread_mutex_init error code %d", ret);
}

ConditionalVariable::~ConditionalVariable()
{
    int32 ret = pthread_cond_destroy(&cv);
    if(ret)
        Logger::FrameworkDebug("[ConditionalVariable::~ConditionalVariable()]: pthread_cond_destroy error code %d", ret);
    ret = pthread_mutex_destroy(&exMutex);
    if(ret)
        Logger::FrameworkDebug("[ConditionalVariable::~ConditionalVariable()]: pthread_mutex_destroy error code %d", ret);
}

void Thread::InitMainThread()
{
    mainThreadId = GetCurrentThreadId();
}

bool Thread::IsMainThread()
{
    if (mainThreadId == 0)
    {
        Logger::Error("Main thread not initialized");
    }
    return (mainThreadId == GetCurrentThreadId());
}

Thread::Id Thread::GetCurrentThreadId()
{
    // try to find in map
    // if not found - create new id
    Id retId;
    
    threadIdListMutex.Lock();
    Map<Handle, Id>::iterator it = threadIdList.find(GetCurrentHandle());
    if (it == threadIdList.end())
    {
        Handle newHandle = GetCurrentHandle();
        static Id newId = 0;
        threadIdList[newHandle] = newId;
        retId = newId++;
    }
    else
        retId = it->second;
    threadIdListMutex.Unlock();
    
    return retId;
}

Thread * Thread::Create(const Message& msg)
{
	Thread * t = new Thread(msg);
	t->state = STATE_CREATED;
	
	return t;
}

void Thread::Kill()
{
    if(state != STATE_ENDED && state != STATE_KILLED)
    {
        threadIdListMutex.Lock();
        threadIdList.erase(threadIdList.find(handle));
        threadIdListMutex.Unlock();
        KillNative(handle);
        state = STATE_KILLED;
    }
}

void Thread::KillAll()
{
    threadListMutex.Lock();
    Set<Thread *>::iterator i = threadList.begin();
    Set<Thread *>::iterator end = threadList.end();
    for(; i != end; ++i)
    {
        (*i)->Kill();
    }
    threadListMutex.Unlock();
}

Thread::Thread(const Message& _msg)
    : msg(_msg)
{
    threadListMutex.Lock();
    threadList.insert(this);
    threadListMutex.Unlock();

    Init();
}

Thread::~Thread()
{
    Shutdown();
    threadIdListMutex.Lock();
    threadIdList.erase(handle);
    threadIdListMutex.Unlock();
    threadListMutex.Lock();
    threadList.erase(this);
    threadListMutex.Unlock();
}

Thread::eThreadState Thread::GetState()
{
	return state;
}

void Thread::Wait(ConditionalVariable * cv)
{
    int32 ret = 0;
    if((ret = pthread_mutex_lock(&cv->exMutex)))
        Logger::FrameworkDebug("[Thread::Wait]: pthread_mutex_lock error code %d", ret);
    
    if((ret = pthread_cond_wait(&cv->cv, &cv->exMutex)))
        Logger::FrameworkDebug("[Thread::Wait]: pthread_cond_wait error code %d", ret);

    if((ret = pthread_mutex_unlock(&cv->exMutex)))
        Logger::FrameworkDebug("[Thread::Wait]: pthread_mutex_unlock error code %d", ret);
}

void Thread::Signal(ConditionalVariable * cv)
{
    int32 ret = pthread_cond_signal(&cv->cv);
    if(ret)
        Logger::FrameworkDebug("[Thread::Signal]: pthread_cond_signal error code %d", ret);
}

void Thread::Broadcast(ConditionalVariable * cv)
{
    int32 ret = pthread_cond_broadcast(&cv->cv);
    if(ret)
        Logger::FrameworkDebug("[Thread::Broadcast]: pthread_cond_broadcast error code %d", ret);
}

void Thread::SetId(const Id &threadId)
{
    id = threadId;
}

Thread::Id Thread::GetId()
{
    return id;
}
    
};
