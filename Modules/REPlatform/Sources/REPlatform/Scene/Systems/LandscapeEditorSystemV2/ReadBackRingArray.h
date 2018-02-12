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
    ReadBackRingArray(const Texture::FBODescriptor& descriptor, uint32 initialSize);
    ~ReadBackRingArray();

    RefPtr<Texture> AcquireTexture(rhi::HSyncObject syncObject);

    Signal<RefPtr<Texture>> textureReady;

private:
    void AllocateTexture();
    struct TextureNode
    {
        Token callbackToken;
        rhi::HSyncObject syncObject;
        RefPtr<Texture> texture;
    };

    RefPtr<Texture> AcquireTexture(TextureNode& node, rhi::HSyncObject syncObject);

    Vector<TextureNode> nodes;
    Texture::FBODescriptor descriptor;
};
} // namespace DAVA
