#pragma once

#include "Render/Highlevel/RenderPass.h"

namespace DAVA
{
/*Draw opaque surface params to g-buffer -> draw decals to g-buffer-> resolve  sun light -> add spot lights -> draw translusent forward stuff*/
class HDRDeferredPass : public RenderPass
{
public:
    HDRDeferredPass();
    ~HDRDeferredPass();

    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;

    void InvalidateMaterials() override;

private:
    class GBufferResolvePass;
    class DeferredDecalPass;
    class GBufferPass;

    GBufferPass* gBufferPass = nullptr;
    DeferredDecalPass* deferredDecalPass = nullptr;
    GBufferResolvePass* gBufferResolvePass = nullptr;

    void DebugDumpGBuffers();
};
}
