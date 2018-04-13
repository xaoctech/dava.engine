#include "REPlatform/Commands/TilemaskEditorCommands.h"
#include "REPlatform/Scene/Systems/LandscapeProxy.h"
#include "REPlatform/Scene/SceneEditor2.h"

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

    Rect2i updatedRecti(int32(updatedRect.x), int32(updatedRect.y), int32(updatedRect.dx), int32(updatedRect.dy));
    for (uint32 i = 0; i < landscapeProxy->GetLayersCount(); ++i)
    {
        redoImageMask.push_back(landscapeProxy->GetLandscapeTexture(i, Landscape::TEXTURE_TILEMASK)->CreateImageFromRegion(updatedRecti));
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
    //GFX_COMPLETE layer index
    Texture* tilemask = landscapeProxy->GetTilemaskDrawTexture(0, LandscapeProxy::TILEMASK_TEXTURE_SOURCE);

    Rect rect = updatedRect;
    rect.x /= tilemask->GetWidth();
    rect.dx /= tilemask->GetWidth();
    rect.y /= tilemask->GetHeight();
    rect.dy /= tilemask->GetHeight();
    rect.y = 1.f - (rect.y + rect.dy);

    landscapeProxy->GetBaseLandscape()->InvalidatePages(rect);
}

void ModifyTilemaskCommand::ApplyImageToTexture(Image* image, Texture* dstTex)
{
    ScopedPtr<Texture> fboTexture(Texture::CreateFromData(image->GetPixelFormat(), image->GetData(), image->GetWidth(), image->GetHeight(), false));

    auto material = RenderSystem2D::DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL;

    RenderSystem2D::RenderTargetPassDescriptor desc;
    desc.priority = PRIORITY_SERVICE_2D;
    desc.colorAttachment = dstTex->handle;
    desc.depthAttachment = dstTex->handleDepthStencil;
    desc.width = dstTex->GetWidth();
    desc.height = dstTex->GetHeight();
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
