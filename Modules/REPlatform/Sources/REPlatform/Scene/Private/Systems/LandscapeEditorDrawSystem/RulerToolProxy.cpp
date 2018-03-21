#include "REPlatform/Scene/Private/Systems/LandscapeEditorDrawSystem/RulerToolProxy.h"

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <Render/RenderBase.h>
#include <Render/RenderHelper.h>
#include <Render/RHI/rhi_Type.h>
#include <Render/Texture.h>

namespace DAVA
{
RulerToolProxy::RulerToolProxy(int32 size)
    : size(size)
    , spriteChanged(false)
{
    uint32 unsignedSize = static_cast<uint32>(size);

    Texture::RenderTargetTextureKey key;
    key.width = unsignedSize;
    key.height = unsignedSize;
    key.format = FORMAT_RGBA8888;
    rulerToolTexture = GetEngineContext()->assetManager->GetAsset<Texture>(key, AssetManager::SYNC);

    rhi::Viewport viewport;
    viewport.x = viewport.y = 0U;
    viewport.width = unsignedSize;
    viewport.height = unsignedSize;
    RenderHelper::CreateClearPass(rulerToolTexture->handle, rhi::HTexture(), PRIORITY_CLEAR, Color(0.f, 0.f, 0.f, 0.f), viewport);
}

RulerToolProxy::~RulerToolProxy()
{
}

int32 RulerToolProxy::GetSize()
{
    return size;
}

const Asset<Texture>& RulerToolProxy::GetTexture() const
{
    return rulerToolTexture;
}
} // namespace DAVA
