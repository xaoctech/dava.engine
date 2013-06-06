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
 =====================================================================================*/

#include "LandscapeEditorDrawSystem.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/CustomColorsProxy.h"
#include "LandscapeEditorDrawSystem/NotPassableTerrainProxy.h"
#include "LandscapeEditor/LandscapeRenderer.h"

LandscapeEditorDrawSystem::LandscapeEditorDrawSystem(Scene* scene)
:	SceneSystem(scene)
,	customDrawRequestCount(0)
,	landscapeProxy(NULL)
,	heightmapProxy(NULL)
,	landscapeNode(NULL)
,	baseLandscape(NULL)
,	cursorTexture(NULL)
,	notPassableTerrainProxy(NULL)
,	customColorsProxy(NULL)
{
}

LandscapeEditorDrawSystem::~LandscapeEditorDrawSystem()
{
	SafeRelease(baseLandscape);
	SafeRelease(landscapeProxy);
	SafeRelease(heightmapProxy);
	SafeRelease(customColorsProxy);
}

LandscapeProxy* LandscapeEditorDrawSystem::GetLandscapeProxy()
{
	return landscapeProxy;
}

HeightmapProxy* LandscapeEditorDrawSystem::GetHeightmapProxy()
{
	return heightmapProxy;
}

CustomColorsProxy* LandscapeEditorDrawSystem::GetCustomColorsProxy()
{
	return customColorsProxy;
}

void LandscapeEditorDrawSystem::EnableCustomDraw()
{
	if (customDrawRequestCount != 0)
	{
		++customDrawRequestCount;
		return;
	}
	
	if (baseLandscape == 0)
	{
		landscapeNode = EditorScene::GetLandscapeNode(GetScene());
		baseLandscape = SafeRetain(GetLandscape(landscapeNode));
		
		landscapeProxy = new LandscapeProxy(baseLandscape);
		heightmapProxy = new HeightmapProxy(baseLandscape->GetHeightmap()->Clone(NULL));
		landscapeProxy->SetHeightmap(heightmapProxy);
		
		int32 size = baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();
		customColorsProxy = new CustomColorsProxy(size);
	}
	
	AABBox3 landscapeBoundingBox = baseLandscape->GetBoundingBox();
	LandscapeRenderer* landscapeRenderer = new LandscapeRenderer(heightmapProxy, landscapeBoundingBox);
	landscapeProxy->SetRenderer(landscapeRenderer);
	
	landscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
	landscapeNode->AddComponent(new RenderComponent(landscapeProxy));
	
	++customDrawRequestCount;
}

void LandscapeEditorDrawSystem::DisableCustomDraw()
{
	if (customDrawRequestCount == 0)
	{
		return;
	}
	
	--customDrawRequestCount;
	
	if (customDrawRequestCount == 0)
	{
		landscapeNode->RemoveComponent(Component::RENDER_COMPONENT);
		landscapeNode->AddComponent(new RenderComponent(baseLandscape));
		
		UpdateBaseLandscapeHeightmap();
	}
}

bool LandscapeEditorDrawSystem::IsNotPassableTerrainEnabled()
{
	if (!notPassableTerrainProxy)
	{
		return false;
	}
	
	return notPassableTerrainProxy->IsEnabled();
}

void LandscapeEditorDrawSystem::EnableNotPassableTerrain()
{
	if (!notPassableTerrainProxy)
	{
		notPassableTerrainProxy = new NotPassableTerrainProxy();
	}
	
	if (notPassableTerrainProxy->IsEnabled())
	{
		return;
	}
	
	EnableCustomDraw();
	notPassableTerrainProxy->Enable();
	notPassableTerrainProxy->UpdateTexture(heightmapProxy,
										   landscapeProxy->GetLandscapeBoundingBox(),
										   Rect(0.f, 0.f, (float32)(heightmapProxy->Size() - 1), (float32)(heightmapProxy->Size() - 1)));
	
	landscapeProxy->SetNotPassableTexture(notPassableTerrainProxy->GetTexture());
	landscapeProxy->SetNotPassableTextureEnabled(true);
}

void LandscapeEditorDrawSystem::DisableNotPassableTerrain()
{
	if (!notPassableTerrainProxy || !notPassableTerrainProxy->IsEnabled())
	{
		return;
	}
	
	notPassableTerrainProxy->Disable();
	landscapeProxy->SetNotPassableTexture(NULL);
	landscapeProxy->SetNotPassableTextureEnabled(false);
	
	DisableCustomDraw();
}

void LandscapeEditorDrawSystem::EnableCursor()
{
	landscapeProxy->CursorEnable();
	landscapeProxy->SetBigTextureSize((float32)heightmapProxy->Size());
}

void LandscapeEditorDrawSystem::DisableCursor()
{
	landscapeProxy->CursorDisable();
}

void LandscapeEditorDrawSystem::SetCursorTexture(Texture* cursorTexture)
{
	SafeRelease(this->cursorTexture);
	this->cursorTexture = SafeRetain(cursorTexture);
	
	landscapeProxy->SetCursorTexture(cursorTexture);
}

void LandscapeEditorDrawSystem::SetCursorSize(uint32 cursorSize)
{
	this->cursorSize = cursorSize;
	landscapeProxy->SetCursorScale((float32)cursorSize);
	UpdateCursorPosition();
}

void LandscapeEditorDrawSystem::SetCursorPosition(const Vector2& cursorPos)
{
	cursorPosition = cursorPos;// - Vector2(cursorSize / 2.f, cursorSize / 2.f);
	UpdateCursorPosition();
}

void LandscapeEditorDrawSystem::UpdateCursorPosition()
{
	Vector2 p = cursorPosition - Vector2(cursorSize / 2.f, cursorSize / 2.f);
	landscapeProxy->SetCursorPosition(p);
}

void LandscapeEditorDrawSystem::Update(DAVA::float32 timeElapsed)
{
	if (heightmapProxy && heightmapProxy->IsHeightmapChanged())
	{
		Rect changedRect = heightmapProxy->GetChangedRect();
		
		if (landscapeProxy)
		{
			landscapeProxy->GetRenderer()->RebuildVertexes(changedRect);
		}
		
		if (notPassableTerrainProxy && notPassableTerrainProxy->IsEnabled())
		{
			notPassableTerrainProxy->UpdateTexture(heightmapProxy, landscapeProxy->GetLandscapeBoundingBox(), changedRect);
			landscapeProxy->SetNotPassableTexture(notPassableTerrainProxy->GetTexture());
		}
		
		if (customDrawRequestCount == 0)
		{
			UpdateBaseLandscapeHeightmap();
		}
		
		heightmapProxy->ResetHeightmapChanged();
	}
	
	if (customColorsProxy &&  customColorsProxy->IsSpriteChanged())
	{
		if (landscapeProxy)
		{
			landscapeProxy->SetCustomColorsTexture(customColorsProxy->GetSprite()->GetTexture());
		}
		customColorsProxy->ResetSpriteChanged();
	}
}

void LandscapeEditorDrawSystem::UpdateBaseLandscapeHeightmap()
{
	Heightmap* h = new Heightmap();
	heightmapProxy->Clone(h);
	
	baseLandscape->SetHeightmap(h);
	
	SafeRelease(h);
}

Vector3 LandscapeEditorDrawSystem::GetLandscapeSize()
{
	AABBox3 transformedBox;
	baseLandscape->GetBoundingBox().GetTransformedBox(*baseLandscape->GetWorldTransformPtr(), transformedBox);
	
	Vector3 landSize = transformedBox.max - transformedBox.min;
	return landSize;
}

float32 LandscapeEditorDrawSystem::GetLandscapeMaxHeight()
{
	Vector3 landSize = GetLandscapeSize();
	return landSize.z;
}
