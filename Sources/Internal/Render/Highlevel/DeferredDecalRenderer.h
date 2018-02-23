#pragma once

#include "Render/RHI/rhi_Public.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/Highlevel/RenderHierarchy.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{
class DeferredDecalRenderer
{
public:
    DeferredDecalRenderer(bool useFetch = false);
    ~DeferredDecalRenderer();

    /*return gBufferMask*/
    uint32 Draw(RenderHierarchy::ClipResult& visibilityArray, rhi::HPacketList packetList);
    enum GbufferCopyMask
    {
        GBUFFER_COPY_0 = 0,
        GBUFFER_COPY_1 = 1,
        GBUFFER_COPY_2 = 2,
        GBUFFER_COPY_3 = 3
    };

private:
    struct DecalGroup
    {
        uint32 start = 0;
        uint32 next = 0;
        NMaterial* material;

        DecalGroup(uint32 _start, uint32 _next, NMaterial* _material)
            : start(_start)
            , next(_next)
            , material(_material)
        {
        }
    };
    Vector<DecalGroup> visibleDecalGroups;

    PolygonGroup* unityCube = nullptr;
    //decals
    rhi::Packet deferredDecalsPacket;
    bool useFetch;
};
}
