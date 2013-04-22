#include "Platform/Thread.h"

#if defined(__DAVAENGINE_ANDROID__)

#include <EGL/eglplatform.h>
#include <EGL/egl.h>

#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

namespace DAVA
{

#include <unistd.h>

#include <pthread.h>

pid_t Thread::mainThreadId = 0;

EGLContext Thread::currentContext;
EGLDisplay Thread::currentDisplay;
EGLSurface Thread::currentDrawSurface;
EGLSurface Thread::currentReadSurface;



bool GetConfig(EGLDisplay disp, EGLConfig& bestConfig)
{
	EGLint count = 0;
	if (!eglGetConfigs(disp, NULL, 0, &count))
	{
		Logger::Error("[GetConfig] cannot query count of all configs");
		return false;
	}

	Logger::Info("[GetConfig] Count = %d", count);

	EGLConfig* configs = new EGLConfig[count];
	if (!eglGetConfigs(disp, configs, count, &count))
	{
		Logger::Error("[GetConfig] cannot query all configs");
		return false;
	}

	int bestMatch = 1<<30;
	int bestIndex = -1;

	int i;
	for (i = 0; i < count; i++)
	{
		int match = 0;
		EGLint surfaceType = 0;
		EGLint blueBits = 0;
		EGLint greenBits = 0;
		EGLint redBits = 0;
		EGLint alphaBits = 0;
		EGLint depthBits = 0;
		EGLint stencilBits = 0;
		EGLint renderableFlags = 0;

		EGLint configId = 0;

		eglGetConfigAttrib(disp, configs[i], EGL_SURFACE_TYPE, &surfaceType);
		eglGetConfigAttrib(disp, configs[i], EGL_BLUE_SIZE, &blueBits);
		eglGetConfigAttrib(disp, configs[i], EGL_GREEN_SIZE, &greenBits);
		eglGetConfigAttrib(disp, configs[i], EGL_RED_SIZE, &redBits);
		eglGetConfigAttrib(disp, configs[i], EGL_ALPHA_SIZE, &alphaBits);
		eglGetConfigAttrib(disp, configs[i], EGL_DEPTH_SIZE, &depthBits);
		eglGetConfigAttrib(disp, configs[i], EGL_STENCIL_SIZE, &stencilBits);
		eglGetConfigAttrib(disp, configs[i], EGL_RENDERABLE_TYPE, &renderableFlags);

		eglGetConfigAttrib(disp, configs[i], EGL_CONFIG_ID, &configId);


		Logger::Info("Config[%d]: R%dG%dB%dA%d D%dS%d Type=%04x Render=%04x id = %d",
			i, redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, surfaceType, renderableFlags, configId);

		if ((surfaceType & EGL_WINDOW_BIT) == 0)
			continue;
		if ((renderableFlags & EGL_OPENGL_ES2_BIT) == 0)
			continue;
		if ((depthBits < 16) || (stencilBits < 8))
			continue;
		if ((redBits < 8) || (greenBits < 8) || (blueBits < 8) || (alphaBits < 8))
			continue;

		int penalty = depthBits - 16;
		match += penalty * penalty;
		penalty = redBits - 8;
		match += penalty * penalty;
		penalty = greenBits - 8;
		match += penalty * penalty;
		penalty = blueBits - 8;
		match += penalty * penalty;
		penalty = alphaBits - 8;
		match += penalty * penalty;
		penalty = stencilBits;
		match += penalty * penalty;

		if ((match < bestMatch) || (bestIndex == -1))
		{
			bestMatch = match;
			bestIndex = i;
			Logger::Info("Config[%d] is the new best config", i, configs[i]);
		}
	}

	if (bestIndex < 0)
	{
		delete[] configs;
		return false;
	}

	bestConfig = configs[bestIndex];
	delete[] configs;

	return true;
}



void * PthreadMain (void * param)
{
	Logger::Info("[PthreadMain] param = %p", param);

	Thread * t = (Thread*)param;

	if(t->needCopyContext)
    {
    	EGLConfig localConfig;
    	bool ret = GetConfig(Thread::currentDisplay, localConfig);
		Logger::Info("[PthreadMain] GetConfig returned = %d", ret);

    	if(ret)
    	{
        	EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        	t->localContext = eglCreateContext(t->currentDisplay, localConfig, t->currentContext, contextAttrs);
//        	t->localContext = eglCreateContext(t->currentDisplay, localConfig, EGL_NO_CONTEXT, contextAttrs);
    	}

    	if(t->localContext == EGL_NO_CONTEXT)
    	{
    		Logger::Error("[PthreadMain] Can't create local context");
    	}

    	GLint surfAttribs[] =
    	{
    			EGL_HEIGHT, 768,
    			EGL_WIDTH, 1024,
    			EGL_NONE
    	};

        
    	EGLSurface readSurface = eglCreatePbufferSurface(t->currentDisplay, localConfig, surfAttribs);
//    	EGLSurface drawSurface = eglCreatePbufferSurface(t->currentDisplay, localConfig, surfAttribs);

        //TODO: set context
//		bool ret2 = eglMakeCurrent(t->currentDisplay, t->currentDrawSurface, t->currentReadSurface, t->localContext);
		bool ret2 = eglMakeCurrent(t->currentDisplay, readSurface, readSurface, t->localContext);
		Logger::Info("[PthreadMain] set eglMakeCurrent returned = %d", ret2);
    }

	t->state = Thread::STATE_RUNNING;
	t->msg(t);

    if(t->needCopyContext)
	{
        //TODO: Restore context
		bool ret = eglMakeCurrent(t->currentDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		Logger::Info("[PthreadMain] restore eglMakeCurrent returned = %d", ret);
	}

	t->state = Thread::STATE_ENDED;
	t->Release();

	pthread_exit(0);
}



void Thread::StartAndroid()
{
    if(needCopyContext)
	{
    	localContext = EGL_NO_CONTEXT;
//		bool ret = eglMakeCurrent(currentDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
//		Logger::Info("[PthreadMain] restore eglMakeCurrent returned = %d", ret);
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

    currentContext = eglGetCurrentContext();
    if(currentContext == EGL_NO_CONTEXT)
    {
		Logger::Error("[Thread::InitMainThread] EGL_NO_CONTEXT");
    }

    currentDisplay = eglGetCurrentDisplay();
	if (currentDisplay == EGL_NO_DISPLAY)
	{
		Logger::Error("[Thread::InitMainThread] EGL_NO_DISPLAY");
	}

    currentDrawSurface = eglGetCurrentSurface(EGL_DRAW);
    if(currentDrawSurface == EGL_NO_SURFACE)
    {
		Logger::Error("[Thread::InitMainThread] EGL_NO_SURFACE | EGL_DRAW");
    }

    currentReadSurface = eglGetCurrentSurface(EGL_READ);
    if(currentReadSurface == EGL_NO_SURFACE)
    {
		Logger::Error("[Thread::InitMainThread] EGL_NO_SURFACE | EGL_READ");
    }


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

