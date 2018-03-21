#include "HDRDeferredFetchPass.h"
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
#include "Render/Highlevel/RenderLayer.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Asset/AssetManager.h"

//for debug dump gbuffers
#include "Logger/Logger.h"
#include "Render/Image/Image.h"

namespace DAVA
{
HDRDeferredFetchPass::HDRDeferredFetchPass()
    : RenderPass(PASS_GBUFFER_FETCH)
{
    defaultCubemap = GetEngineContext()->assetManager->GetAsset<Texture>(Texture::MakePinkKey(rhi::TextureType::TEXTURE_TYPE_CUBE), AssetManager::SYNC);
    //init layers
    //deferred
    AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));

    //forward
    AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_SKY_ID, 0));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT));
    AddRenderLayer(new RenderLayer(RENDER_LAYER_DEBUG_DRAW_ID, RenderLayer::LAYER_SORTING_FLAGS_DEBUG_DRAW));

    deferredLightsRenderer = new DeferredLightsRenderer(true);
    deferredDecalRenderer = new DeferredDecalRenderer(true);

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

    Renderer::GetDynamicBindings().SetDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_GLOBAL_REFLECTION, defaultCubemap->handle);
    Renderer::GetDynamicBindings().SetDynamicTexture(DynamicBindings::DYNAMIC_TEXTURE_LOCAL_REFLECTION, defaultCubemap->handle);

    GetPassConfig().usesReverseDepth = rhi::DeviceCaps().isReverseDepthSupported;
}

HDRDeferredFetchPass::~HDRDeferredFetchPass()
{
    SafeDelete(deferredDecalRenderer);
    SafeDelete(deferredLightsRenderer);
    SafeRelease(screenResolveMaterial);
    rhi::DeleteVertexBuffer(quadBuffer);
}

void HDRDeferredFetchPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    const uint32 forwardMask = RenderLayer::MakeLayerMask({ RENDER_LAYER_AFTER_OPAQUE_ID, RENDER_LAYER_SKY_ID, RENDER_LAYER_TRANSLUCENT_ID, RENDER_LAYER_AFTER_TRANSLUCENT_ID, RENDER_LAYER_DEBUG_DRAW_ID });

    DAVA_PROFILER_GPU_RENDER_PASS(deferredPassConfig, ProfilerGPUMarkerName::GBUFFER_PASS);

    //if no postefect resolve it to screen? it might be not possible
    Rect originalVP = viewport;
    PreparePassConfig(renderSystem->GetPostEffectRenderer()->GetHDRTarget());

    //GFX_COMPLETE - GBufferPass and DeferredDecalPass actually share camera params and visibility arrays - so we can do clipping only once
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);

    PrepareVisibilityArrays(mainCamera, renderSystem);

    //per pass viewport bindings
    viewportSize = Vector2(viewport.dx, viewport.dy);
    rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
    viewportOffset = Vector2(viewport.x, viewport.y);
    Vector4 lightsCount = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_LIGHTING_PARAMETERS, lightsCount.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    deferredPassConfig.priority = passConfig.priority + PRIORITY_SERVICE_3D - 1;

    PostEffectRenderer* postEffectRenderer = renderSystem->GetPostEffectRenderer();

    if (BeginRenderPass(deferredPassConfig))
    {
        // draw main deferred stuff
        DrawLayers(mainCamera, 1 << RENDER_LAYER_OPAQUE_ID);
        // draw decals
        // deferredDecalRenderer->Draw(visibilityArray, packetList);
        // draw screen resolve
        if (screenResolveMaterial->PreBuildMaterial(PASS_GBUFFER_FETCH))
        {
            screenResolveMaterial->BindParams(screenResolvePacket);
            DAVA_PROFILER_GPU_PACKET(screenResolvePacket, ProfilerGPUMarkerName::GBUFFER_RESOLVE);
            rhi::AddPacket(packetList, screenResolvePacket);
        }
        // draw deferred lights
        // deferredLightsRenderer->Draw(visibilityArray, packetList);

        DrawLayers(mainCamera, forwardMask);

        postEffectRenderer->Combine(PostEffectRenderer::CombineMode::InplaceDeferred, packetList);

        EndRenderPass();
    }

    rhi::HTexture luminanceTexture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3);
    postEffectRenderer->DownsampleLuminanceInplace(luminanceTexture, Size2i(passConfig.viewport.width, passConfig.viewport.height), -3);
}

void HDRDeferredFetchPass::UpdateScreenResolveData(RenderSystem* renderSystem)
{
    Asset<Texture> reflectionSpecularConvolution2 = renderSystem->GetReflectionRenderer()->GetSpecularConvolution2();
    if (screenResolveMaterial->HasLocalTexture(NMaterialTextureName::TEXTURE_GLOBAL_REFLECTION))
    {
        screenResolveMaterial->SetTexture(NMaterialTextureName::TEXTURE_GLOBAL_REFLECTION, reflectionSpecularConvolution2);
    }
    else
    {
        screenResolveMaterial->AddTexture(NMaterialTextureName::TEXTURE_GLOBAL_REFLECTION, reflectionSpecularConvolution2);
    }
}

void HDRDeferredFetchPass::InvalidateMaterials()
{
    screenResolveMaterial->InvalidateRenderVariants();
}

void HDRDeferredFetchPass::PreparePassConfig(rhi::HTexture colorTarget)
{
    if (!gbuffersInited)
    {
        deferredPassConfig.priority = PRIORITY_MAIN_3D;

        deferredPassConfig.colorBuffer[0].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_0);
        deferredPassConfig.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
        deferredPassConfig.colorBuffer[0].storeAction = rhi::STOREACTION_NONE;

        deferredPassConfig.colorBuffer[1].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_1);
        deferredPassConfig.colorBuffer[1].loadAction = rhi::LOADACTION_NONE;
        deferredPassConfig.colorBuffer[1].storeAction = rhi::STOREACTION_NONE;

        deferredPassConfig.colorBuffer[2].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_2);
        deferredPassConfig.colorBuffer[2].loadAction = rhi::LOADACTION_NONE;
        deferredPassConfig.colorBuffer[2].storeAction = rhi::STOREACTION_NONE;

        deferredPassConfig.colorBuffer[3].texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_GBUFFER_3);
        deferredPassConfig.colorBuffer[3].loadAction = rhi::LOADACTION_NONE;
        deferredPassConfig.colorBuffer[3].storeAction = rhi::STOREACTION_STORE;

        deferredPassConfig.colorBuffer[4].loadAction = rhi::LOADACTION_NONE;
        deferredPassConfig.colorBuffer[4].storeAction = rhi::STOREACTION_NONE;

        deferredPassConfig.colorBuffer[5].texture = rhi::InvalidHandle;
        deferredPassConfig.colorBuffer[5].loadAction = rhi::LOADACTION_NONE;
        deferredPassConfig.colorBuffer[5].storeAction = rhi::STOREACTION_STORE;

        deferredPassConfig.explicitColorBuffersCount = 6;
        deferredPassConfig.usesReverseDepth = GetPassConfig().usesReverseDepth;

        deferredPassConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
        deferredPassConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;

        Size2i gBufferSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_GBUFFER_0);
        SetRenderTargetProperties(gBufferSize.dx, gBufferSize.dy, Renderer::GetRuntimeTextures().GetRuntimeTextureFormat(RuntimeTextures::TEXTURE_GBUFFER_0));

        gbuffersInited = true;
    }

    // per frame reconfiguration
    {
        deferredPassConfig.colorBuffer[4].texture = colorTarget;
        deferredPassConfig.colorBuffer[5].texture = rhi::InvalidHandle;
        forwardStuffPassConfig.colorBuffer[0].texture = colorTarget;
        SetViewport(Rect(0.0, 0.0, viewport.dx, viewport.dy));
    }

    forwardStuffPassConfig.usesReverseDepth = GetPassConfig().usesReverseDepth;
    forwardStuffPassConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
    deferredPassConfig.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
}
}
