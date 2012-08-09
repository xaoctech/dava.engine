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
#include "Render/Image.h"
#include "Render/RenderManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/LandscapeDebugNode.h"
#include "Scene3D/Heightmap.h"

namespace DAVA
{
    
LandscapeDebugNode::LandscapeDebugNode()
    : LandscapeNode()
{
    debugRenderDataObject = new RenderDataObject();
}

LandscapeDebugNode::~LandscapeDebugNode()
{
    SafeRelease(debugRenderDataObject);
}
    
void LandscapeDebugNode::SetDebugHeightmapImage(Heightmap * _debugHeightmapImage, const AABBox3 & _box)
{
    box = _box;
    
    SafeRelease(heightmap);
    heightmap = SafeRetain(_debugHeightmapImage);

    debugVertices.resize(heightmap->Size() * heightmap->Size());
	debugIndices.resize(heightmap->Size() * heightmap->Size() * 6);
    
    ReleaseShaders();
    InitShaders();
}

void LandscapeDebugNode::Draw()
{
    if(0 == heightmap->Size())
        return;
    
    BindMaterial(0);

    int32 index = 0;
	for (int32 y = 0; y < heightmap->Size(); ++y)
	{
		for (int32 x = 0; x < heightmap->Size(); ++x)
		{
			debugVertices[index].position = GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
			debugVertices[index].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), (float32)y / (float32)(heightmap->Size() - 1));           
			index++;
		}
	}

	int32 step = 1;
	int32 indexIndex = 0;
	int32 quadWidth = heightmap->Size();
	for(int32 y = 0; y < heightmap->Size() - 1; y += step)
	{
		for(int32 x = 0; x < heightmap->Size() - 1; x += step)
		{
			debugIndices[indexIndex++] = x + y * quadWidth;
			debugIndices[indexIndex++] = (x + step) + y * quadWidth;
			debugIndices[indexIndex++] = x + (y + step) * quadWidth;

			debugIndices[indexIndex++] = (x + step) + y * quadWidth;
			debugIndices[indexIndex++] = (x + step) + (y + step) * quadWidth;
			debugIndices[indexIndex++] = x + (y + step) * quadWidth;     
		}
	}

    debugRenderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(LandscapeVertex), &debugVertices[0].position); 
	debugRenderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(LandscapeVertex), &debugVertices[0].texCoord); 

#if defined(__DAVAENGINE_OPENGL__)
    if (debugFlags & DEBUG_DRAW_GRID)
    {
        debugFlags &= ~DEBUG_DRAW_GRID;
        DrawLandscape();
        debugFlags |= DEBUG_DRAW_GRID;
        
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        RenderManager::Instance()->SetColor(1.0f, 1.f, 1.f, 1.f);
        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
        RenderManager::Instance()->SetShader(0);
        RenderManager::Instance()->FlushState();
        
    }
#endif //#if defined(__DAVAENGINE_OPENGL__)

    DrawLandscape();
    
#if defined(__DAVAENGINE_OPENGL__)
    if (debugFlags & DEBUG_DRAW_ALL)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
#endif //#if defined(__DAVAENGINE_OPENGL__)

    
	if(cursor)
	{
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
		eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);
		cursor->Prepare();

		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, &debugIndices.front()); 

		RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(src, dst);
	}
    
    UnbindMaterial();
}
    
void LandscapeDebugNode::DrawLandscape()
{
    RenderManager::Instance()->SetRenderData(debugRenderDataObject);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
	RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, &debugIndices.front()); 
}

    
    
void LandscapeDebugNode::SetHeightmapPath(const String &path)
{
    heightmapPath = path;
}
    
};





