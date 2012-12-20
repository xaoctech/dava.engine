/*==================================================================================
 Copyright (c) 2008, DAVA Consulting, LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Revision History:
 * Created by Vitaliy Borodovsky
 =====================================================================================*/

#include "Platform/Android/ThreadContext.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/Android/AndroidSpecifics.h"

namespace DAVA
{
    
ThreadContext::ThreadContext()
{
    context = EGL_NO_CONTEXT;
    display = EGL_NO_DISPLAY;
    surface = EGL_NO_SURFACE;

    config = NULL;
    surfaceFormat = 0;
    
    width = height = 0;
}

void ThreadContext::Bind()
{
    Logger::Debug("[ThreadContext::Bind]");
    
    if(surface != EGL_NO_SURFACE)
    {
        bool ret = eglMakeCurrent(display, surface, surface, context);
        Logger::Debug("[ThreadContext::Bind] set eglMakeCurrent returned = %d", ret);
    }
    else
    {
        Logger::Error("[ThreadContext::Bind] there is no surface");
    }
}

void ThreadContext::Unbind()
{
    Logger::Debug("[ThreadContext::Unbind]");
    if(surface != EGL_NO_SURFACE)
    {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }
    else
    {
        Logger::Error("[ThreadContext::Unbind] there is no surface");
    }
}

void ThreadContext::Swap()
{
    eglSwapBuffers(display, surface);
}
    
bool ThreadContext::CreateEGLContext(EGLContext sharedContext)
{
	if (!CreateConfig())
	{
        Logger::Error("[ThreadContext::CreateEGLContext] Can't create config");
		return false;
	}

    EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    context = eglCreateContext(display, config, sharedContext, contextAttrs);
    if (context == EGL_NO_CONTEXT)
	{
        Logger::Error("[ThreadContext::CreateEGLContext] Can't create context");
		return false;
	}
}


bool ThreadContext::CreateWindowSurface(ANativeWindow *window)
{
    if(surface != EGL_NO_SURFACE)
    {
        Logger::Error("[ThreadContext::CreateSurface] Surface has been created");
        return false;
    }
    
    if(!window)
    {
        Logger::Error("[ThreadContext::CreateSurface] window is NULL");
        
        return false;
    }
    
    ANativeWindow_setBuffersGeometry(window, 0, 0, surfaceFormat);
    surface = eglCreateWindowSurface(display, config, window, NULL);
    if (surface == EGL_NO_SURFACE)
    {
        Logger::Error("[ThreadContext::CreateSurface] Can't create surface");
        width = 0;
        height = 0;
        return false;
    }
    
    width = ANativeWindow_getWidth(window);
    height = ANativeWindow_getHeight(window);

    return true;
}
    
bool ThreadContext::CreateBufferSurface(float32 width, float32 height)
{
    GLint surfAttribs[] =
    {
        EGL_HEIGHT, width,
        EGL_WIDTH, height,
        EGL_NONE
    };
    
    surface = eglCreatePbufferSurface(display, config, surfAttribs);
    if (surface == EGL_NO_SURFACE)
    {
        Logger::Error("[ThreadContext::CreateBufferSurface] Can't create surface");
        return false;
    }
    
    this->width = width;
    this->height = height;
    
    return true;
}


void ThreadContext::DestroySurface()
{
    if(surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(display, surface);
        surface = EGL_NO_SURFACE;

        width = height = 0;
    }
}

void ThreadContext::DestroyContext()
{
    if(context != EGL_NO_CONTEXT)
    {
        eglDestroyContext(display, context);
        context = EGL_NO_CONTEXT;
    }
}
    
bool ThreadContext::CreateConfig()
{
    if (!SelectConfig(display, config))
	{
        Logger::Error("[ThreadContext::CreateConfig] Can't select config");
		return false;
	}
    
    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &surfaceFormat))
	{
        Logger::Error("[ThreadContext::CreateConfig] Can't select surface format");
		return false;
	}

    return true;
}

    
bool ThreadContext::SelectConfig(const EGLDisplay &display, EGLConfig& selectedConfig)
{
    EGLint count = 0;
    if (!eglGetConfigs(display, NULL, 0, &count))
    {
        Logger::Error("[ThreadContext::SelectConfig] cannot query count of all configs");
        return false;
    }
    
    EGLConfig* configs = new EGLConfig[count];
    if (!eglGetConfigs(display, configs, count, &count))
    {
        Logger::Error("[ThreadContext::SelectConfig] cannot query all configs");
        SafeDeleteArray(configs);
        return false;
    }
    
    int32 bestMatch = 1<<30;
    int32 bestIndex = -1;
    for (int32 i = 0; i < count; ++i)
    {
        int32 match = 0;
        EGLint surfaceType = 0;
        EGLint blueBits = 0;
        EGLint greenBits = 0;
        EGLint redBits = 0;
        EGLint alphaBits = 0;
        EGLint depthBits = 0;
        EGLint stencilBits = 0;
        EGLint renderableFlags = 0;
        
        EGLint configId = 0;
        
        eglGetConfigAttrib(display, configs[i], EGL_SURFACE_TYPE, &surfaceType);
        eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &blueBits);
        eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &greenBits);
        eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &redBits);
        eglGetConfigAttrib(display, configs[i], EGL_ALPHA_SIZE, &alphaBits);
        eglGetConfigAttrib(display, configs[i], EGL_DEPTH_SIZE, &depthBits);
        eglGetConfigAttrib(display, configs[i], EGL_STENCIL_SIZE, &stencilBits);
        eglGetConfigAttrib(display, configs[i], EGL_RENDERABLE_TYPE, &renderableFlags);
        
        eglGetConfigAttrib(display, configs[i], EGL_CONFIG_ID, &configId);
        
        
        //        Logger::Debug("[EGLRenderer::SelectConfig] [%d]: R%dG%dB%dA%d D%dS%d Type=%04x Render=%04x id = %d",
        //                     i, redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, surfaceType, renderableFlags, configId);
        
        if ((surfaceType & EGL_WINDOW_BIT) == 0)
            continue;
        if ((renderableFlags & EGL_OPENGL_ES2_BIT) == 0)
            continue;
        if ((depthBits < 16) || (stencilBits < 8))
            continue;
        if ((redBits < 8) || (greenBits < 8) || (blueBits < 8) || (alphaBits < 8))
            continue;
        
        int32 penalty = depthBits - 16;
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
//            Logger::Debug("[ThreadContext::SelectConfig] [%d] is the new best config", i, configs[i]);
        }
    }
    
    if (bestIndex < 0)
    {
        SafeDeleteArray(configs);
        return false;
    }
    
    selectedConfig = configs[bestIndex];
    SafeDeleteArray(configs);
    return true;
}

    
}

#endif // #if defined(__DAVAENGINE_ANDROID__)

