#include "REPlatform/Commands/TilemaskEditorCommands.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Render/2D/Systems/RenderSystem2D.h>
#include <Render/Highlevel/Landscape.h>
#include <Render/Image/Image.h>

namespace DAVA
{
ModifyTilemaskCommand::ModifyTilemaskCommand(LandscapeProxy* landscapeProxy_, const Rect& updatedRect_)
    : RECommand("Tile Mask Modification")
    , landscapeProxy(SafeRetain(landscapeProxy_))
{
    updatedRect = Rect(std::floor(updatedRect_.x), std::floor(updatedRect_.y), std::ceil(updatedRect_.dx), std::ceil(updatedRect_.dy));
    undoImageMask.reserve(4);
    redoImageMask.reserve(4);

    for (uint32 i = 0; i < landscapeProxy->GetLayersCount(); ++i)
    {
        ScopedPtr<Image> currentImageMask(landscapeProxy->GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK)->CreateImageFromRegion());
        redoImageMask.push_back(Image::CopyImageRegion(currentImageMask, updatedRect));

        undoImageMask.push_back(Image::CopyImageRegion(landscapeProxy->GetTilemaskImageCopy(i), updatedRect));
    }
}

ModifyTilemaskCommand::~ModifyTilemaskCommand()
{
    for (uint32 i = 0; i < undoImageMask.size(); ++i)
    {
        SafeRelease(undoImageMask[i]);
        SafeRelease(redoImageMask[i]);
    }
    SafeRelease(landscapeProxy);
}

void ModifyTilemaskCommand::Undo()
{
    for (uint32 i = 0; i < landscapeProxy->GetLayersCount(); ++i)
    {
        ApplyImageToTexture(undoImageMask[i], landscapeProxy->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_SOURCE));
        ApplyImageToTexture(undoImageMask[i], landscapeProxy->GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK));

        Rect r = Rect(Vector2(0, 0), Vector2(undoImageMask[i]->GetWidth(), undoImageMask[i]->GetHeight()));
        Image* mask = landscapeProxy->GetTilemaskImageCopy(i);
        mask->InsertImage(undoImageMask[i], updatedRect.GetPosition(), r);
    }

    landscapeProxy->DecreaseTilemaskChanges();
    InvalidateLandscapePart();
}

void ModifyTilemaskCommand::Redo()
{
    for (uint32 i = 0; i < landscapeProxy->GetLayersCount(); ++i)
    {
        ApplyImageToTexture(redoImageMask[i], landscapeProxy->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_SOURCE));
        ApplyImageToTexture(redoImageMask[i], landscapeProxy->GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK));

        Rect r = Rect(Vector2(0, 0), Vector2(redoImageMask[i]->GetWidth(), redoImageMask[i]->GetHeight()));
        Image* mask = landscapeProxy->GetTilemaskImageCopy(i);
        mask->InsertImage(redoImageMask[i], updatedRect.GetPosition(), r);
    }

    landscapeProxy->IncreaseTilemaskChanges();
    InvalidateLandscapePart();
}

void ModifyTilemaskCommand::InvalidateLandscapePart()
{
    Rect fullRect;
    for (uint32 i = 0; i < landscapeProxy->GetLayersCount(); ++i)
    {
        Asset<Texture> tilemask = landscapeProxy->GetTilemaskDrawTexture(i, LandscapeProxy::TILEMASK_TEXTURE_SOURCE);
        Rect rect = updatedRect;
        rect.x /= tilemask->width;
        rect.dx /= tilemask->width;
        rect.y /= tilemask->height;
        rect.dy /= tilemask->height;
        rect.y = 1.f - (rect.y + rect.dy);
        fullRect.Combine(rect);
    }
    landscapeProxy->GetBaseLandscape()->InvalidatePages(fullRect);
}

void ModifyTilemaskCommand::ApplyImageToTexture(Image* image, const Asset<Texture>& dstTex)
{
    Texture::UniqueTextureKey key(RefPtr<Image>::ConstructWithRetain(image), false);
    Asset<Texture> fboTexture = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);

    auto material = RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL;

    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.priority = PRIORITY_SERVICE_2D;
    desc.colorAttachment = dstTex->handle;
    desc.depthAttachment = rhi::HTexture();
    desc.width = dstTex->width;
    desc.height = dstTex->height;
    desc.clearTarget = false;
    desc.transformVirtualToPhysical = false;
    RenderSystem2D::Instance()->BeginRenderTargetPass(desc);
    RenderSystem2D::Instance()->DrawTexture(fboTexture, material, Color::White, updatedRect);
    RenderSystem2D::Instance()->EndRenderTargetPass();
}

DAVA_VIRTUAL_REFLECTION_IMPL(ModifyTilemaskCommand)
{
    ReflectionRegistrator<ModifyTilemaskCommand>::Begin()
    .End();
}
} // namespace DAVA
