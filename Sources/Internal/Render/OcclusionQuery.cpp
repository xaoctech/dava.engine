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
#include "Render/RenderManager.h"
#include "Render/OcclusionQuery.h"
#include "Math/RectPacker.h"

namespace DAVA
{
#if defined(__DAVAENGINE_OPENGL__)
OcclusionQuery::OcclusionQuery()
{
	RenderManager::Instance()->LockNonMain();
	
    queryActive = false;
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    RENDER_VERIFY(glGenQueries(1, &id));
#else
    RENDER_VERIFY(glGenQueriesEXT(1, &id));
#endif
	
	RenderManager::Instance()->UnlockNonMain();
}

OcclusionQuery::~OcclusionQuery()
{
	RenderManager::Instance()->LockNonMain();

#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    RENDER_VERIFY(glDeleteQueries(1, &id));
#else
    RENDER_VERIFY(glDeleteQueriesEXT(1, &id));
#endif
	
	RenderManager::Instance()->UnlockNonMain();

}

void OcclusionQuery::BeginQuery()
{
	RenderManager::Instance()->LockNonMain();

    queryActive = true;
// Temporarly written, should be refactored and moved to RenderBase.h defines
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    RENDER_VERIFY(glBeginQuery(GL_SAMPLES_PASSED, id));
#else
    RENDER_VERIFY(glBeginQueryEXT(GL_ANY_SAMPLES_PASSED_EXT, id));
#endif
	
	RenderManager::Instance()->UnlockNonMain();

}
    
void OcclusionQuery::EndQuery()
{
// Temporarly written, should be refactored and moved to RenderBase.h defines
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    RENDER_VERIFY(glEndQuery(GL_SAMPLES_PASSED));
#else
    RENDER_VERIFY(glEndQueryEXT(GL_ANY_SAMPLES_PASSED_EXT));
#endif
}
    
void OcclusionQuery::ResetResult()
{
    queryActive = false;
}

    
bool OcclusionQuery::IsResultAvailable()
{
    if (!queryActive)return false;
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    GLint available;
    RENDER_VERIFY(glGetQueryObjectiv(id,
                          GL_QUERY_RESULT_AVAILABLE_ARB,
                          &available));
    return (available != 0);
#else
    GLuint available;
    RENDER_VERIFY(glGetQueryObjectuivEXT(id,
                                     GL_QUERY_RESULT_AVAILABLE_EXT,
                                     &available));
    return (available != 0);
#endif
}
    
void OcclusionQuery::GetQuery(uint32 * resultValue)
{
#if defined(__DAVAENGINE_WIN32__) || defined(__DAVAENGINE_MACOS__)
    RENDER_VERIFY(glGetQueryObjectuiv(id, GL_QUERY_RESULT_ARB, resultValue));
#else
    RENDER_VERIFY(glGetQueryObjectuivEXT(id, GL_QUERY_RESULT_EXT, resultValue));
#endif
}
    
    
#else
#error "Require Occlusion Queries Implementation"
#endif

};
