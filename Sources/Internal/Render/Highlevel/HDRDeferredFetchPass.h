#pragma once

#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/DeferredDecalRenderer.h"
#include "Render/Highlevel/DeferredLightsRenderer.h"
#include "Render/Material/NMaterial.h"
#include "Render/Texture.h"

namespace DAVA
{
/*Draw opaque surface params to g-buffer -> draw decals to g-buffer-> resolve  sun light -> add spot lights -> draw translusent forward stuff*/
//GFX_COMPLETE - for now at least it looks like mega-passes are ok, just move rendering of similar stuff to "Renderers"
class HDRDeferredFetchPass : public RenderPass
{
public:
    HDRDeferredFetchPass();
    ~HDRDeferredFetchPass();
    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;

    void InvalidateMaterials() override;

    void PreparePassConfig(rhi::HTexture colorTarget);

private:
    void UpdateScreenResolveData(RenderSystem* renderSystem);

private:
    Asset<Texture> defaultCubemap;
    DeferredLightsRenderer* deferredLightsRenderer = nullptr;
    DeferredDecalRenderer* deferredDecalRenderer = nullptr;

    //resolve
    NMaterial* screenResolveMaterial = nullptr;
    rhi::Packet screenResolvePacket;
    rhi::HVertexBuffer quadBuffer;

    rhi::RenderPassConfig deferredPassConfig;
    rhi::RenderPassConfig forwardStuffPassConfig;

    bool gbuffersInited = false;
};
}
