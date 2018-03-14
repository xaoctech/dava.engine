#pragma once

#include "Render/RHI/rhi_Public.h"
#include "Render/RHI/rhi_Type.h"
#include "Render/Highlevel/RenderHierarchy.h"
#include "Render/Shader/ShaderDescriptor.h"
#include "Asset/AssetListener.h"

namespace DAVA
{
class DeferredLightsRenderer
{
public:
    DeferredLightsRenderer(bool useFetch = false);
    ~DeferredLightsRenderer();
    void Draw(RenderHierarchy::ClipResult& visibilityArray, rhi::HPacketList packetList);

    void InvalidateMaterials();

private:
    void UpdatePacketParams();

    PolygonGroup* unityCube = nullptr;
    //lights
    rhi::Packet deferredLightsPacket;
    Asset<ShaderDescriptor> deferredLightsShader = nullptr;
    SimpleAssetListener listener;

    bool initTextureSet = true; //used to init texture set once after first use - to prevent allocating gbuffers if not required
    bool useFetch = false;
};
}
