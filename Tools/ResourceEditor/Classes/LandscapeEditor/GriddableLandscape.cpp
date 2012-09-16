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
#include "GriddableLandscape.h"
#include "LandscapeRenderer.h"

using namespace DAVA;

GriddableLandscape::GriddableLandscape()
    : EditorLandscapeNode()
{
    SetName(String("GriddableLandscape"));

    griddableSprite = Sprite::CreateAsRenderTarget(TEXTURE_TILE_FULL_SIZE, TEXTURE_TILE_FULL_SIZE, DAVA::FORMAT_RGBA8888);
}

GriddableLandscape::~GriddableLandscape()
{
    SafeRelease(griddableSprite);
}


void GriddableLandscape::HeihghtmapUpdated(const DAVA::Rect &forRect)
{
    EditorLandscapeNode::HeihghtmapUpdated(forRect);
    
    Vector3 landSize;
    AABBox3 transformedBox;
    landscape->GetBoundingBox().GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;
    
    Texture *notPassableMap = griddableSprite->GetTexture();
    float32 dx = (float32)notPassableMap->GetWidth() / (float32)(heightmap->Size() - 1);
    
    RenderManager::Instance()->LockNonMain();
    RenderManager::Instance()->SetRenderTarget(griddableSprite);
    
    RenderManager::Instance()->ClipPush();
    RenderManager::Instance()->ClipRect(Rect(forRect.x * dx, forRect.y * dx, forRect.dx * dx, forRect.dy * dx));

    DrawFullTiledTexture();
    
    int32 lastY = (int32)(forRect.y + forRect.dy);
    int32 lastX = (int32)(forRect.x + forRect.dx);
    for (int32 y = (int32)forRect.y; y < lastY; ++y)
    {
        int32 yOffset = y * heightmap->Size();
        for (int32 x = (int32)forRect.x; x < lastX; ++x)
        {
            float32 ydx = y * dx;
            float32 xdx = x * dx;

            RenderManager::Instance()->SetColor(Color::White());
            RenderHelper::Instance()->DrawLine(Vector2(xdx, ydx),
                                               Vector2((xdx + dx), ydx));

            RenderManager::Instance()->SetColor(Color::White());
            RenderHelper::Instance()->DrawLine(Vector2(xdx, ydx),
                                               Vector2(xdx, (ydx + dx)));
        }
    }
    
    RenderManager::Instance()->ClipPop();

    RenderManager::Instance()->RestoreRenderTarget();
    RenderManager::Instance()->UnlockNonMain();
}


void GriddableLandscape::DrawFullTiledTexture()
{
    Texture *notPassableMap = griddableSprite->GetTexture();
    Texture *fullTiledTexture = landscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL);
    Sprite *background = Sprite::CreateFromTexture(fullTiledTexture, 0, 0, fullTiledTexture->GetWidth(), fullTiledTexture->GetHeight());
    background->SetPosition(0.f, 0.f);
    background->SetScaleSize(griddableSprite->GetWidth(), griddableSprite->GetHeight());
    
    background->Draw();
}


void GriddableLandscape::SetDisplayedTexture()
{
    SetTexture(LandscapeNode::TEXTURE_TILE_FULL, griddableSprite->GetTexture());
}

  

