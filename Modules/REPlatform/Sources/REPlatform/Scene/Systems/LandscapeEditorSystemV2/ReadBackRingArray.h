#pragma once

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <Render/Texture.h>
#include <Render/RHI/rhi_Public.h>
#include <Functional/Signal.h>

namespace DAVA
{
class ReadBackRingArray final
{
public:
    ReadBackRingArray(const Texture::RenderTargetTextureKey& descriptor, uint32 initialSize);
    ~ReadBackRingArray();

    Asset<Texture> AcquireTexture(rhi::HSyncObject syncObject);

    Signal<const Asset<Texture>&> textureReady;

private:
    void AllocateTexture();
    struct TextureNode
    {
        Token callbackToken;
        rhi::HSyncObject syncObject;
        Asset<Texture> texture;
    };

    Asset<Texture> AcquireTexture(TextureNode& node, rhi::HSyncObject syncObject);

    Vector<TextureNode> nodes;
    Texture::RenderTargetTextureKey descriptor;
};
} // namespace DAVA
