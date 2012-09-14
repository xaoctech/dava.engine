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
#include "NotPassableTerrain.h"

using namespace DAVA;

NotPassableTerrain::NotPassableTerrain(DAVA::LandscapeNode *land)
    : SceneNode()
{
    SetName(String("NotPassableTerrain"));

    notPassableAngleTan = (float32)tan(DegToRad((float32)NotPassableTerrain::NOT_PASSABLE_ANGLE));
    hardPassableAngleTan = (float32)tan(DegToRad((float32)NotPassableTerrain::HARD_PASSABLE_ANGLE));

    terrainRenderObject = new RenderDataObject();
    DVASSERT(terrainRenderObject);
    
    DVASSERT(land);
    landscape = SafeRetain(land);

    InitializeRenderData();
    
    Texture *tex = landscape->GetTexture(LandscapeNode::TEXTURE_TILE_MASK);
    DVASSERT(tex);
    
    notPassableMap = Texture::CreateFBO(tex->GetWidth(), tex->GetHeight(), DAVA::FORMAT_RGBA8888, Texture::DEPTH_NONE);
    
    BuildMapForLandscape();
}

NotPassableTerrain::~NotPassableTerrain()
{
    SafeRelease(notPassableMap);
    SafeRelease(landscape);
    
    SafeRelease(shader);
    uniformFogColor = -1;
    uniformFogDensity = -1;
    
    SafeRelease(terrainRenderObject);
}

void NotPassableTerrain::InitializeRenderData()
{
    Heightmap *heightmap = landscape->GetHeightmap();
    DVASSERT(heightmap);

    vertices.resize(heightmap->Size() * heightmap->Size());
	indices.resize(heightmap->Size() * heightmap->Size() * 6);
    
    InitShader();
    
    RebuildVertexes();
    RebuildIndexes();
    
    terrainRenderObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(LandscapeNode::LandscapeVertex), &vertices[0].position);
	terrainRenderObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(LandscapeNode::LandscapeVertex), &vertices[0].texCoord);
}

void NotPassableTerrain::InitShader()
{
    bool isFogEnabled = false;
    
    shader = new Shader();
    shader->LoadFromYaml("~res:/Shaders/Landscape/fulltiled_texture.shader");
	if(isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
    {
        shader->SetDefineList("VERTEX_FOG");
    }
    
    shader->Recompile();
    
    if(isFogEnabled && RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::FOG_ENABLE))
    {
        uniformFogColor = shader->FindUniformLocationByName("fogColor");
        uniformFogDensity = shader->FindUniformLocationByName("fogDensity");
    }
    else
    {
        uniformFogColor = -1;
        uniformFogDensity = -1;
    }
}

void NotPassableTerrain::RebuildVertexes()
{
    Heightmap *heightmap = landscape->GetHeightmap();
    
    for (int32 y = 0; y < heightmap->Size(); ++y)
    {
        int32 index = y * heightmap->Size();
        for (int32 x = 0; x < heightmap->Size(); ++x)
        {
            vertices[index + x].position = landscape->GetPoint(x, y, heightmap->Data()[y * heightmap->Size() + x]);
            vertices[index + x].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), (float32)y / (float32)(heightmap->Size() - 1));
        }
    }
}

void NotPassableTerrain::RebuildIndexes()
{
    Heightmap *heightmap = landscape->GetHeightmap();

    int32 step = 1;
    int32 indexIndex = 0;
    int32 quadWidth = heightmap->Size();
    for(int32 y = 0; y < heightmap->Size() - 1; y += step)
    {
        for(int32 x = 0; x < heightmap->Size() - 1; x += step)
        {
            indices[indexIndex++] = x + y * quadWidth;
            indices[indexIndex++] = (x + step) + y * quadWidth;
            indices[indexIndex++] = x + (y + step) * quadWidth;
            
            indices[indexIndex++] = (x + step) + y * quadWidth;
            indices[indexIndex++] = (x + step) + (y + step) * quadWidth;
            indices[indexIndex++] = x + (y + step) * quadWidth;
        }
    }
}

void NotPassableTerrain::BuildMapForLandscape()
{
    Vector3 landSize;
    AABBox3 transformedBox;
    landscape->GetBoundingBox().GetTransformedBox(landscape->GetWorldTransform(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;

    Heightmap *heightmap = landscape->GetHeightmap();
    float32 angleCellDistance = landSize.x / (float32)(heightmap->Size() - 1);
    float32 angleHeightDelta = landSize.z / (float32)(Heightmap::MAX_VALUE - 1);
    
    float32 dx = (float32)notPassableMap->GetWidth() / (float32)(heightmap->Size() - 1);
    
    Rect oldViewport = RenderManager::Instance()->GetViewport();
    RenderManager::Instance()->SetRenderTarget(notPassableMap);
    RenderManager::Instance()->SetViewport(Rect(0, 0, (float32)(notPassableMap->width), (float32)(notPassableMap->height)), true);

    Texture *fullTiledTexture = landscape->GetTexture(LandscapeNode::TEXTURE_TILE_FULL);
    Sprite *background = Sprite::CreateFromTexture(fullTiledTexture, 0, 0, fullTiledTexture->GetWidth(), fullTiledTexture->GetHeight());
    background->SetPosition(0.f, 0.f);
    background->SetScaleSize(notPassableMap->GetWidth(), notPassableMap->GetHeight());
    background->Draw();
    
    for (int32 y = 0; y < heightmap->Size() - 2; ++y)
    {
        int32 yOffset = y * heightmap->Size();
        for (int32 x = 0; x < heightmap->Size() - 2; ++x)
        {
            uint16 currentPoint = heightmap->Data()[yOffset + x];
            uint16 rightPoint = heightmap->Data()[yOffset + x + 1];
            uint16 bottomPoint = heightmap->Data()[yOffset + x + heightmap->Size()];
            
            uint16 deltaRight = (uint16)abs((int32)currentPoint - (int32)rightPoint);
            uint16 deltaBottom = (uint16)abs((int32)currentPoint - (int32)bottomPoint);
            
            float32 tanRight = (float32)deltaRight * (float32)angleHeightDelta / (float32)angleCellDistance;
            float32 tanBottom = (float32)deltaBottom * (float32)angleHeightDelta / (float32)angleCellDistance;
            
            float32 ydx = y * dx;
            float32 xdx = x * dx;
            
            RenderManager::Instance()->SetColor(GetColorForAngle(tanRight));
            RenderHelper::Instance()->DrawLine(Vector2(xdx, ydx), Vector2(xdx + dx, ydx));

            RenderManager::Instance()->SetColor(GetColorForAngle(tanBottom));
            RenderHelper::Instance()->DrawLine(Vector2(xdx, ydx), Vector2(xdx, ydx + dx));
        }
    }
    
#ifdef __DAVAENGINE_OPENGL__
	BindFBO(RenderManager::Instance()->GetFBOViewFramebuffer());
#endif //#ifdef __DAVAENGINE_OPENGL__
    
	RenderManager::Instance()->SetViewport(oldViewport, true);
}


DAVA::Color NotPassableTerrain::GetColorForAngle(DAVA::float32 tanOfAngle)
{
    if(notPassableAngleTan <= tanOfAngle)
    {
        return Color(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else if(hardPassableAngleTan <= tanOfAngle)
    {
        return Color(1.0f, 0.4f, 0.0f, 1.0f);
    }

    return Color(0.0f, 1.0f, 0.0f, 1.0f);
}



void NotPassableTerrain::Draw()
{
    if (!(flags & NODE_VISIBLE)) return;

    Heightmap *heightmap = landscape->GetHeightmap();
    
    BindMaterial();
    
    RenderManager::Instance()->SetRenderData(terrainRenderObject);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
	RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, &indices.front());
    
    UnbindMaterial();
}
    
void NotPassableTerrain::BindMaterial()
{
    RenderManager::Instance()->SetTexture(notPassableMap, 0);
    
    RenderManager::Instance()->SetShader(shader);
    RenderManager::Instance()->FlushState();
    
    
    float32 fogDensity = 0.006f;
    Color fogColor(Color::White());

    if (uniformFogColor != -1)
        shader->SetUniformValue(uniformFogColor, fogColor);
    if (uniformFogDensity != -1)
        shader->SetUniformValue(uniformFogDensity, fogDensity);
}

void NotPassableTerrain::UnbindMaterial()
{
    RenderManager::Instance()->SetTexture(0, 0);
    
    RenderManager::Instance()->SetShader(NULL);
    RenderManager::Instance()->FlushState();
}

