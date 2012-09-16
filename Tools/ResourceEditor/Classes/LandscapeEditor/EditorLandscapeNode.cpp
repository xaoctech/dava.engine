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
#include "EditorLandscapeNode.h"
#include "LandscapeRenderer.h"
#include "EditorHeightmap.h"

using namespace DAVA;

EditorLandscapeNode::EditorLandscapeNode()
    : LandscapeNode()
{
    landscapeRenderer = NULL;
    landscape = NULL;
}

EditorLandscapeNode::~EditorLandscapeNode()
{
    SafeRelease(landscapeRenderer);
    SafeRelease(landscape);
}

void EditorLandscapeNode::SetLandscape(DAVA::LandscapeNode *landscapeNode)
{
    SafeRelease(landscape);
    landscape = SafeRetain(landscapeNode);
    
    SetName(landscape->GetName());
    heightmapPath = landscape->GetHeightmapPathname();
    SetDebugFlags(landscape->GetDebugFlags());
    
    SetTiledShaderMode(landscape->GetTiledShaderMode());
    for(int32 iTex = 0; iTex < LandscapeNode::TEXTURE_COUNT; ++iTex)
    {
        SetTexture((LandscapeNode::eTextureLevel)iTex,
                                       landscape->GetTexture((LandscapeNode::eTextureLevel)iTex));
        
        SetTextureTiling((LandscapeNode::eTextureLevel)iTex,
                                             landscape->GetTextureTiling((LandscapeNode::eTextureLevel)iTex));
    }

    SetHeightmap(landscape->GetHeightmap());
    
    box = landscape->GetBoundingBox();

    SetDisplayedTexture();
}


void EditorLandscapeNode::SetHeightmap(DAVA::Heightmap *height)
{
    LandscapeNode::SetHeightmap(height);
    
    EditorHeightmap *editorHeightmap = dynamic_cast<EditorHeightmap *>(height);
    if(editorHeightmap)
    {
        HeihghtmapUpdated(Rect(0, 0, (float32)editorHeightmap->Size() - 1.f, (float32)editorHeightmap->Size() - 1.f));
    }
}


void EditorLandscapeNode::SetRenderer(LandscapeRenderer *renderer)
{
    SafeRelease(landscapeRenderer);
    landscapeRenderer = SafeRetain(renderer);
}


void EditorLandscapeNode::Draw()
{
    if (!(flags & NODE_VISIBLE)) return;
    if(!landscapeRenderer) return;
    
    landscapeRenderer->BindMaterial(GetTexture(LandscapeNode::TEXTURE_TILE_FULL));
    landscapeRenderer->DrawLandscape();
    landscapeRenderer->UnbindMaterial();
}
    


void EditorLandscapeNode::HeihghtmapUpdated(const DAVA::Rect &forRect)
{
    EditorLandscapeNode *editorLandscape = dynamic_cast<EditorLandscapeNode *>(landscape);
    if(editorLandscape)
    {
        editorLandscape->HeihghtmapUpdated(forRect);
    }
}

void EditorLandscapeNode::SetDisplayedTexture()
{
    
}

DAVA::LandscapeNode *EditorLandscapeNode::GetEditedLandscape()
{
    return landscape;
}


