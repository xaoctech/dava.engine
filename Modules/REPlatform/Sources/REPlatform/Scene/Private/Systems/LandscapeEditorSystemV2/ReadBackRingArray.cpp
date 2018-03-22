#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/ReadBackRingArray.h"

#include <Asset/AssetManager.h>
#include <Engine/Engine.h>
#include <Render/Renderer.h>

namespace DAVA
{
ReadBackRingArray::ReadBackRingArray(const Texture::RenderTargetTextureKey& descriptor_, uint32 initialSize)
    : descriptor(descriptor_)
{
    for (uint32 i = 0; i < initialSize; ++i)
    {
        AllocateTexture();
    }
}

ReadBackRingArray::~ReadBackRingArray()
{
    for (const TextureNode& node : nodes)
    {
        if (node.callbackToken.IsEmpty() == false)
        {
            Renderer::UnRegisterSyncCallback(node.callbackToken);
        }
    }

    nodes.clear();
}

Asset<Texture> ReadBackRingArray::AcquireTexture(rhi::HSyncObject syncObject)
{
    for (TextureNode& node : nodes)
    {
        if (node.callbackToken.IsEmpty())
        {
            return AcquireTexture(node, syncObject);
        }
    }

    AllocateTexture();
    return AcquireTexture(nodes.back(), syncObject);
}

Asset<Texture> ReadBackRingArray::AcquireTexture(TextureNode& node, rhi::HSyncObject syncObject)
{
    DVASSERT(node.callbackToken.IsEmpty() == true);
    DVASSERT(node.syncObject.IsValid() == false);
    DVASSERT(node.texture != nullptr);
    node.syncObject = syncObject;
    node.callbackToken = Renderer::RegisterSyncCallback(syncObject, [this](rhi::HSyncObject syncObject) {
        for (TextureNode& node : nodes)
        {
            if (node.syncObject == syncObject)
            {
                textureReady.Emit(node.texture);
                node.callbackToken.Clear();
                node.syncObject = rhi::HSyncObject();
            }
        }
    });

    return node.texture;
}

void ReadBackRingArray::AllocateTexture()
{
    TextureNode node;
    node.texture = GetEngineContext()->assetManager->GetAsset<Texture>(descriptor, AssetManager::SYNC);
    node.texture->SetMinMagFilter(rhi::TEXFILTER_NEAREST, rhi::TEXFILTER_NEAREST, rhi::TEXMIPFILTER_NONE);
    node.texture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    nodes.push_back(node);
}
} // namespace DAVA
