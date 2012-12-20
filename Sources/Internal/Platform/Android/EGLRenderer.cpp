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

#include "Platform/Android/EGLRenderer.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/Android/AndroidSpecifics.h"
#include "Platform/Android/ThreadContext.h"

namespace DAVA
{
    
EGLRenderer::EGLRenderer()
{
    mainThreadContext = new ThreadContext();
    
//    defaultFramebuffer = 0;
//    colorRenderbuffer = 0;
//    
//    depthRenderbuffer = 0;
//    
//    backingWidth = 0;
//	backingHeight = 0;
}
    
    
EGLRenderer::~EGLRenderer()
{
    SafeRelease(mainThreadContext);
}
    
bool EGLRenderer::InitializeGL()
{
    Logger::Debug("[EGLRenderer::InitializeGL]");

    mainThreadContext->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (mainThreadContext->display == EGL_NO_DISPLAY)
	{
        Logger::Error("[EGLRenderer::InitializeGL] Can't initialize display");
		return false;
	}
    
    if (!eglInitialize(mainThreadContext->display, 0, 0))
    {
        Logger::Error("[EGLRenderer::InitializeGL] eglInitialize failed");
		return false;
	}
    
    if (!mainThreadContext->CreateEGLContext(NULL))
	{
        Logger::Error("[EGLRenderer::InitializeGL] Can't create context");
		return false;
	}
    
//    glGenFramebuffers(1, &defaultFramebuffer);
//    glGenRenderbuffers(1, &colorRenderbuffer);
//    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
    
    return true;
}

void EGLRenderer::Shutdown()
{
    Logger::Debug("[EGLRenderer::Shutdown]");
    
    
//    if (defaultFramebuffer)
//	{
//		glDeleteFramebuffers(1, &defaultFramebuffer);
//		defaultFramebuffer = 0;
//	}
//    
//	if (colorRenderbuffer)
//	{
//		glDeleteRenderbuffers(1, &colorRenderbuffer);
//		colorRenderbuffer = 0;
//	}
    
    mainThreadContext->Unbind();
    mainThreadContext->DestroySurface();
    mainThreadContext->DestroyContext();
    
    if(mainThreadContext->display != EGL_NO_DISPLAY)
    {
        eglTerminate(mainThreadContext->display);
        mainThreadContext->display = EGL_NO_DISPLAY;
    }
}
    
void EGLRenderer::SwapBuffers()
{
//    if (m_status < NV_IS_BOUND)
//		return false;

	mainThreadContext->Swap();
}
    
void EGLRenderer::SetWindow(ANativeWindow* window)
{
    Logger::Debug("[EGLRenderer::SetWindow] Window has changed!");
    mainThreadContext->Unbind();
    mainThreadContext->DestroySurface();

    if (window)
	{
        mainThreadContext->CreateWindowSurface(window);
        mainThreadContext->Bind();
    }
    
//    Resize();
}
    


void EGLRenderer::BeginFrame()
{
//    glDepthMask(GL_TRUE);
//    RENDER_VERIFY(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
//    
//    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
//    glViewport(0, 0, backingWidth, backingHeight);
}

void EGLRenderer::EndFrame()
{
//    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
//    const GLenum discards[]  = {GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0};
//    RENDER_VERIFY(glDiscardFramebuffer(GL_FRAMEBUFFER,2,discards));

//    [context presentRenderbuffer:GL_RENDERBUFFER];

    SwapBuffers();
}
    
void EGLRenderer::Resize()
{
//    backingWidth = (GLint)width;
//	backingHeight = (GLint)height;
//    
//    if(colorRenderbuffer)
//    {
//        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
//
//        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
//        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
//        
//        glGenRenderbuffers(1, &depthRenderbuffer);
//        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
//        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, backingWidth, backingHeight);
//        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
//        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
//        
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        {
//            Logger::Error("[EGLRenderer::Resize] cannot correctly initialize renderbuffers");
//        }
//    }
}
    
    
float32 EGLRenderer::GetWidth()
{
    return mainThreadContext->width;
}

float32 EGLRenderer::GetHeight()
{
    return mainThreadContext->height;
}

int32 EGLRenderer::GetColorRenderbuffer()
{
    return 0;
}
    
int32 EGLRenderer::GetDefaultFramebuffer()
{
    return 0;
}

ThreadContext * EGLRenderer::CreateThreadContext()
{
    Logger::Debug("[EGLRenderer::CreateThreadContext]");

    ThreadContext *context = new ThreadContext();
    
    context->display = mainThreadContext->display;
    context->CreateEGLContext(mainThreadContext->context);
    
    context->CreateBufferSurface(mainThreadContext->width, mainThreadContext->height);
    
    return context;
}
    
void EGLRenderer::BindThreadContext(ThreadContext *context)
{
    Logger::Debug("[EGLRenderer::BindThreadContext]");
    
    context->Bind();
}
    
void EGLRenderer::UnbindThreadContext(ThreadContext *context)
{
    Logger::Debug("[EGLRenderer::UnbindThreadContext]");

    context->Unbind();
}
    
void EGLRenderer::ReleaseThreadContext(ThreadContext *context)
{
    Logger::Debug("[EGLRenderer::ReleaseThreadContext]");

    if(context)
    {
        context->DestroySurface();
        context->DestroyContext();
        
        context->Release();
    }
}

    
}
#endif // #if defined(__DAVAENGINE_ANDROID__)
