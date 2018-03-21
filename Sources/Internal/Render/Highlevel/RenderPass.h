#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/QuadRenderer.h"
#include "Render/Texture.h"

namespace DAVA
{
class Camera;
class RenderLayer;

class RenderPass
{
public:
    RenderPass(const FastName& name);
    virtual ~RenderPass();

    void SetName(const FastName& name);
    const FastName& GetName() const;

    void AddRenderLayer(RenderLayer* layer, eRenderLayerID afterLayer = RENDER_LAYER_INVALID_ID);
    void RemoveRenderLayer(RenderLayer* layer);

    virtual void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF);

    uint32 GetRenderLayerCount() const;
    RenderLayer* GetRenderLayer(uint32 index) const;

    rhi::RenderPassConfig& GetPassConfig();
    void SetViewport(const Rect& viewPort);

    void SetRenderTargetProperties(uint32 width, uint32 height, PixelFormat format);
    void SetEnableFrameJittering(bool jitter);

    virtual void InvalidateMaterials();

protected:
    FastName passName;
    rhi::RenderPassConfig passConfig;
    Rect viewport;
    bool enableFrameJittering = false; // for TXAA

    Vector2 viewportSize, rcpViewportSize, viewportOffset; //storage fro dynamic bindings

    /*convinience*/
    void PrepareVisibilityArrays(Camera* camera, RenderSystem* renderSystem);
    void PrepareLayersArrays(const Vector<RenderObject*> objectsArray, Camera* camera);
    void ClearLayersArrays();

    void SetupCameraParams(Camera* mainCamera, Camera* drawCamera, Vector4* externalClipPlane = NULL);

    void DrawLayers(Camera* camera, uint32 drawLayersMask);
    void DrawDebug(Camera* camera, RenderSystem* renderSystem);

    bool BeginRenderPass(const rhi::RenderPassConfig& config);
    void EndRenderPass();

    void ValidateMultisampledTextures(const rhi::RenderPassConfig& forConfig);

    Vector2 GetCurrentFrameJitterOffset() const;

    Vector<RenderLayer*> renderLayers;
    std::array<RenderBatchArray, RENDER_LAYER_ID_COUNT> layersBatchArrays;
    RenderHierarchy::ClipResult visibilityArray;

    rhi::HPacketList packetList;
    rhi::HRenderPass renderPass;

    Texture::RenderTargetTextureKey multisampledDescription;
    Asset<Texture> multisampledTexture;
    Asset<Texture> multisampledDepthTexture;
    RenderTargetProperites renderTargetProperties;
    float currentDistantDepthValue = 1.0f;

    friend class RenderSystem;
};

inline rhi::RenderPassConfig& RenderPass::GetPassConfig()
{
    return passConfig;
}

inline void RenderPass::SetViewport(const Rect& _viewport)
{
    viewport = _viewport;
    passConfig.viewport.x = int32(viewport.x);
    passConfig.viewport.y = int32(viewport.y);
    passConfig.viewport.width = int32(viewport.dx);
    passConfig.viewport.height = int32(viewport.dy);
}

inline void RenderPass::SetName(const FastName& name)
{
    passName = name;
}

inline const FastName& RenderPass::GetName() const
{
    return passName;
}

inline uint32 RenderPass::GetRenderLayerCount() const
{
    return uint32(renderLayers.size());
}

inline RenderLayer* RenderPass::GetRenderLayer(uint32 index) const
{
    return renderLayers[index];
}

inline void RenderPass::InvalidateMaterials()
{
}

inline void RenderPass::SetEnableFrameJittering(bool jitter)
{
    enableFrameJittering = jitter;
}
}
