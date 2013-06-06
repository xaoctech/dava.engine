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

#include "LandscapeProxy.h"
#include "LandscapeEditor/LandscapeRenderer.h"

LandscapeProxy::LandscapeProxy(Landscape* landscape)
:	landscapeRenderer(0)
,	displayingTexture(0)
{
	baseLandscape = SafeRetain(landscape);
	for (int32 i = 0; i < TEXTURE_TYPES_COUNT; ++i)
	{
		texturesToBlend[i] = NULL;
		texturesEnabled[i] = false;
	}
}

LandscapeProxy::~LandscapeProxy()
{
	SafeRelease(baseLandscape);
	SafeRelease(displayingTexture);
}

void LandscapeProxy::SetRenderer(LandscapeRenderer *renderer)
{
	SafeRelease(landscapeRenderer);
	landscapeRenderer = SafeRetain(renderer);
}

LandscapeRenderer* LandscapeProxy::GetRenderer()
{
	return landscapeRenderer;
}

void LandscapeProxy::SetDisplayingTexture(DAVA::Texture *texture)
{
	SafeRelease(displayingTexture);
	displayingTexture = SafeRetain(texture);
}

void LandscapeProxy::Draw(DAVA::Camera *camera)
{
	if(!landscapeRenderer)
	{
		return;
	}
	
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, camera->GetMatrix());
	
	Texture* texture = displayingTexture;
	if (!displayingTexture)
	{
		texture = baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL);
	}
	
	landscapeRenderer->BindMaterial(texture);
	
	landscapeRenderer->DrawLandscape();
	
	if (cursor)
	{
		RenderManager::Instance()->AppendState(RenderState::STATE_BLEND);
		eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);
		cursor->Prepare();
        
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, landscapeRenderer->Indicies());
        
		RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		RenderManager::Instance()->RemoveState(RenderState::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(src, dst);
	}
	
	landscapeRenderer->UnbindMaterial();
}

AABBox3 LandscapeProxy::GetLandscapeBoundingBox()
{
	return baseLandscape->GetBoundingBox();
}

Texture* LandscapeProxy::GetLandscapeTexture(Landscape::eTextureLevel level)
{
	return baseLandscape->GetTexture(level);
}

void LandscapeProxy::SetTilemaskTexture(Texture* texture)
{
	//	SafeRelease(texturesToBlend[TEXTURE_TYPE_TILEMASK]);
	//	texturesToBlend[TEXTURE_TYPE_TILEMASK] = SafeRetain(texture);
	baseLandscape->SetTexture(TEXTURE_TILE_FULL, texture);
	baseLandscape->UpdateFullTiledTexture();
	
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetTilemaskTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_TILEMASK] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetNotPassableTexture(Texture* texture)
{
	SafeRelease(texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE]);
	texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE] = SafeRetain(texture);
	
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetNotPassableTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_NOT_PASSABLE] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetCustomColorsTexture(Texture* texture)
{
	SafeRelease(texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS]);
	texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS] = SafeRetain(texture);
	
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetCustomColorsTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_CUSTOM_COLORS] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetVisibilityCheckToolTexture(Texture* texture)
{
	SafeRelease(texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL]);
	texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL] = SafeRetain(texture);
	
	UpdateDisplayedTexture();
}

void LandscapeProxy::SetVisibilityCheckToolTextureEnabled(bool enabled)
{
	texturesEnabled[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL] = enabled;
	UpdateDisplayedTexture();
}

void LandscapeProxy::UpdateDisplayedTexture()
{
	RenderManager::Instance()->LockNonMain();
	
	Texture* tilemaskTexture = texturesToBlend[TEXTURE_TYPE_TILEMASK];
	if (!tilemaskTexture || !texturesEnabled[TEXTURE_TYPE_TILEMASK])
	{
		tilemaskTexture = baseLandscape->GetTexture(Landscape::TEXTURE_TILE_FULL);
	}
	
	int32 tilemaskWidth = tilemaskTexture->GetWidth();
	int32 tilemaskHeight = tilemaskTexture->GetHeight();
	Sprite* tilemaskSprite = Sprite::CreateFromTexture(tilemaskTexture, 0, 0, (float32)tilemaskWidth, (float32)tilemaskHeight);
	
	Sprite *dstSprite = Sprite::CreateAsRenderTarget((float32)tilemaskWidth, (float32)tilemaskHeight, FORMAT_RGBA8888);
	RenderManager::Instance()->SetRenderTarget(dstSprite);
	
	tilemaskSprite->SetPosition(0.f, 0.f);
	tilemaskSprite->Draw();
	SafeRelease(tilemaskSprite);
	
	eBlendMode srcBlend = RenderManager::Instance()->GetSrcBlend();
	eBlendMode dstBlend = RenderManager::Instance()->GetDestBlend();
	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	
	Texture* notPassableTexture = texturesToBlend[TEXTURE_TYPE_NOT_PASSABLE];
	Sprite* notPassableSprite = NULL;
	if (notPassableTexture && texturesEnabled[TEXTURE_TYPE_NOT_PASSABLE])
	{
		RenderManager::Instance()->SetColor(Color::White());
		notPassableSprite = Sprite::CreateFromTexture(notPassableTexture, 0, 0, (float32)tilemaskWidth, (float32)tilemaskHeight);
		notPassableSprite->SetPosition(0.f, 0.f);
		notPassableSprite->Draw();
	}
	SafeRelease(notPassableSprite);
	
	Texture* customColorsTexture = texturesToBlend[TEXTURE_TYPE_CUSTOM_COLORS];
	Sprite* customColorsSprite = NULL;
	if (customColorsTexture && texturesEnabled[TEXTURE_TYPE_CUSTOM_COLORS])
	{
		RenderManager::Instance()->SetColor(1.f, 1.f, 1.f, .5f);
		customColorsSprite = Sprite::CreateFromTexture(customColorsTexture, 0, 0, (float32)tilemaskWidth, (float32)tilemaskHeight);
		customColorsSprite->SetPosition(0.f, 0.f);
		customColorsSprite->Draw();
		RenderManager::Instance()->SetColor(Color::White());
	}
	SafeRelease(customColorsSprite);
	
	Texture* visibilityCheckToolTexture = texturesToBlend[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL];
	Sprite* visibilityCheckToolSprite = NULL;
	if (visibilityCheckToolTexture && texturesEnabled[TEXTURE_TYPE_VISIBILITY_CHECK_TOOL])
	{
		RenderManager::Instance()->SetColor(Color::White());
		visibilityCheckToolSprite = Sprite::CreateFromTexture(visibilityCheckToolTexture, 0, 0, (float32)tilemaskWidth, (float32)tilemaskHeight);
		visibilityCheckToolSprite->SetPosition(0.f, 0.f);
		visibilityCheckToolSprite->Draw();
	}
	SafeRelease(visibilityCheckToolSprite);
	
	RenderManager::Instance()->RestoreRenderTarget();
	
	SafeRelease(displayingTexture);
	displayingTexture = SafeRetain(dstSprite->GetTexture());
	
	SafeRelease(dstSprite);
	
	RenderManager::Instance()->SetBlendMode(srcBlend, dstBlend);
	
	RenderManager::Instance()->UnlockNonMain();
}
