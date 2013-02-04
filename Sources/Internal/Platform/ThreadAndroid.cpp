#include "Platform/Thread.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"
//#include "Platform/Android/EGLRenderer.h"

namespace DAVA
{


#include <pthread.h>

pid_t Thread::mainThreadId = 0;


void * PthreadMain (void * param)
{
	Logger::Info("[Thread::ThreadFunction] param = %p", param);

	Thread * t = (Thread*)param;

    /*ThreadContext *context = NULL;
    CorePlatformAndroid *core = (CorePlatformAndroid *)Core::Instance();

	if(t->needCopyContext)
    {
        context = core->CreateThreadContext();
        core->BindThreadContext(context);
    }

	t->state = Thread::STATE_RUNNING;
	t->msg(t);

    if(t->needCopyContext)
	{
        core->UnbindThreadContext(context);
        core->ReleaseThreadContext(context);
        context = NULL;
	}

	t->state = Thread::STATE_ENDED;
	t->Release();*/

	pthread_exit(0);
}



void Thread::StartAndroid()
{
    if(needCopyContext)
	{
//
	}
    
    pthread_t threadId;
	pthread_create(&threadId, 0, PthreadMain, (void*)this);

    Logger::Info("[Thread::StartAndroid]");
}

bool Thread::IsMainThread()
{
    return (mainThreadId == gettid());
}

void Thread::InitMainThread()
{
    mainThreadId = gettid();
    Logger::Info("[Thread::InitMainThread] %ld", mainThreadId);
}

pthread_mutex_t fakeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fakeCond = PTHREAD_COND_INITIALIZER;
void Thread::SleepThread(int32 timems)
{
//	Logger::Info("[Thread::SleepThread] timems = %d", timems);

	struct timespec timeToWait;
	struct timeval now;
	int rt;

	gettimeofday(&now,NULL);

	timeToWait.tv_sec = now.tv_sec;
	timeToWait.tv_nsec = now.tv_usec*1000 + timems*1000000;

//	Logger::Info("[Thread::SleepThread] sec = %ld, nsec = %ld", timeToWait.tv_sec, timeToWait.tv_nsec);


	pthread_mutex_lock(&fakeMutex);
	rt = pthread_cond_timedwait(&fakeCond, &fakeMutex, &timeToWait);
	pthread_mutex_unlock(&fakeMutex);

//	Logger::Info("[Thread::SleepThread] done");
}

void Thread::YieldThread()
{
    sched_yield();
}


};

#endif //#if defined(__DAVAENGINE_ANDROID__)

