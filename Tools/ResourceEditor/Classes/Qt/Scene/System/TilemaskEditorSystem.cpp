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

#include "TilemaskEditorSystem.h"
#include "CollisionSystem.h"
#include "SelectionSystem.h"
#include "ModifSystem.h"
#include "LandscapeEditorDrawSystem.h"
#include "../SceneEditorProxy.h"
#include "LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "LandscapeEditorDrawSystem/LandscapeProxy.h"

TilemaskEditorSystem::TilemaskEditorSystem(Scene* scene)
:	SceneSystem(scene)
,	enabled(false)
,	editingIsEnabled(false)
,	curToolSize(0)
,	cursorSize(30)
,	toolImage(NULL)
,	toolImageSprite(NULL)
,	strength(0)
,	toolImagePath("")
,	tileTextureNum(0)
,	maskSprite(NULL)
,	oldMaskSprite(NULL)
,	toolSprite(NULL)
{
	cursorTexture = Texture::CreateFromFile("~res:/LandscapeEditor/Tools/cursor/cursor.png");
	cursorTexture->SetWrapMode(Texture::WRAP_CLAMP_TO_EDGE, Texture::WRAP_CLAMP_TO_EDGE);
	
    tileMaskEditorShader = new Shader();
	tileMaskEditorShader->LoadFromYaml("~res:/Shaders/Landscape/tilemask-editor.shader");
	tileMaskEditorShader->Recompile();
	
	collisionSystem = ((SceneEditorProxy *) GetScene())->collisionSystem;
	selectionSystem = ((SceneEditorProxy *) GetScene())->selectionSystem;
	modifSystem = ((SceneEditorProxy *) GetScene())->modifSystem;
	drawSystem = ((SceneEditorProxy *) GetScene())->landscapeEditorDrawSystem;
}

TilemaskEditorSystem::~TilemaskEditorSystem()
{
	SafeRelease(cursorTexture);
	SafeRelease(tileMaskEditorShader);
	SafeRelease(toolImage);
	SafeRelease(toolImageSprite);
}

bool TilemaskEditorSystem::EnableLandscapeEditing()
{
	if (enabled)
	{
		return true;
	}
	
	selectionSystem->SetLocked(true);
	modifSystem->SetLocked(true);
	
	drawSystem->EnableCustomDraw();
	drawSystem->GetLandscapeProxy()->SetTilemaskTextureEnabled(true);

	landscapeSize = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE_FULL)->GetWidth();

	drawSystem->EnableCursor(landscapeSize);
	drawSystem->SetCursorTexture(cursorTexture);
	drawSystem->SetCursorSize(cursorSize);
	
	CreateMaskTexture();
	
	enabled = true;
	return enabled;
}

bool TilemaskEditorSystem::DisableLandscapeEdititing()
{
	if (!enabled)
	{
		return true;
	}
	
	selectionSystem->SetLocked(false);
	modifSystem->SetLocked(false);
	
	drawSystem->GetLandscapeProxy()->SetTilemaskTextureEnabled(false);
	drawSystem->GetLandscapeProxy()->SetTilemaskTexture(NULL);
	
	drawSystem->DisableCursor();
	drawSystem->DisableCustomDraw();
	
	enabled = false;
	return !enabled;
}

bool TilemaskEditorSystem::IsLandscapeEditingEnabled() const
{
	return enabled;
}

void TilemaskEditorSystem::Update(float32 timeElapsed)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	if (editingIsEnabled && isIntersectsLandscape)
	{
		float32 landscapeSize = drawSystem->GetHeightmapProxy()->Size();
		float32 textureSize = toolSprite->GetSize().x;
		float32 scaleFactor = textureSize / landscapeSize;
		
		Vector2 toolSize = Vector2(cursorSize, cursorSize) * scaleFactor;
		Vector2 toolPos = cursorPosition * scaleFactor - toolSize / 2.f;
		
		RenderManager::Instance()->SetRenderTarget(toolSprite);
		toolImageSprite->SetScaleSize(toolSize.x, toolSize.y);
		toolImageSprite->SetPosition(toolPos.x, toolPos.y);
		toolImageSprite->Draw();
		RenderManager::Instance()->RestoreRenderTarget();
		
		UpdateBrushTool(timeElapsed);
	}
}

void TilemaskEditorSystem::ProcessUIEvent(UIEvent* event)
{
	if (!IsLandscapeEditingEnabled())
	{
		return;
	}
	
	UpdateCursorPosition();
	
	if (event->tid == UIEvent::BUTTON_1)
	{
		Vector3 point;
		
		switch(event->phase)
		{
			case UIEvent::PHASE_BEGAN:
				if (isIntersectsLandscape)
				{
					UpdateToolImage();
					editingIsEnabled = true;
				}
				break;
				
			case UIEvent::PHASE_DRAG:
				break;
				
			case UIEvent::PHASE_ENDED:
				if (editingIsEnabled)
				{
					editingIsEnabled = false;
				}
				break;
		}
	}
}

void TilemaskEditorSystem::SetBrushSize(int32 brushSize)
{
	if (brushSize > 0)
	{
		cursorSize = (uint32)brushSize;
		drawSystem->SetCursorSize(cursorSize);
	}
}

void TilemaskEditorSystem::SetStrength(float32 strength)
{
	if (strength >= 0)
	{
		this->strength = strength;
	}
}

void TilemaskEditorSystem::SetToolImage(const FilePath& toolImagePath)
{
	this->toolImagePath = toolImagePath;
	UpdateToolImage(true);
}

void TilemaskEditorSystem::SetTileTexture(uint32 tileTexture)
{
	if (tileTexture >= GetTileTextureCount())
	{
		return;
	}
	
	tileTextureNum = tileTexture;
}

void TilemaskEditorSystem::UpdateCursorPosition()
{
	Vector3 landPos;
	isIntersectsLandscape = false;
	if (collisionSystem->LandRayTestFromCamera(landPos))
	{
		isIntersectsLandscape = true;
		Vector2 point(landPos.x, landPos.y);
		
		point.x = (float32)((int32)point.x);
		point.y = (float32)((int32)point.y);
		
		AABBox3 box = drawSystem->GetLandscapeProxy()->GetLandscapeBoundingBox();
		
		cursorPosition.x = (point.x - box.min.x) * (landscapeSize - 1) / (box.max.x - box.min.x);
		cursorPosition.y = (point.y - box.min.y) * (landscapeSize - 1) / (box.max.y - box.min.y);
		
		drawSystem->SetCursorPosition(cursorPosition);
	}
}

void TilemaskEditorSystem::UpdateToolImage(bool force)
{
	if (toolImage)
	{
		if (curToolSize != cursorSize || force)
		{
			SafeRelease(toolImage);
		}
	}
	
	if (!toolImage)
	{
		if (!toolImagePath.IsEmpty())
		{
			toolImage = CreateToolImage(32, toolImagePath);
			curToolSize = cursorSize;
		}
	}
}

void TilemaskEditorSystem::UpdateBrushTool(float32 timeElapsed)
{
	if (!toolImage)
	{
		DAVA::Logger::Error("Tool image is empty!");
		return;
	}
	
	int32 colorType = (int32)tileTextureNum;
	
	RenderManager::Instance()->SetRenderTarget(maskSprite);
	
	srcBlendMode = RenderManager::Instance()->GetSrcBlend();
	dstBlendMode = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
	
	RenderManager::Instance()->SetShader(tileMaskEditorShader);
	oldMaskSprite->PrepareSpriteRenderData(0);
	RenderManager::Instance()->SetRenderData(oldMaskSprite->spriteRenderObject);
	RenderManager::Instance()->SetTexture(oldMaskSprite->GetTexture(), 0);
	RenderManager::Instance()->SetTexture(toolSprite->GetTexture(), 1);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->AttachRenderData();
	
	int32 tex0 = tileMaskEditorShader->FindUniformLocationByName("texture0");
	tileMaskEditorShader->SetUniformValue(tex0, 0);
	int32 tex1 = tileMaskEditorShader->FindUniformLocationByName("texture1");
	tileMaskEditorShader->SetUniformValue(tex1, 1);
	int32 colorTypeUniform = tileMaskEditorShader->FindUniformLocationByName("colorType");
	tileMaskEditorShader->SetUniformValue(colorTypeUniform, colorType);
	int32 intensityUniform = tileMaskEditorShader->FindUniformLocationByName("intensity");
	
	tileMaskEditorShader->SetUniformValue(intensityUniform, strength);
	
	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, 0, 4);
	
	RenderManager::Instance()->SetBlendMode(srcBlendMode, dstBlendMode);
	RenderManager::Instance()->RestoreRenderTarget();
	RenderManager::Instance()->SetColor(Color::White());
	
	drawSystem->GetLandscapeProxy()->SetTilemaskTexture(maskSprite->GetTexture());
	Sprite * temp = oldMaskSprite;
	oldMaskSprite = maskSprite;
	maskSprite = temp;
	
	RenderManager::Instance()->SetRenderTarget(toolSprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	RenderManager::Instance()->RestoreRenderTarget();
}

Image* TilemaskEditorSystem::CreateToolImage(int32 sideSize, const FilePath& filePath)
{
	RenderManager::Instance()->LockNonMain();
	
	Sprite *dstSprite = Sprite::CreateAsRenderTarget(sideSize, sideSize, FORMAT_RGBA8888);
	Texture *srcTex = Texture::CreateFromFile(filePath);
	Sprite *srcSprite = Sprite::CreateFromTexture(srcTex, 0, 0, (float32)srcTex->GetWidth(), (float32)srcTex->GetHeight());
	
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
	
	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	RenderManager::Instance()->SetColor(Color::White());
	
	srcSprite->SetScaleSize((float32)sideSize, (float32)sideSize);
	srcSprite->SetPosition(Vector2((dstSprite->GetTexture()->GetWidth() - sideSize)/2.0f,
								   (dstSprite->GetTexture()->GetHeight() - sideSize)/2.0f));
	srcSprite->Draw();
	RenderManager::Instance()->RestoreRenderTarget();
	
	SafeRelease(toolImageSprite);
	toolImageSprite = SafeRetain(dstSprite);
	Image *retImage = dstSprite->GetTexture()->CreateImageFromMemory();
	
	SafeRelease(srcSprite);
	SafeRelease(srcTex);
	SafeRelease(dstSprite);
	
	RenderManager::Instance()->UnlockNonMain();
	
	return retImage;
}

void TilemaskEditorSystem::AddRectToAccumulator(const Rect& rect)
{
	updatedRectAccumulator = updatedRectAccumulator.Combine(rect);
}

void TilemaskEditorSystem::ResetAccumulatorRect()
{
	float32 inf = std::numeric_limits<float32>::infinity();
	updatedRectAccumulator = Rect(inf, inf, -inf, -inf);
}

Rect TilemaskEditorSystem::GetUpdatedRect()
{
	EditorHeightmap* editorHeightmap = drawSystem->GetHeightmapProxy();
	
	float32 heightmapSize = editorHeightmap->Size() - 1.f;
	Rect r = updatedRectAccumulator;
	
	r.x = Max(r.x, 0.f);
	r.y = Max(r.y, 0.f);
	r.dx = Min(r.dx, heightmapSize);
	r.dy = Min(r.dy, heightmapSize);
	
	return r;
}

uint32 TilemaskEditorSystem::GetTileTextureCount() const
{
	return 4;
}

Texture* TilemaskEditorSystem::GetTileTexture(int32 index)
{
	if (index < 0 || index >= (int32)GetTileTextureCount())
	{
		return NULL;
	}
	
	Landscape::eTextureLevel level = (Landscape::eTextureLevel)(Landscape::TEXTURE_TILE0 + index);
	Texture* texture = drawSystem->GetLandscapeProxy()->GetLandscapeTexture(level);
	
	return texture;
}

void TilemaskEditorSystem::CreateMaskTexture()
{
	CreateMaskFromTexture(drawSystem->GetLandscapeProxy()->GetLandscapeTexture(Landscape::TEXTURE_TILE_MASK));
}

void TilemaskEditorSystem::CreateMaskFromTexture(Texture* texture)
{
    SafeRelease(maskSprite);
	SafeRelease(oldMaskSprite);
    SafeRelease(toolSprite);
	
	float32 texSize = 1024;
	if(texture)
	{
		texSize = (float32)texture->GetWidth();
	}
    
	maskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	oldMaskSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	toolSprite = Sprite::CreateAsRenderTarget(texSize, texSize, FORMAT_RGBA8888);
	
	if(texture)
	{
		RenderManager::Instance()->LockNonMain();
		RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ZERO);
		
		Sprite *oldMask = Sprite::CreateFromTexture(texture, 0, 0,
													(float32)texture->GetWidth(), (float32)texture->GetHeight());
		
		RenderManager::Instance()->SetRenderTarget(oldMaskSprite);
		oldMask->SetPosition(0.f, 0.f);
		oldMask->Draw();
		RenderManager::Instance()->RestoreRenderTarget();
		
		RenderManager::Instance()->SetRenderTarget(maskSprite);
		oldMask->SetPosition(0.f, 0.f);
		oldMask->Draw();
		RenderManager::Instance()->RestoreRenderTarget();
		
		SafeRelease(oldMask);
		
		RenderManager::Instance()->UnlockNonMain();
	}
	
	drawSystem->GetLandscapeProxy()->SetTilemaskTexture(oldMaskSprite->GetTexture());
}
