#include "HDRDeferredPass.h"
#include "Render/ServiceTextures.h"
#include "Render/Highlevel/PostEffectRenderer.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "FileSystem/FileSystem.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Render/3D/MeshUtils.h"
#include "Render/Highlevel/Light.h"
#include "Render/DynamicBufferAllocator.h"
#include "Render/Highlevel/DecalRenderObject.h"
#include "Render/Highlevel/DeferredDecalRenderer.h"
#include "Render/Highlevel/DeferredLightsRenderer.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/RhiUtils.h"
#include "Render/Highlevel/VelocityPass.h"
#include "Render/Shader/ShaderAssetLoader.h"

//for debug dump gbuffers
#include "Logger/Logger.h"
#include "Render/Image/Image.h"

namespace DAVA
{
class HDRDeferredPass::GBufferPass : public RenderPass
{
public:
    GBufferPass();
    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;

private:
    ScopedPtr<Texture> defaultCubemap;
};

class HDRDeferredPass::DeferredDecalPass : public RenderPass
{
public:
    DeferredDecalPass();
    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;

private:
    DeferredDecalRenderer* deferredDecalRenderer = nullptr;

    NMaterial* gbufferCopyMaterial;
    rhi::Packet gbufferCopyPacket;
    rhi::HVertexBuffer quadBuffer;
};

class HDRDeferredPass::GBufferResolvePass : public RenderPass
{
public:
    GBufferResolvePass();
    ~GBufferResolvePass();
    void Draw(RenderSystem* renderSystem, uint32 drawLayersMask = 0xFFFFFFFF) override;

    void InvalidateMaterials() override;

private:
    //screen resolve
    NMaterial* screenResolveMaterial;
    rhi::Packet screenResolvePacket;
    rhi::HVertexBuffer quadBuffer;

    DeferredLightsRenderer* deferredLightsRenderer = nullptr;
};

HDRDeferredPass::GBufferPass::GBufferPass()
    : RenderPass(PASS_GBUFFER)
    , defaultCubemap(Texture::CreatePink(rhi::TextureType::TEXTURE_TYPE_CUBE))
{
    AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));

    Renderer::GetDynamicBindings().SetDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_GLOBAL_REFLECTION, defaultCubemap->handle);
    Renderer::GetDynamicBindings().SetDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_LOCAL_REFLECTION, defaultCubemap->handle);
}

void HDRDeferredPass::GBufferPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::GBUFFER_PASS);
    RenderPass::Draw(renderSystem, drawLayersMask);
}

HDRDeferredPass::DeferredDecalPass::DeferredDecalPass()
    : RenderPass(PASS_GBUFFER)
{
    //AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
    //AddRenderLayer(new RenderLayer(RENDER_LAYER_DEFERRED_DECALS_ID, 0));

    deferredDecalRenderer = new DeferredDecalRenderer();

    Array<Vector3, 6> quad = {
        Vector3(-1.f, -1.f, 1.f), Vector3(-1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f),
        Vector3(-1.f, 1.f, 1.f), Vector3(1.f, 1.f, 1.f), Vector3(1.f, -1.f, 1.f)
    };
    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(Vector3) * 6;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    quadBuffer = rhi::CreateVertexBuffer(vDesc);

    gbufferCopyPacket.vertexStreamCount = 1;
    gbufferCopyPacket.vertexStream[0] = quadBuffer;
    gbufferCopyPacket.vertexCount = 4;
    gbufferCopyPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    gbufferCopyPacket.primitiveCount = 2;

    rhi::VertexLayout gbufferCopyLayout;
    gbufferCopyLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    gbufferCopyPacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(gbufferCopyLayout);

    gbufferCopyMaterial = new NMaterial();
    gbufferCopyMaterial->SetFXName(NMaterialName::GBUFFER_COPY);
    gbufferCopyMaterial->AddFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_0, 0);
    gbufferCopyMaterial->AddFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_1, 0);
    gbufferCopyMaterial->AddFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_2, 0);
    gbufferCopyMaterial->AddFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_3, 0);
}

void HDRDeferredPass::DeferredDecalPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DAVA_PROFILER_GPU_RENDER_PASS(passConfig, ProfilerGPUMarkerName::DEFERRED_DECALS_PASS);
    //RenderPass::Draw(renderSystem, drawLayersMask);

    //GFX_COMPLETE - GBufferPass and DeferredDecalPass actually share camera params and visibility arrays - so we can do clipping only once
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);
    uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::ENABLE_STATIC_OCCLUSION))
        currVisibilityCriteria &= ~RenderObject::VISIBLE_STATIC_OCCLUSION;
    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(mainCamera, visibilityArray, currVisibilityCriteria);

    if (visibilityArray.decalArray.empty())
        return;

    //per pass viewport bindings
    viewportSize = Vector2(viewport.dx, viewport.dy);
    rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
    viewportOffset = Vector2(viewport.x, viewport.y);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));

    //GFX_COMPLETE consider using framebuffer fetch?
    //GFX_COMPLETE looks like some custom decal shaders cant be done via framebuffer fetch, as they access neighbor pixels (eg: DTE:ripples) ...
    //GFX_COMPLETE copy required gbuffers - for now all are copied

    uint32 gbuffersMask = 0;
    if (BeginRenderPass(passConfig))
    {
        gbuffersMask = deferredDecalRenderer->Draw(visibilityArray, packetList);

        EndRenderPass();
    }

    if (gbuffersMask != 0)
    {
        //copy gbuffers
        rhi::RenderPassConfig copyPassConfig;
        copyPassConfig.priority = passConfig.priority + 1;
        copyPassConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
        copyPassConfig.viewport = passConfig.viewport;
        DAVA_PROFILER_GPU_RENDER_PASS(copyPassConfig, ProfilerGPUMarkerName::COPY_GBUFFERS_PASS);

        gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_0, 0);
        gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_1, 0);
        gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_2, 0);
        gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_3, 0);

        if (gbuffersMask & (1 << DeferredDecalRenderer::GBUFFER_COPY_0))
        {
            copyPassConfig.colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_0_COPY);
            copyPassConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
            copyPassConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
            copyPassConfig.colorBuffer[0].clearColor[0] = 0;
            copyPassConfig.colorBuffer[0].clearColor[1] = 0;
            copyPassConfig.colorBuffer[0].clearColor[2] = 1;
            copyPassConfig.colorBuffer[0].clearColor[3] = 1;

            gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_0, 1);
        }

        if (gbuffersMask & (1 << DeferredDecalRenderer::GBUFFER_COPY_1))
        {
            copyPassConfig.colorBuffer[1].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_1_COPY);
            copyPassConfig.colorBuffer[1].loadAction = rhi::LOADACTION_NONE;
            copyPassConfig.colorBuffer[1].storeAction = rhi::STOREACTION_STORE;
            gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_1, 1);
        }

        if (gbuffersMask & (1 << DeferredDecalRenderer::GBUFFER_COPY_2))
        {
            copyPassConfig.colorBuffer[2].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_2_COPY);
            copyPassConfig.colorBuffer[2].loadAction = rhi::LOADACTION_NONE;
            copyPassConfig.colorBuffer[2].storeAction = rhi::STOREACTION_STORE;
            gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_2, 1);
        }
        if (gbuffersMask & (1 << DeferredDecalRenderer::GBUFFER_COPY_3))
        {
            copyPassConfig.colorBuffer[3].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3_COPY);
            copyPassConfig.colorBuffer[3].loadAction = rhi::LOADACTION_NONE;
            copyPassConfig.colorBuffer[3].storeAction = rhi::STOREACTION_STORE;
            gbufferCopyMaterial->SetFlag(NMaterialFlagName::FLAG_COPY_GBUFFER_3, 1);
        }

        if (gbufferCopyMaterial->PreBuildMaterial(PASS_GBUFFER))
        {
            gbufferCopyMaterial->BindParams(gbufferCopyPacket);

            rhi::HPacketList packetList;
            rhi::HRenderPass pass = rhi::AllocateRenderPass(copyPassConfig, 1, &packetList);

            rhi::BeginRenderPass(pass);
            rhi::BeginPacketList(packetList);

            rhi::AddPacket(packetList, gbufferCopyPacket);

            rhi::EndPacketList(packetList);
            rhi::EndRenderPass(pass);
        }
    }
}

HDRDeferredPass::GBufferResolvePass::GBufferResolvePass()
    : RenderPass(PASS_GBUFFER_RESOLVE)
{
    /*AddRenderLayer(new RenderLayer(RENDER_LAYER_RESOLVE_GBUFFER_ID, 0));*/
    AddRenderLayer(new RenderLayer(RENDER_LAYER_DEFERRED_LIGHTS_ID, 0));

    const static uint32 VERTEX_COUNT = 6;

    struct ResVertex
    {
        Vector3 pos;
        Vector2 uv;
    };

    std::array<ResVertex, VERTEX_COUNT> quad =
    {
      Vector3(-1.f, -1.f, 1.f), Vector2(0.f, 0.f), Vector3(-1.f, 1.f, 1.f), Vector2(0.f, 1.f), Vector3(1.f, -1.f, 1.f), Vector2(1.f, 0.f),
      Vector3(-1.f, 1.f, 1.f), Vector2(0.f, 1.f), Vector3(1.f, 1.f, 1.f), Vector2(1.f, 1.f), Vector3(1.f, -1.f, 1.f), Vector2(1.f, 0.f)
    };

    rhi::VertexBuffer::Descriptor vDesc;
    vDesc.size = sizeof(ResVertex) * VERTEX_COUNT;
    vDesc.initialData = quad.data();
    vDesc.usage = rhi::USAGE_STATICDRAW;
    quadBuffer = rhi::CreateVertexBuffer(vDesc);

    rhi::VertexLayout vxLayout;
    vxLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    vxLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    screenResolvePacket.vertexLayoutUID = rhi::VertexLayout::UniqueId(vxLayout);

    screenResolvePacket.vertexStreamCount = 1;
    screenResolvePacket.vertexStream[0] = quadBuffer;
    screenResolvePacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    screenResolvePacket.primitiveCount = 2;

    screenResolveMaterial = new NMaterial();
    screenResolveMaterial->SetFXName(NMaterialName::GBUFFER_RESOLVE);

    deferredLightsRenderer = new DeferredLightsRenderer();
}

HDRDeferredPass::GBufferResolvePass::~GBufferResolvePass()
{
    SafeDelete(deferredLightsRenderer);
    SafeRelease(screenResolveMaterial);
}

void HDRDeferredPass::GBufferResolvePass::InvalidateMaterials()
{
    screenResolveMaterial->InvalidateRenderVariants();
    deferredLightsRenderer->InvalidateMaterials();
}

void HDRDeferredPass::GBufferResolvePass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();

    Vector4 lightsCount = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_LIGHTING_PARAMETERS, lightsCount.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    Texture* reflectionSpecularConvolution2 = renderSystem->GetReflectionRenderer()->GetSpecularConvolution2();
    if (screenResolveMaterial->HasLocalTexture(NMaterialTextureName::TEXTURE_GLOBAL_REFLECTION))
    {
        screenResolveMaterial->SetTexture(NMaterialTextureName::TEXTURE_GLOBAL_REFLECTION, reflectionSpecularConvolution2);
    }
    else
    {
        screenResolveMaterial->AddTexture(NMaterialTextureName::TEXTURE_GLOBAL_REFLECTION, reflectionSpecularConvolution2);
    }

    // from DrawLayers
    ShaderAssetListener::Instance()->ClearDynamicBindigs();
    SetupCameraParams(mainCamera, drawCamera);

    if (BeginRenderPass(passConfig))
    {
        //per pass viewport bindings
        viewportSize = Vector2(viewport.dx, viewport.dy);
        rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
        viewportOffset = Vector2(viewport.x, viewport.y);
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));

        //draw full screen quad with attached gBuffers
        if (screenResolveMaterial->PreBuildMaterial(passName))
        {
            screenResolveMaterial->BindParams(screenResolvePacket);
            DAVA_PROFILER_GPU_PACKET(screenResolvePacket, ProfilerGPUMarkerName::GBUFFER_RESOLVE);
            rhi::AddPacket(packetList, screenResolvePacket);
        }

        visibilityArray.Clear();
        renderSystem->GetRenderHierarchy()->Clip(mainCamera, visibilityArray, RenderObject::CLIPPING_VISIBILITY_CRITERIA);
        deferredLightsRenderer->Draw(visibilityArray, packetList);

        EndRenderPass();
    }
}

HDRDeferredPass::HDRDeferredPass()
    : RenderPass(PASS_FORWARD) // it is 'PASS_FORWARD' - so it will draw translucent forward geometry - old architecture kostyl
{
    AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_SKY_ID, 0));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_DEBUG_DRAW_ID, RenderLayer::LAYER_SORTING_FLAGS_DEBUG_DRAW));
    passConfig.priority = PRIORITY_MAIN_3D;
    passConfig.usesReverseDepth = rhi::DeviceCaps().isReverseDepthSupported;

    Size2i gBufferSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_GBUFFER_0);

    gBufferPass = new GBufferPass();
    gBufferPass->GetPassConfig().colorBuffer[0].clearColor[0] = 0.0f;
    gBufferPass->GetPassConfig().colorBuffer[0].clearColor[1] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[0].clearColor[2] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[0].clearColor[3] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_0);
    gBufferPass->GetPassConfig().colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    gBufferPass->GetPassConfig().colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    gBufferPass->GetPassConfig().colorBuffer[1].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_1);
    gBufferPass->GetPassConfig().colorBuffer[1].loadAction = rhi::LOADACTION_CLEAR;
    gBufferPass->GetPassConfig().colorBuffer[1].storeAction = rhi::STOREACTION_STORE;
    gBufferPass->GetPassConfig().colorBuffer[2].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_2);
    gBufferPass->GetPassConfig().colorBuffer[2].clearColor[0] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[2].clearColor[1] = 0.0f;
    gBufferPass->GetPassConfig().colorBuffer[2].clearColor[2] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[2].clearColor[3] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[2].loadAction = rhi::LOADACTION_CLEAR;
    gBufferPass->GetPassConfig().colorBuffer[2].storeAction = rhi::STOREACTION_STORE;
    gBufferPass->GetPassConfig().colorBuffer[3].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3);
    gBufferPass->GetPassConfig().colorBuffer[3].loadAction = rhi::LOADACTION_CLEAR;
    gBufferPass->GetPassConfig().colorBuffer[3].storeAction = rhi::STOREACTION_STORE;
    gBufferPass->GetPassConfig().colorBuffer[3].clearColor[0] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[3].clearColor[1] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[3].clearColor[2] = 1.0f;
    gBufferPass->GetPassConfig().colorBuffer[3].clearColor[3] = 1.0f;
    gBufferPass->GetPassConfig().depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
    gBufferPass->GetPassConfig().depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    gBufferPass->GetPassConfig().depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    gBufferPass->GetPassConfig().usesReverseDepth = passConfig.usesReverseDepth;
    gBufferPass->SetRenderTargetProperties(gBufferSize.dx, gBufferSize.dy, Renderer::GetRuntimeTextures().GetRuntimeTextureFormat(RuntimeTextures::TEXTURE_GBUFFER_0));

    deferredDecalPass = new DeferredDecalPass();
    deferredDecalPass->GetPassConfig().colorBuffer[0].clearColor[0] = 1.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_0);
    deferredDecalPass->GetPassConfig().colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    deferredDecalPass->GetPassConfig().colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    deferredDecalPass->GetPassConfig().colorBuffer[1].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_1);
    deferredDecalPass->GetPassConfig().colorBuffer[1].loadAction = rhi::LOADACTION_LOAD;
    deferredDecalPass->GetPassConfig().colorBuffer[1].storeAction = rhi::STOREACTION_STORE;
    deferredDecalPass->GetPassConfig().colorBuffer[2].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_2);
    deferredDecalPass->GetPassConfig().colorBuffer[2].clearColor[0] = 1.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[2].clearColor[1] = 0.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[2].clearColor[2] = 1.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[2].clearColor[3] = 1.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[2].loadAction = rhi::LOADACTION_LOAD;
    deferredDecalPass->GetPassConfig().colorBuffer[2].storeAction = rhi::STOREACTION_STORE;
    deferredDecalPass->GetPassConfig().colorBuffer[3].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3);
    deferredDecalPass->GetPassConfig().colorBuffer[3].loadAction = rhi::LOADACTION_LOAD;
    deferredDecalPass->GetPassConfig().colorBuffer[3].storeAction = rhi::STOREACTION_STORE;
    deferredDecalPass->GetPassConfig().colorBuffer[3].clearColor[0] = 1.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[3].clearColor[1] = 1.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[3].clearColor[2] = 1.0f;
    deferredDecalPass->GetPassConfig().colorBuffer[3].clearColor[3] = 1.0f;
    deferredDecalPass->GetPassConfig().depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
    deferredDecalPass->GetPassConfig().depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    deferredDecalPass->GetPassConfig().depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
    deferredDecalPass->GetPassConfig().usesReverseDepth = passConfig.usesReverseDepth;
    deferredDecalPass->SetRenderTargetProperties(gBufferSize.dx, gBufferSize.dy, Renderer::GetRuntimeTextures().GetRuntimeTextureFormat(RuntimeTextures::TEXTURE_GBUFFER_0));

    gBufferResolvePass = new GBufferResolvePass();
    gBufferResolvePass->GetPassConfig().colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    gBufferResolvePass->GetPassConfig().colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    gBufferResolvePass->GetPassConfig().depthStencilBuffer.texture = rhi::InvalidHandle;
    gBufferResolvePass->GetPassConfig().depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    gBufferResolvePass->GetPassConfig().depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    gBufferResolvePass->GetPassConfig().usesReverseDepth = passConfig.usesReverseDepth;

    velocityPass = new VelocityPass();
}

void HDRDeferredPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    DebugDumpGBuffers();

    rhi::RenderPassConfig originalConfig = passConfig;
    Rect originalVP = viewport;
    {
        passConfig.colorBuffer[0].texture = renderSystem->GetPostEffectRenderer()->GetHDRTarget();
        SetViewport(Rect(0.0, 0.0, originalVP.dx, originalVP.dy));
        Size2i rtSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
        SetRenderTargetProperties(rtSize.dx, rtSize.dy, FORMAT_RGBA16F);
    }
    passConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

    //draw to g-buffer
    gBufferPass->GetPassConfig().priority = passConfig.priority + PRIORITY_SERVICE_3D;
    gBufferPass->SetEnableFrameJittering(enableFrameJittering);
    gBufferPass->SetViewport(viewport);
    gBufferPass->Draw(renderSystem);

    // velocity  pass
    velocityPass->GetPassConfig().priority = passConfig.priority + PRIORITY_SERVICE_3D - 1;
    velocityPass->SetEnableFrameJittering(enableFrameJittering);
    velocityPass->SetViewport(viewport);
    velocityPass->Draw(renderSystem);

    //deferred decals
    deferredDecalPass->GetPassConfig().priority = passConfig.priority + PRIORITY_SERVICE_3D - 1; //right after
    deferredDecalPass->SetViewport(viewport);
    deferredDecalPass->Draw(renderSystem);

    //resolve g-buffer
    gBufferResolvePass->GetPassConfig().priority = passConfig.priority + PRIORITY_MAIN_3D + 1; //right before
    gBufferResolvePass->SetViewport(viewport);
    gBufferResolvePass->SetRenderTargetProperties(renderTargetProperties.width, renderTargetProperties.height, renderTargetProperties.format);
    gBufferResolvePass->GetPassConfig().colorBuffer[0] = passConfig.colorBuffer[0];
    gBufferResolvePass->GetPassConfig().colorBuffer[0].clearColor[0] = 0.25f;
    gBufferResolvePass->GetPassConfig().colorBuffer[0].clearColor[1] = 1.0f;
    gBufferResolvePass->GetPassConfig().colorBuffer[0].clearColor[2] = 1.0f;
    gBufferResolvePass->GetPassConfig().colorBuffer[0].clearColor[3] = 1.0f;
    gBufferResolvePass->GetPassConfig().depthStencilBuffer = passConfig.depthStencilBuffer;

    gBufferResolvePass->Draw(renderSystem);

    //GFX_COMPLETE - right now forward stuff requires post effect
    if (QualitySettingsSystem::Instance()->IsOptionEnabled(QualitySettingsSystem::QUALITY_OPTION_DEFERRED_DRAW_FORWARD))
    {
        for (Landscape* landscape : renderSystem->GetLandscapes())
            landscape->SetPageUpdateLocked(true);

        Camera* mainCamera = renderSystem->GetMainCamera();
        Camera* drawCamera = renderSystem->GetDrawCamera();
        SetupCameraParams(mainCamera, drawCamera);
        PrepareVisibilityArrays(mainCamera, renderSystem);
        //draw forward stuff
        passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
        passConfig.priority = passConfig.priority + PRIORITY_MAIN_3D;
        if (BeginRenderPass(passConfig))
        {
            DrawLayers(mainCamera, drawLayersMask);
            EndRenderPass();
        }

        for (Landscape* landscape : renderSystem->GetLandscapes())
            landscape->SetPageUpdateLocked(false);
    }
    else
    {
        renderSystem->GetDebugDrawer()->Clear();
    }

    passConfig = originalConfig;

    {
        SetViewport(originalVP);
        PostEffectRenderer* postEffectRenderer = renderSystem->GetPostEffectRenderer();
        postEffectRenderer->Render(passConfig.colorBuffer[0].texture, passConfig.viewport);
    }
}

void HDRDeferredPass::InvalidateMaterials()
{
    gBufferResolvePass->InvalidateMaterials();
}

HDRDeferredPass::~HDRDeferredPass()
{
    SafeDelete(gBufferPass);
    SafeDelete(gBufferResolvePass);
    SafeDelete(velocityPass);
    SafeDelete(deferredDecalPass);
}

void HDRDeferredPass::DebugDumpGBuffers()
{
#define DUMP_GBUFFERS 0
#define DUMP_GBUFFERS_COPYS 0
#if DUMP_GBUFFERS
    static int i = 1;
    i++;
    if (!(i % 200))
    {
        Logger::Info(" ******* SAVING GBUFFERS ******");

        String folder = String("~doc:/debug_texture_dump/");
        String fileName;
        FileSystem::Instance()->CreateDirectory(FilePath(folder), true);
        Size2i size = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_GBUFFER_0);
        Image* img;
        void* data;
        rhi::HTexture tex;

        fileName = folder + "gBuffer0.png";
        tex = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_0);
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);

        fileName = folder + "gBuffer1.png";
        tex = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_1);
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);

        fileName = folder + "gBuffer2.png";
        tex = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_2);
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);

        fileName = folder + "gBuffer3.png";
        tex = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3);
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);

#if DUMP_GBUFFERS_COPYS
        fileName = folder + "gBuffer0_copy.png";
        tex = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_0_COPY);
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);

        fileName = folder + "gBuffer1_copy.png";
        tex = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_1_COPY);
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);

        fileName = folder + "gBuffer2_copy.png";
        tex = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_2_COPY);
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);
#endif
    }
#endif
}
}
