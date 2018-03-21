#include "Render/VirtualTexture.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/Material/NMaterial.h"
#include "Render/Material/NMaterialNames.h"
#include "Render/RHI/rhi_Public.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Utils/Random.h"
#include "Utils/StringFormat.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"

#define VIRTUAL_TEXTURE_USE_BLITAPI 0

namespace DAVA
{
VirtualTexture::VirtualTexture(const Descriptor& descriptor)
    : width(descriptor.width)
    , height(descriptor.height)
    , pageSize(descriptor.pageSize)
    , mipLevelCount(descriptor.mipLevelCount)
{
    DVASSERT(IsPowerOf2(width) && IsPowerOf2(height));
    DVASSERT((width % pageSize) == 0 && (height % pageSize) == 0);
    DVASSERT(descriptor.intermediateBuffers.size() >= descriptor.virtualTextureLayers.size());

    Texture::RenderTargetTextureKey textureConfig;
    textureConfig.width = width;
    textureConfig.height = height;
    textureConfig.sampleCount = 1;
    textureConfig.mipLevelsCount = mipLevelCount;
    textureConfig.textureType = rhi::TEXTURE_TYPE_2D;
    textureConfig.isDepth = false;
    textureConfig.needPixelReadback = false;
    textureConfig.ensurePowerOf2 = false;

    Texture::RenderTargetTextureKey pageConfig;
    pageConfig.width = pageSize;
    pageConfig.height = pageSize;
    pageConfig.sampleCount = 1;
    pageConfig.textureType = rhi::TEXTURE_TYPE_2D;
    pageConfig.isDepth = false;
    pageConfig.needPixelReadback = false;
    pageConfig.ensurePowerOf2 = false;

    virtualTextureLayers.resize(descriptor.virtualTextureLayers.size());

    intermediateBuffers.size = uint32(descriptor.intermediateBuffers.size());
    intermediateBuffers.src.resize(intermediateBuffers.size);
    intermediateBuffers.dst.resize(intermediateBuffers.size);

    uint32 layersCount = uint32(virtualTextureLayers.size());
    AssetManager* assetManager = GetEngineContext()->assetManager;
    for (uint32 i = 0; i < layersCount; ++i)
    {
        textureConfig.format = descriptor.virtualTextureLayers[i];
        virtualTextureLayers[i] = assetManager->GetAsset<Texture>(textureConfig, AssetManager::SYNC);
    }
    for (uint32 i = 0; i < intermediateBuffers.size; ++i)
    {
        pageConfig.format = descriptor.intermediateBuffers[i];
        intermediateBuffers.src[i] = assetManager->GetAsset<Texture>(pageConfig, AssetManager::SYNC);
        intermediateBuffers.dst[i] = assetManager->GetAsset<Texture>(pageConfig, AssetManager::SYNC);
    }

    pageCount = width / pageSize;
    pageCount *= height / pageSize;

    pages.resize(pageCount);
    availablePages.reserve(pageCount);

    uint32 pagesInRow = width / pageSize;
    uint32 pagesInCol = height / pageSize;
    for (uint32 py = 0; py < pagesInCol; ++py)
    {
        for (uint32 px = 0; px < pagesInRow; ++px)
        {
            uint32 pageIdx = (py * pagesInRow + px);
            PageInfo& page = pages[pageIdx];
            page.offsetX = px * pageSize;
            page.offsetY = py * pageSize;

            availablePages.push_back(pageIdx);
        }
    }

//////////////////////////////////////////////////////////////////////////

#if !VIRTUAL_TEXTURE_USE_BLITAPI
    blitPacket.vertexStreamCount = 1;
    blitPacket.vertexCount = 4;
    blitPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    blitPacket.primitiveCount = 2;
    blitPacket.options |= rhi::Packet::OPT_OVERRIDE_SCISSOR;

    rhi::VertexLayout blitQuadLayout;
    blitQuadLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    blitPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(blitQuadLayout);

    blitMaterial = new NMaterial();
    blitMaterial->SetFXName(NMaterialName::TEXTURE_BLIT);

    blitMaterial->AddFlag(NMaterialFlagName::FLAG_TEXTURE_COUNT, layersCount);
#endif
}

VirtualTexture::~VirtualTexture()
{
    virtualTextureLayers.clear();
    intermediateBuffers.src.clear();
    intermediateBuffers.dst.clear();

    SafeRelease(blitMaterial);
}

Asset<Texture> VirtualTexture::GetLayerTexture(uint32 layer) const
{
    DVASSERT(std::size_t(layer) < virtualTextureLayers.size());
    return virtualTextureLayers[layer];
}

Asset<Texture> VirtualTexture::GetIntermediateSourceBuffer(uint32 intermediateBufferLayer) const
{
    DVASSERT(std::size_t(intermediateBufferLayer) < intermediateBuffers.size);
    return intermediateBuffers.src[intermediateBufferLayer];
}

Asset<Texture> VirtualTexture::GetIntermediateDestinationBuffer(uint32 intermediateBufferLayer) const
{
    DVASSERT(std::size_t(intermediateBufferLayer) < intermediateBuffers.size);
    return intermediateBuffers.dst[intermediateBufferLayer];
}

Vector<Asset<Texture>> VirtualTexture::GetIntermediateSourceBuffers() const
{
    return intermediateBuffers.src;
}

Vector<Asset<Texture>> VirtualTexture::GetIntermediateDestinationBuffers() const
{
    return intermediateBuffers.dst;
}

void VirtualTexture::SwapIntermediateBuffers()
{
    intermediateBuffers.src.swap(intermediateBuffers.dst);
}

uint32 VirtualTexture::GetFreePagesCount() const
{
    return uint32(availablePages.size());
}

int32 VirtualTexture::AcquireFreePage()
{
    int32 pageID = -1;
    if (!availablePages.empty())
    {
        pageID = availablePages.back();
        availablePages.pop_back();
        pages[pageID].isFree = false;
    }
    return pageID;
}

void VirtualTexture::FreePage(int32 pageID)
{
    DVASSERT(std::size_t(pageID) < pages.size());

    if (!pages[pageID].isFree)
    {
        pages[pageID].isFree = true;
        availablePages.push_back(pageID);
    }
}

void VirtualTexture::FreePages()
{
    for (uint32 i = 0; i < pageCount; ++i)
    {
        FreePage(int32(i));
    }
}

void VirtualTexture::BlitIntermediateBuffer(int32 pageID)
{
    DVASSERT(std::size_t(pageID) < pages.size());
    DVASSERT(!pages[pageID].isFree);
    DVASSERT(virtualTextureLayers.size() <= rhi::MAX_RENDER_TARGET_COUNT);


#if VIRTUAL_TEXTURE_USE_BLITAPI
    DVASSERT(false); //TODO: optional blit-api
#else
    uint32 layersCount = uint32(virtualTextureLayers.size());
    // If we have more intermediate layers then virtual texture layers we blit only first virtualTextureLayers.size() buffer layers to texture.
    for (uint32 i = 0; i < layersCount; i++)
        Renderer::GetDynamicBindings().SetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_SRC_0 + i), intermediateBuffers.src[i]->handle);

    if (blitMaterial->PreBuildMaterial(PASS_FORWARD))
        blitMaterial->BindParams(blitPacket);
    else
        return;

    DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(Vector3), 4);
    blitPacket.vertexStream[0] = vb.buffer;
    blitPacket.baseVertex = vb.baseVertex;
    blitPacket.vertexCount = vb.allocatedVertices;
    blitPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);

    Vector3* blitVBData = reinterpret_cast<Vector3*>(vb.data);

    const PageInfo& page = pages[pageID];
    for (uint32 mip = 0; mip < mipLevelCount; ++mip)
    {
        rhi::RenderPassConfig passConfig;
        passConfig.name = "BlitPageToTexture";
        passConfig.priority = PRIORITY_SERVICE_3D + 20;
        for (std::size_t i = 0; i < virtualTextureLayers.size(); ++i)
        {
            passConfig.colorBuffer[i].texture = virtualTextureLayers[i]->handle;
            passConfig.colorBuffer[i].loadAction = rhi::LOADACTION_LOAD;
            passConfig.colorBuffer[i].storeAction = rhi::STOREACTION_STORE;
            passConfig.colorBuffer[i].textureLevel = mip;
        }
        passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
        passConfig.viewport.x = (page.offsetX >> mip);
        passConfig.viewport.y = (page.offsetY >> mip);
        passConfig.viewport.width = (pageSize >> mip);
        passConfig.viewport.height = (pageSize >> mip);
        DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::LANDSCAPE_PAGE_UPDATE);

        blitPacket.scissorRect.x = uint16(passConfig.viewport.x);
        blitPacket.scissorRect.y = uint16(passConfig.viewport.y);
        blitPacket.scissorRect.width = uint16(passConfig.viewport.width);
        blitPacket.scissorRect.height = uint16(passConfig.viewport.height);

        // float fPageSize = static_cast<float>(pageSize >> mip);
        Vector3 offset(0.0f, 0.0f, 0.0f); // GFX_COMPLETE
        // Deal with half-pixel offset on DX9
        // = rhi::DeviceCaps().isCenterPixelMapping ? Vector3(1.0f / fPageSize, 1.0f / fPageSize, 0.0f) : Vector3(0.0f, 0.0f, 0.0f);

        blitVBData[0] = Vector3(-1.f, -1.f, .0f) + offset;
        blitVBData[1] = Vector3(-1.f, 1.f, .0f) + offset;
        blitVBData[2] = Vector3(1.f, -1.f, .0f) + offset;
        blitVBData[3] = Vector3(1.f, 1.f, .0f) + offset;

        rhi::HPacketList packetList;
        rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);

        rhi::BeginRenderPass(pass);
        rhi::BeginPacketList(packetList);

        rhi::AddPackets(packetList, &blitPacket, 1);

        rhi::EndPacketList(packetList);
        rhi::EndRenderPass(pass);
    }
#endif
}

const VirtualTexture::PageInfo& VirtualTexture::GetPageInfo(int32 pageID) const
{
    DVASSERT(std::size_t(pageID) < pages.size());

    return pages[pageID];
}
}
