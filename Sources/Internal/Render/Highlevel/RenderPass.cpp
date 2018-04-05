#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/ShadowVolumeRenderLayer.h"
#include "Render/Highlevel/PostEffectRenderer.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "Render/ShaderCache.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Concurrency/Thread.h"

#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "Render/Image/ImageSystem.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/VisibilityQueryResults.h"

#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Particles/ParticlesRandom.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

namespace DAVA
{
RenderPass::RenderPass(const FastName& _name)
    : passName(_name)
{
    DVASSERT(passName.empty() == false);

    renderLayers.reserve(RENDER_LAYER_ID_COUNT);

    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].clearColor[0] = 0.0f;
    passConfig.colorBuffer[0].clearColor[1] = 0.0f;
    passConfig.colorBuffer[0].clearColor[2] = 0.0f;
    passConfig.colorBuffer[0].clearColor[3] = 1.0f;
    passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
    passConfig.priority = PRIORITY_MAIN_3D;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = Renderer::GetFramebufferWidth();
    passConfig.viewport.height = Renderer::GetFramebufferHeight();
}

RenderPass::~RenderPass()
{
    ClearLayersArrays();
    for (RenderLayer* layer : renderLayers)
    {
        SafeDelete(layer);
    }
    SafeRelease(multisampledTexture);
}

void RenderPass::AddRenderLayer(RenderLayer* layer, eRenderLayerID afterLayer)
{
    if (RENDER_LAYER_INVALID_ID != afterLayer)
    {
        uint32 size = static_cast<uint32>(renderLayers.size());
        for (uint32 i = 0; i < size; ++i)
        {
            eRenderLayerID layerID = renderLayers[i]->GetRenderLayerID();
            if (afterLayer == layerID)
            {
                renderLayers.insert(renderLayers.begin() + i + 1, layer);
                layersBatchArrays[layerID].SetSortingFlags(layer->GetSortingFlags());
                return;
            }
        }
        DVASSERT(0 && "RenderPass::AddRenderLayer afterLayer not found");
    }
    else
    {
        renderLayers.push_back(layer);
        layersBatchArrays[layer->GetRenderLayerID()].SetSortingFlags(layer->GetSortingFlags());
    }
}

void RenderPass::RemoveRenderLayer(RenderLayer* layer)
{
    Vector<RenderLayer*>::iterator it = std::find(renderLayers.begin(), renderLayers.end(), layer);
    DVASSERT(it != renderLayers.end());

    renderLayers.erase(it);
}

void RenderPass::SetupCameraParams(Camera* mainCamera, Camera* drawCamera, Vector4* externalClipPlane)
{
    DVASSERT(drawCamera);
    DVASSERT(mainCamera);

    bool invertProjection = rhi::IsInvertedProjectionRequired(passConfig.IsRenderTargetPass(), passConfig.IsCubeRenderTargetPass());

    Vector2 offset = enableFrameJittering ? GetCurrentFrameJitterOffset() : Vector2::Zero;
    mainCamera->SetProjectionJitterOffset(offset);

    drawCamera->SetupDynamicParameters(invertProjection, passConfig.usesReverseDepth, externalClipPlane);

    if (mainCamera != drawCamera)
        mainCamera->PrepareDynamicParameters(invertProjection, passConfig.usesReverseDepth, externalClipPlane);
    bindFlags = 0;
    if (invertProjection)
        bindFlags |= NMaterial::FLAG_INV_CULL;
    if (passConfig.usesReverseDepth)
        bindFlags |= NMaterial::FLAG_INV_Z;
}

void RenderPass::Draw(RenderSystem* renderSystem, uint32 drawLayersMask)
{
    Camera* mainCamera = renderSystem->GetMainCamera();

    Clip(mainCamera, renderSystem);
    PrepareRenderObjectsToRender(visibilityArray.geometryArray, mainCamera);

    DrawVisibilityArray(renderSystem, visibilityArray, drawLayersMask);
}

void RenderPass::DrawVisibilityArray(RenderSystem* renderSystem, RenderHierarchy::ClipResult& preparedVisibilityArray, uint32 drawLayersMask /*= 0xFFFFFFFF*/)
{
    Camera* mainCamera = renderSystem->GetMainCamera();
    Camera* drawCamera = renderSystem->GetDrawCamera();
    SetupCameraParams(mainCamera, drawCamera);

    ClearLayersArrays();
    PrepareLayersArrays(preparedVisibilityArray.geometryArray, mainCamera);

    if (BeginRenderPass(passConfig))
    {
        DrawLayers(mainCamera, drawLayersMask);
        EndRenderPass();
    }
}

void RenderPass::PrepareVisibilityArrays(Camera* camera, RenderSystem* renderSystem)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::RENDER_PASS_PREPARE_ARRAYS)

    Clip(camera, renderSystem);
    PrepareRenderObjectsToRender(visibilityArray.geometryArray, camera);
    ClearLayersArrays();
    PrepareLayersArrays(visibilityArray.geometryArray, camera);
}

void RenderPass::Clip(Camera* camera, RenderSystem* renderSystem)
{
    uint32 currVisibilityCriteria = RenderObject::CLIPPING_VISIBILITY_CRITERIA;
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::ENABLE_STATIC_OCCLUSION))
        currVisibilityCriteria &= ~RenderObject::VISIBLE_STATIC_OCCLUSION;

    visibilityArray.Clear();
    renderSystem->GetRenderHierarchy()->Clip(camera, visibilityArray, currVisibilityCriteria);
}

void RenderPass::PrepareLayersArrays(const Vector<RenderObject*>& objectsArray, Camera* camera)
{
    size_t size = objectsArray.size();
    for (size_t ro = 0; ro < size; ++ro)
    {
        RenderObject* renderObject = objectsArray[ro];
        uint32 batchCount = renderObject->GetActiveRenderBatchCount();
        for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
        {
            RenderBatch* batch = renderObject->GetActiveRenderBatch(batchIndex);

            NMaterial* material = batch->GetMaterial();
            DVASSERT(material);
            if (material->PreBuildMaterial(passName))
            {
                layersBatchArrays[material->GetRenderLayerID()].AddRenderBatch(batch);
            }
        }
    }
}

void RenderPass::PrepareRenderObjectsToRender(const Vector<RenderObject*>& objectsArray, Camera* camera)
{
    for (RenderObject* renderObject : objectsArray)
    {
        if (renderObject->GetFlags() & RenderObject::CUSTOM_PREPARE_TO_RENDER)
            renderObject->PrepareToRender(camera);
    }
}

void RenderPass::DrawLayers(Camera* camera, uint32 drawLayersMask)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::RENDER_PASS_DRAW_LAYERS)

    ShaderDescriptorCache::ClearDynamicBindigs();

    //per pass viewport bindings
    viewportSize = Vector2(viewport.dx, viewport.dy);
    rcpViewportSize = Vector2(1.0f / viewport.dx, 1.0f / viewport.dy);
    viewportOffset = Vector2(viewport.x, viewport.y);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_SIZE, &viewportSize, reinterpret_cast<pointer_size>(&viewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RCP_VIEWPORT_SIZE, &rcpViewportSize, reinterpret_cast<pointer_size>(&rcpViewportSize));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_VIEWPORT_OFFSET, &viewportOffset, reinterpret_cast<pointer_size>(&viewportOffset));

    SetNdcToZMapping();

    size_t size = renderLayers.size();
    for (size_t k = 0; k < size; ++k)
    {
        RenderLayer* layer = renderLayers[k];

        eRenderLayerID id = layer->GetRenderLayerID();
        if (drawLayersMask & (1 << id))
        {
            RenderBatchArray& batchArray = layersBatchArrays[id];
            batchArray.Sort(camera);
            layer->Draw(camera, batchArray, packetList, bindFlags);
        }
    }
}

void RenderPass::DrawDebug(Camera* camera, RenderSystem* renderSystem)
{
    if (!renderSystem->GetDebugDrawer()->IsEmpty())
    {
        renderSystem->GetDebugDrawer()->Present(packetList, &camera->GetMatrix(), &camera->GetProjectionMatrix(), bindFlags);
        renderSystem->GetDebugDrawer()->Clear();
    }
}

void RenderPass::SetRenderTargetProperties(uint32 width, uint32 height, PixelFormat format)
{
    renderTargetProperties.width = width;
    renderTargetProperties.height = height;
    renderTargetProperties.format = format;
    renderTargetProperties.dimensions = Vector2(float(width), float(height));
}

void RenderPass::ValidateMultisampledTextures(const rhi::RenderPassConfig& config)
{
    uint32 requestedSamples = rhi::TextureSampleCountForAAType(config.antialiasingType);

    bool invalidDescription =
    (multisampledDescription.sampleCount != requestedSamples) ||
    (multisampledDescription.format != renderTargetProperties.format) ||
    (multisampledDescription.width != renderTargetProperties.width) ||
    (multisampledDescription.height != renderTargetProperties.height);

    if (invalidDescription || (multisampledTexture == nullptr))
    {
        SafeRelease(multisampledTexture);

        multisampledDescription.width = renderTargetProperties.width;
        multisampledDescription.height = renderTargetProperties.height;
        multisampledDescription.format = renderTargetProperties.format;
        multisampledDescription.needDepth = true;
        multisampledDescription.needPixelReadback = false;
        multisampledDescription.ensurePowerOf2 = false;
        multisampledDescription.sampleCount = requestedSamples;

        multisampledTexture = Texture::CreateFBO(multisampledDescription);
    }
}

void RenderPass::SetNdcToZMapping()
{
    static const Vector2 glNdcToZMapping(0.5f, 0.5f);
    static const Vector2 nonGlNdcToZMapping(1.0f, 0.0f);

    if (Renderer::GetAPI() == rhi::RHI_GLES2 && !passConfig.usesReverseDepth)
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_NDC_TO_Z_MAPPING, glNdcToZMapping.data, reinterpret_cast<pointer_size>(glNdcToZMapping.data));
    else
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_NDC_TO_Z_MAPPING, nonGlNdcToZMapping.data, reinterpret_cast<pointer_size>(nonGlNdcToZMapping.data));
}

Vector2 RenderPass::GetCurrentFrameJitterOffset() const
{
    Size2i size = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);

    int32 taaSampleIndex = Renderer::GetOptions()->GetOptionValue(RenderOptions::TAA_SAMPLE_INDEX);
    if (taaSampleIndex == -1)
        taaSampleIndex = Engine::Instance()->GetGlobalFrameIndex();

    Vector2 currOffset = ParticlesRandom::SobolSequenceV2Prebult(uint32(taaSampleIndex));
    return Vector2(currOffset.x / float32(size.dx), currOffset.y / float32(size.dy)) * 2.0f;
}

bool RenderPass::BeginRenderPass(const rhi::RenderPassConfig& config)
{
    bool success = false;

    //GFX_COMPLETE binding in BeginRenderPass is trash - everything should be set up before
    const float minHalfPrecision = std::pow(2.0f, -14.0f);
    currentDistantDepthValue = config.usesReverseDepth ? (0.0f + minHalfPrecision) : (1.0f - minHalfPrecision);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_RENDER_TARGET_SIZE, renderTargetProperties.dimensions.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_DISTANT_DEPTH_VALUE, &currentDistantDepthValue, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    rhi::RenderPassConfig internalConfig = config;
    if (internalConfig.name == nullptr)
        internalConfig.name = passName.c_str();
    
#ifdef __DAVAENGINE_RENDERSTATS__
    internalConfig.queryBuffer = VisibilityQueryResults::GetQueryBuffer();
#endif

    if (internalConfig.antialiasingType != rhi::AntialiasingType::NONE)
    {
        DVASSERT(renderTargetProperties.width > 0);
        DVASSERT(renderTargetProperties.height > 0);
        DVASSERT(renderTargetProperties.format != PixelFormat::FORMAT_INVALID);

        ValidateMultisampledTextures(internalConfig);
        internalConfig.colorBuffer[0].multisampleTexture = multisampledTexture->handle;
        internalConfig.depthStencilBuffer.multisampleTexture = multisampledTexture->handleDepthStencil;
    }

    renderPass = rhi::AllocateRenderPass(internalConfig, 1, &packetList);
    if (renderPass != rhi::InvalidHandle)
    {
        rhi::BeginRenderPass(renderPass);
        rhi::BeginPacketList(packetList);
        success = true;
    }

    return success;
}

void RenderPass::EndRenderPass()
{
    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(renderPass);
}

void RenderPass::ClearLayersArrays()
{
    for (uint32 id = 0; id < static_cast<uint32>(RENDER_LAYER_ID_COUNT); ++id)
    {
        layersBatchArrays[id].Clear();
    }
}
}
