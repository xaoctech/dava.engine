#pragma once

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/Texture.h"

namespace rhi
{
struct Packet;
}

namespace DAVA
{
class NMaterial;
class VirtualTexture
{
public:
    struct Descriptor
    {
        Vector<PixelFormat> virtualTextureLayers;
        Vector<PixelFormat> intermediateBuffers;

        uint32 width = 0;
        uint32 height = 0;
        uint32 pageSize = 0;
        uint32 mipLevelCount = 0;
    };

    struct PageInfo
    {
        uint32 offsetX = 0;
        uint32 offsetY = 0;
        bool isFree = true;
    };

    VirtualTexture(const Descriptor& descriptor);
    ~VirtualTexture();

    Asset<Texture> GetLayerTexture(uint32 layer) const;
    Asset<Texture> GetIntermediateSourceBuffer(uint32 intermediateBufferLayer) const;
    Asset<Texture> GetIntermediateDestinationBuffer(uint32 intermediateBufferLayer) const;

    Vector<Asset<Texture>> GetIntermediateSourceBuffers() const;
    Vector<Asset<Texture>> GetIntermediateDestinationBuffers() const;

    void SwapIntermediateBuffers();
    void BlitIntermediateBuffer(int32 pageID);

    uint32 GetFreePagesCount() const;

    int32 AcquireFreePage();
    void FreePage(int32 pageID);
    void FreePages();

    const PageInfo& GetPageInfo(int32 pageID) const;

    uint32 GetWidth() const;
    uint32 GetHeight() const;
    uint32 GetPageSize() const;
    uint32 GetLayersCount() const;
    uint32 GetIntermediateBufffersLayersCount() const;
    uint32 GetMipLevelCount() const;

protected:
    struct
    {
        Vector<Asset<Texture>> src;
        Vector<Asset<Texture>> dst;
        uint32 size;
    } intermediateBuffers;

    NMaterial* blitMaterial = nullptr;
    rhi::Packet blitPacket;

    Vector<Asset<Texture>> virtualTextureLayers;

    Vector<PageInfo> pages;
    Vector<int32> availablePages;

    uint32 width = 0;
    uint32 height = 0;
    uint32 pageSize = 0;
    uint32 pageCount = 0;
    uint32 mipLevelCount = 0;
};

inline uint32 VirtualTexture::GetWidth() const
{
    return width;
}

inline uint32 VirtualTexture::GetHeight() const
{
    return height;
}

inline uint32 VirtualTexture::GetPageSize() const
{
    return pageSize;
}

inline uint32 VirtualTexture::GetLayersCount() const
{
    return uint32(virtualTextureLayers.size());
}

inline uint32 VirtualTexture::GetIntermediateBufffersLayersCount() const
{
    return intermediateBuffers.size;
}

inline uint32 VirtualTexture::GetMipLevelCount() const
{
    return mipLevelCount;
}

}; //ns DAVA