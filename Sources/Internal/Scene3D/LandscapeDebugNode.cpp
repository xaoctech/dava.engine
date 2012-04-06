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

namespace DAVA
{
    
LandscapeDebugNode::LandscapeDebugNode(Scene * scene)
    : LandscapeNode(scene)
{
    debugRenderDataObject = new RenderDataObject();
}

LandscapeDebugNode::~LandscapeDebugNode()
{
    SafeRelease(debugRenderDataObject);
}
    
void LandscapeDebugNode::SetDebugHeightmapImage(Image * _debugHeightmapImage, const AABBox3 & _box)
{
    box = _box;
    heightmap = SafeRetain(_debugHeightmapImage);

    debugVertices.resize(heightmap->GetWidth() * heightmap->GetHeight());
	debugIndices.resize(heightmap->GetWidth() * heightmap->GetHeight() * 6);
    
    ReleaseShaders();
    InitShaders();
}

void LandscapeDebugNode::Draw()
{
    BindMaterial();
    
	int32 index = 0;
	for (int32 y = 0; y < heightmap->GetHeight(); ++y)
	{
		for (int32 x = 0; x < heightmap->GetWidth(); ++x)
		{
			debugVertices[index].position = GetPoint(x, y, heightmap->GetData()[y * heightmap->GetWidth() + x]);
			debugVertices[index].texCoord = Vector2((float32)x / (float32)(heightmap->GetWidth() - 1), (float32)y / (float32)(heightmap->GetHeight() - 1));           
			index++;
		}
	}
    
	int32 step = 1;
	int32 indexIndex = 0;
	int32 quadWidth = heightmap->GetWidth();
	for(int32 y = 0; y < heightmap->GetHeight() - 1; y += step)
	{
		for(int32 x = 0; x < heightmap->GetWidth() - 1; x += step)
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
                                     
    RenderManager::Instance()->SetRenderData(debugRenderDataObject);
    RenderManager::Instance()->FlushState();
    RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->GetWidth() - 1) * (heightmap->GetHeight() - 1) * 6, EIF_32, &debugIndices.front()); 
    
    UnbindMaterial();
}
    
};





