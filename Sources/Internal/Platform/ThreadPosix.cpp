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

#if defined(DAVAENGINE_PTHREAD)

#include <time.h>
#include <errno.h>

#if defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
#import <QuartzCore/QuartzCore.h>
#import <Foundation/NSAutoreleasePool.h>
#elif defined (__DAVAENGINE_ANDROID__)
#include <pthread.h>
#endif

namespace DAVA
{
// becouse android have no way to kill a thread, we will use signal to determine
// if we need to end thread
#if defined (__DAVAENGINE_ANDROID__)
void Thread::thread_exit_handler(int sig)
{
	if (SIGKILL == sig)
		pthread_exit(0);
}
#endif

void Thread::Init()
{
#if defined (__DAVAENGINE_ANDROID__)
	struct sigaction cancelThreadAction;
	memset(&cancelThreadAction, 0, sizeof(cancelThreadAction));
	sigemptyset(&cancelThreadAction.sa_mask);
	cancelThreadAction.sa_flags = 0;
	cancelThreadAction.sa_handler = thread_exit_handler;
	sigaction(SIGKILL, &cancelThreadAction, NULL);
#endif
}

void Thread::Shutdown()
{
    
}

void Thread::KillNative(Handle _handle)
{
	uint32 ret = 0;
#if defined (__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
    ret = pthread_cancel(_handle);
#endif
#if defined (__DAVAENGINE_ANDROID__)
    ret = pthread_kill(_handle, SIGKILL);
#endif
    if (0 != ret)
    	Logger::FrameworkDebug("[Thread::Cancel] for android: id = %d, error = %d", Thread::GetCurrentThreadId(), ret);
}

void Thread::SleepThread(uint32 timeMS)
{
    timespec req, rem;
    req.tv_sec = timeMS / 1000;
    req.tv_nsec = (timeMS % 1000) * 1000000L;
    int32 ret = EINTR;
    while(ret == EINTR)
    {
        ret = nanosleep(&req, &rem);
        req = rem;
    }
}

void *PthreadMain(void *param)
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
#endif
    
    Thread::ThreadFunction(param);

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    [pool release];
#endif
    pthread_exit(0);
}

void Thread::Start()
{
    pthread_create(&handle, 0, PthreadMain, (void *)this);
}

void Thread::YieldThread()
{
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_MACOS__)
    pthread_yield_np();
#elif defined(__DAVAEBGINE_ANDROID__)
    sched_yield();
#endif
}

void Thread::Join()
{
    pthread_join(handle, NULL);
}

Thread::Handle Thread::GetCurrentHandle()
{
    return pthread_self();
}

}

#endif
