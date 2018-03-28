#include "Scene3D/Entity.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderBatchArray.h"

#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/LDRForwardPass.h"
#include "Render/Highlevel/HDRForwardPass.h"
#include "Render/Highlevel/HDRDeferredPass.h"
#include "Render/Highlevel/HDRDeferredFetchPass.h"
#include "Render/Highlevel/UtilityRenderPass.h"
#include "Render/Highlevel/PickingRenderPass.h"

#include "Render/Highlevel/VisibilityQuadTree.h"
#include "Render/Highlevel/PostEffectRenderer.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "Render/Highlevel/CubemapRenderer.h"
#include "Render/Highlevel/VTDecalManager.h"
#include "Render/Highlevel/DecalRenderObject.h"

#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/TextureDescriptor.h"
#include "Render/ShaderCache.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"
#include "Debug/ProfilerGPU.h"
#include "Debug/ProfilerMarkerNames.h"

#if (!__DAVAENGINE_IPHONE__) && (!__DAVAENGINE_ANDROID__)
    #define ENABLE_DEBUG_DRAW 1
#endif

#define LOG_CAMERA_POSITION 0
#define DEBUG_DRAW_SPATIAL_HIERARCHY 0
#define DEBUG_DRAW_VT_HIERARCHY 0

namespace DAVA
{
RenderSystem::RenderSystem()
{
    renderHierarchy = new QuadTree(10);
    vtDecalManager = new VTDecalManager(40); //GFX_COMPLETE - make it at least configurable
    markedObjects.reserve(100);
    velocityUpdatedObjects.reserve(100);
    debugDrawer = new RenderHelper();
    postEffectRenderer = new PostEffectRenderer();
    cubemapRenderer = new CubemapRenderer();
    geoDecalManager = new GeoDecalManager();
    reflectionRenderer = new ReflectionRenderer(this);
    rescalePass = new RescalePass();
    pickingRenderPass = new PickingRenderPass();
}

RenderSystem::~RenderSystem()
{
    SafeRelease(mainCamera);
    SafeRelease(drawCamera);
    SafeRelease(globalMaterial);

    SafeDelete(cubemapRenderer);
    SafeDelete(reflectionRenderer);
    SafeDelete(postEffectRenderer);

    SafeDelete(renderHierarchy);
    SafeDelete(vtDecalManager);
    SafeDelete(debugDrawer);
    SafeDelete(geoDecalManager);

    SafeDelete(rescalePass);
    SafeDelete(pickingRenderPass);

    SafeRelease(ldrCurrent);
    SafeRelease(ldrHistory);
}

void RenderSystem::RenderPermanent(RenderObject* renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() == static_cast<uint32>(-1));

    /*on add calculate valid world bbox*/
    renderObject->Retain();
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex(static_cast<uint32>(renderObjectArray.size() - 1));

    AddRenderObject(renderObject);
}

void RenderSystem::RemoveFromRender(RenderObject* renderObject)
{
    DVASSERT(renderObject->GetRemoveIndex() != static_cast<uint32>(-1));

    FindAndRemoveExchangingWithLast(markedObjects, renderObject);
    renderObject->RemoveFlag(RenderObject::MARKED_FOR_UPDATE);

    RenderObject* lastRenderObject = renderObjectArray[renderObjectArray.size() - 1];
    renderObjectArray[renderObject->GetRemoveIndex()] = lastRenderObject;
    renderObjectArray.pop_back();
    lastRenderObject->SetRemoveIndex(renderObject->GetRemoveIndex());
    renderObject->SetRemoveIndex(-1);

    RemoveRenderObject(renderObject);

    renderObject->Release();
}

void RenderSystem::AddRenderObject(RenderObject* renderObject)
{
    renderObject->RecalculateWorldBoundingBox();

    if ((renderObject->GetType() == RenderObject::TYPE_DECAL) && (static_cast<DecalRenderObject*>(renderObject)->GetDomain() == DecalRenderObject::DOMAIN_VT))
        vtDecalManager->AddDecal(static_cast<DecalRenderObject*>(renderObject));
    else
        renderHierarchy->AddRenderObject(renderObject);

    if (renderObject->GetType() == RenderObject::TYPE_LANDSCAPE)
        allLandscapes.emplace_back(static_cast<Landscape*>(renderObject));

    renderObject->SetRenderSystem(this);

    uint32 size = renderObject->GetRenderBatchCount();
    for (uint32 i = 0; i < size; ++i)
    {
        RenderBatch* batch = renderObject->GetRenderBatch(i);
        RegisterBatch(batch);
    }
}

void RenderSystem::RemoveRenderObject(RenderObject* renderObject)
{
    auto i = std::remove(allLandscapes.begin(), allLandscapes.end(), renderObject);
    if (i != allLandscapes.end())
        allLandscapes.erase(i);

    uint32 size = renderObject->GetRenderBatchCount();
    for (uint32 i = 0; i < size; ++i)
    {
        RenderBatch* batch = renderObject->GetRenderBatch(i);
        UnregisterBatch(batch);
    }

    if ((renderObject->GetType() == RenderObject::TYPE_DECAL) && (static_cast<DecalRenderObject*>(renderObject)->GetDomain() == DecalRenderObject::DOMAIN_VT))
        vtDecalManager->RemoveDecal(static_cast<DecalRenderObject*>(renderObject));
    else
    {
        geoDecalManager->RemoveRenderObject(renderObject);
        renderHierarchy->RemoveRenderObject(renderObject);
    }

    renderObject->SetRenderSystem(nullptr);
}

void RenderSystem::PrebuildMaterial(NMaterial* material)
{
    if (activeRenderPass != nullptr)
        material->PreBuildMaterial(activeRenderPass->GetName());
}

void RenderSystem::RegisterBatch(RenderBatch* batch)
{
    RegisterMaterial(batch->GetMaterial());
}

void RenderSystem::UnregisterBatch(RenderBatch* batch)
{
    UnregisterMaterial(batch->GetMaterial());
}

void RenderSystem::RegisterMaterial(NMaterial* material)
{
    if (material == nullptr)
        return;

    // set globalMaterial to be parent for top material
    NMaterial* topParent = material->GetTopLevelParent();
    if (topParent != globalMaterial && topParent->GetMaterialType() == NMaterial::TYPE_LEGACY)
    {
        topParent->SetParent(globalMaterial);
    }

    PrebuildMaterial(material);
}

void RenderSystem::UnregisterMaterial(NMaterial* material)
{
    /*
    if (!material) return;

    while (material->GetParent() && material->GetParent() != globalMaterial)
    {
        material = material->GetParent();
    }

    if (material->GetParent())
    {
        material->SetParent(nullptr);
    }
    */
}

void RenderSystem::PrepareForShutdown()
{
    renderHierarchy->PrepareForShutdown();
    vtDecalManager->PrepareForShutdown();
}

void RenderSystem::SetGlobalMaterial(NMaterial* newGlobalMaterial)
{
    Set<DataNode*> dataNodes;
    for (RenderObject* obj : renderObjectArray)
    {
        obj->GetDataNodes(dataNodes);
    }
    for (DataNode* dataNode : dataNodes)
    {
        NMaterial* batchMaterial = dynamic_cast<NMaterial*>(dataNode);
        if (batchMaterial && batchMaterial->GetMaterialType() == NMaterial::TYPE_LEGACY)
        {
            NMaterial* topMaterial = batchMaterial;
            while (topMaterial->GetParent() && topMaterial->GetParent() != globalMaterial && topMaterial->GetParent() != newGlobalMaterial)
            {
                topMaterial = topMaterial->GetParent();
            }
            topMaterial->SetParent(newGlobalMaterial);

            PrebuildMaterial(batchMaterial);
        }
    }

    SafeRelease(globalMaterial);
    globalMaterial = SafeRetain(newGlobalMaterial);

    UpdateSceneLights();
}

NMaterial* RenderSystem::GetGlobalMaterial() const
{
    return globalMaterial;
}

void RenderSystem::MarkForUpdate(RenderObject* renderObject)
{
    uint32 flags = renderObject->GetFlags();
    if (flags & RenderObject::MARKED_FOR_UPDATE)
        return;

    flags |= RenderObject::NEED_UPDATE;
    if ((flags & RenderObject::CLIPPING_VISIBILITY_CRITERIA) == RenderObject::CLIPPING_VISIBILITY_CRITERIA)
    {
        markedObjects.push_back(renderObject);
        flags |= RenderObject::MARKED_FOR_UPDATE;
    }
    renderObject->SetFlags(flags);
    if (renderObject->GetType() == RenderObject::TYPE_LIGHT)
        lightsToUpdate.push_back(DynamicTypeCheck<Light*>(renderObject));
}

void RenderSystem::RegisterForUpdate(IRenderUpdatable* updatable)
{
    objectsForUpdate.push_back(updatable);
}

void RenderSystem::UnregisterFromUpdate(IRenderUpdatable* updatable)
{
    uint32 size = static_cast<uint32>(objectsForUpdate.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (objectsForUpdate[i] == updatable)
        {
            objectsForUpdate[i] = objectsForUpdate[size - 1];
            objectsForUpdate.pop_back();
            return;
        }
    }
}

void RenderSystem::UpdateNearestLights(RenderObject* renderObject)
{
    uint32 lightIndex = 0;

    if (sunLight != nullptr)
    {
        renderObject->SetLight(lightIndex, sunLight);
        ++lightIndex;
    }

    Light* nearestLight = nullptr;
    Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
    float32 squareMinDistance = std::numeric_limits<float>::max();
    for (Light* light : dynamicLights)
    {
        const Vector3& lightPosition = light->GetPosition();
        float32 squareDistanceToLight = (position - lightPosition).SquareLength();
        if ((light != sunLight) && (squareDistanceToLight < squareMinDistance))
        {
            squareMinDistance = squareDistanceToLight;
            nearestLight = light;
        }
    }

    renderObject->SetLight(lightIndex, nearestLight);
}

void RenderSystem::UpdateSceneLights()
{
    sunLight = nullptr;
    dynamicLights.clear();

    for (Light* light : allLights)
    {
        DVASSERT(light != nullptr, "WTF??? How light could be nullptr here?");

        if (light->GetLightType() == Light::eType::TYPE_SUN)
        {
            sunLight = light;
        }
        else if (light->GetIsDynamic())
        {
            dynamicLights.emplace_back(light);
        }
    }

    for (RenderObject* renderObject : renderObjectArray)
        UpdateNearestLights(renderObject);
}

void RenderSystem::AddLight(Light* light)
{
    allLights.push_back(SafeRetain(light));

    AddRenderObject(light);
    UpdateSceneLights();
}

void RenderSystem::RemoveLight(Light* light)
{
    FindAndRemoveExchangingWithLast(allLights, light);
    UpdateSceneLights();
    RemoveRenderObject(light);
    SafeRelease(light);
}

Vector<Light*>& RenderSystem::GetLights()
{
    return allLights;
}

void RenderSystem::SetForceUpdateLights()
{
    forceUpdateLights = true;
}

void RenderSystem::Update(float32 timeElapsed)
{
    if (!hierarchyInitialized)
    {
        renderHierarchy->Initialize();
        vtDecalManager->Initialize();
        hierarchyInitialized = true;
    }

    for (RenderObject* obj : markedObjects)
    {
        if ((obj->GetType() == RenderObject::TYPE_DECAL) && (static_cast<DecalRenderObject*>(obj)->GetDomain() == DecalRenderObject::DOMAIN_VT))
        {
            //GFX_COMPLETE - think about merging updates through frame
            //update before and after as source and dest should be re-rendered in VT
            AABBox3 prevBox = obj->GetWorldBoundingBox();
            obj->RecalculateWorldBoundingBox();
            vtDecalManager->DecalUpdated(static_cast<DecalRenderObject*>(obj), prevBox);
            if (prevBox.IntersectsWithBox(obj->GetWorldBoundingBox())) //merge
            {
                prevBox.AddAABBox(obj->GetWorldBoundingBox());
                vtDecalManager->InvalidateVTPages(prevBox);
            }
            else
            {
                vtDecalManager->InvalidateVTPages(prevBox);
                vtDecalManager->InvalidateVTPages(obj->GetWorldBoundingBox());
            }
        }
        else
        {
            obj->RecalculateWorldBoundingBox();
            UpdateNearestLights(obj);

            if (obj->GetTreeNodeIndex() != QuadTree::INVALID_TREE_NODE_INDEX)
                renderHierarchy->ObjectUpdated(obj);

            velocityUpdatedObjects.push_back(obj);
            obj->AddFlag(RenderObject::VELOCITY_UPDATE);
        }

        obj->RemoveFlag(RenderObject::NEED_UPDATE | RenderObject::MARKED_FOR_UPDATE);
    }
    markedObjects.clear();

    renderHierarchy->Update();

    if (lightsToUpdate.size() > 0 || forceUpdateLights)
    {
        UpdateSceneLights();

        forceUpdateLights = false;
        lightsToUpdate.clear();
    }

    lightShadowSystem.UpdateLights(this, sunLight, dynamicLights);

    uint32 size = static_cast<uint32>(objectsForUpdate.size());
    for (uint32 i = 0; i < size; ++i)
    {
        objectsForUpdate[i]->RenderUpdate(mainCamera, timeElapsed);
    }
}

void RenderSystem::DebugDrawHierarchy(const Matrix4& cameraMatrix)
{
#if DEBUG_DRAW_SPATIAL_HIERARCHY
    renderHierarchy->DebugDraw(cameraMatrix, GetDebugDrawer());
#endif 
#if DEBUG_DRAW_VT_HIERARCHY
    vtDecalManager->DebugDraw(GetDebugDrawer());
#endif
}

void RenderSystem::ConfigureActivePass()
{
    renderConfig.rescale = false;
    renderConfig.scaledViewport.x = 0;
    renderConfig.scaledViewport.y = 0;

    if (QualitySettingsSystem::Instance()->IsOptionEnabled(QualitySettingsSystem::QUALITY_OPTION_HALF_RESOLUTION_3D))
    {
        renderConfig.rescale = true;
        renderConfig.scaledViewport.dx = floorf(renderConfig.viewport.dx * 0.5f + 0.5f);
        renderConfig.scaledViewport.dy = floorf(renderConfig.viewport.dy * 0.5f + 0.5f);
    }
    else
    {
        renderConfig.scaledViewport.dx = renderConfig.viewport.dx;
        renderConfig.scaledViewport.dy = renderConfig.viewport.dy;
    }

    renderConfig.rescale |= QualitySettingsSystem::Instance()->GetForceRescale() || GetTxaaEnabled();

    if (Renderer::GetCurrentRenderFlow() != currentRenderFlow)
    {
        currentRenderFlow = Renderer::GetCurrentRenderFlow();
        SafeDelete(activeRenderPass);

        switch (currentRenderFlow)
        {
        case RenderFlow::LDRForward:
        {
            activeRenderPass = new LDRForwardPass();
            break;
        }
        case RenderFlow::HDRForward:
        {
            activeRenderPass = new HDRForwardPass(false);
            break;
        }
        case RenderFlow::TileBasedHDRForward:
        {
            activeRenderPass = new HDRForwardPass(true);
            break;
        }
        case RenderFlow::HDRDeferred:
        {
            activeRenderPass = new HDRDeferredPass();
            break;
        }
        case RenderFlow::TileBasedHDRDeferred:
        {
            activeRenderPass = new HDRDeferredFetchPass();
            break;
        }

        default:
            DVASSERT(false, "Invalid RenderFlow specified specified (check your new flow added to selection)");
        }

        activeRenderPass->InvalidateMaterials(); // GFX_COMPLETE - should we?
    }

    DVASSERT(activeRenderPass != nullptr);

    // set active pass configuration
    rhi::RenderPassConfig& config = activeRenderPass->GetPassConfig();

    config.colorBuffer[0].loadAction = renderConfig.loadContent ? rhi::LOADACTION_LOAD : rhi::LOADACTION_CLEAR;
    config.colorBuffer[0].storeAction = (renderConfig.allowAntialiasing) ? rhi::STOREACTION_RESOLVE : rhi::STOREACTION_STORE;
    config.colorBuffer[0].clearColor[0] = renderConfig.clearColor.r;
    config.colorBuffer[0].clearColor[1] = renderConfig.clearColor.g;
    config.colorBuffer[0].clearColor[2] = renderConfig.clearColor.b;
    config.colorBuffer[0].clearColor[3] = renderConfig.clearColor.a;

    config.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    config.depthStencilBuffer.storeAction = renderConfig.storeDepth ? rhi::STOREACTION_STORE : rhi::STOREACTION_NONE;

#if (ENABLE_DEBUG_DRAW)
    if (renderConfig.rescale)
        config.depthStencilBuffer.storeAction = rhi::STOREACTION_STORE;
#endif

    config.priority = renderConfig.basePriority; // +PRIORITY_SERVICE_3D;

    ConfigureFBOs(renderConfig.rescale, GetTxaaEnabled());
    if (renderConfig.rescale)
    {
        uint32 ldrBuffer = (Renderer::GetCurrentRenderFlow() == RenderFlow::TileBasedHDRDeferred) ? 5u : 0u;
        config.colorBuffer[ldrBuffer].texture = Renderer::GetDynamicBindings().GetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_LDR_CURRENT));

        config.depthStencilBuffer.texture = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
        activeRenderPass->SetViewport(renderConfig.scaledViewport);
    }
    else
    {
        activeRenderPass->SetViewport(renderConfig.viewport);

        uint32 ldrBuffer = (Renderer::GetCurrentRenderFlow() == RenderFlow::TileBasedHDRDeferred) ? 5u : 0u;
        config.colorBuffer[ldrBuffer].texture = renderConfig.colorTarget;

        config.depthStencilBuffer.texture = renderConfig.depthTarget;
    }

    activeRenderPass->SetRenderTargetProperties(renderConfig.renderTargetProperties.width, renderConfig.renderTargetProperties.height, renderConfig.renderTargetProperties.format);

    rhi::AntialiasingType aaType = QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Antialiasing>();
    config.antialiasingType = rhi::DeviceCaps().SupportsAntialiasingType(aaType) ? aaType : rhi::AntialiasingType::NONE;
}

Texture::FBODescriptor RenderSystem::GetFBOConfig() const
{
    Size2i sizei = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
    Texture::FBODescriptor config;
    config.width = sizei.dx;
    config.height = sizei.dy;
    config.sampleCount = 1;
    config.textureType = rhi::TEXTURE_TYPE_2D;
    config.needDepth = false;
    config.needPixelReadback = false;
    config.ensurePowerOf2 = false;
    config.format = PixelFormat::FORMAT_RGBA8888;

    return config;
}

void RenderSystem::ConfigureFBOs(bool rescale, bool txaa)
{
    // Set ldr_current and ldr_history here because ConfigureActivePass called after the moment when current and history had been set.
    if (rescale && ldrCurrent == nullptr)
    {
        Texture::FBODescriptor config = GetFBOConfig();
        ldrCurrent = Texture::CreateFBO(config);
        Renderer::GetDynamicBindings().SetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_LDR_CURRENT), ldrCurrent->handle);
    }
    else if (!rescale && ldrCurrent != nullptr)
        SafeRelease(ldrCurrent);

    if (txaa && ldrHistory == nullptr)
    {
        Texture::FBODescriptor config = GetFBOConfig();
        ldrHistory = Texture::CreateFBO(config);
        Renderer::GetDynamicBindings().SetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_LDR_HISTORY), ldrHistory->handle);
    }
    else if (!txaa && ldrHistory != nullptr)
        SafeRelease(ldrHistory);
}

bool RenderSystem::GetTxaaEnabled() const
{
    return (Renderer::GetCurrentRenderFlow() == RenderFlow::HDRDeferred || Renderer::GetCurrentRenderFlow() == RenderFlow::TileBasedHDRDeferred) &&
    (QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Antialiasing>() == rhi::AntialiasingType::TEMPORAL_REPROJECTION);
}

void RenderSystem::Render()
{
    bool taaEnabled = GetTxaaEnabled();

    if (ldrHistory != nullptr)
        Renderer::GetDynamicBindings().SetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_LDR_HISTORY), ldrHistory->handle);
    if (ldrCurrent != nullptr)
        Renderer::GetDynamicBindings().SetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_LDR_CURRENT), ldrCurrent->handle);

    ConfigureActivePass();
    lightShadowSystem.Render(this);

    reflectionRenderer->Draw(mainCamera);
    activeRenderPass->SetEnableFrameJittering(taaEnabled);
    activeRenderPass->Draw(this);

    if (renderConfig.shouldRenderPickingPass == true)
    {
        pickingRenderPass->Draw(this);
    }

#if (ENABLE_DEBUG_DRAW)
    DebugDrawHierarchy(Matrix4::IDENTITY);

    RenderPass debugDrawPass(FastName("Debug"));
    debugDrawPass.passConfig = activeRenderPass->GetPassConfig();
    debugDrawPass.passConfig.priority = renderConfig.basePriority - 4;
    debugDrawPass.passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    debugDrawPass.passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    debugDrawPass.passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_LOAD;
    debugDrawPass.passConfig.depthStencilBuffer.storeAction = renderConfig.storeDepth ? rhi::STOREACTION_STORE : rhi::STOREACTION_NONE;
    debugDrawPass.passConfig.antialiasingType = rhi::AntialiasingType::NONE;

    DAVA_PROFILER_GPU_RENDER_PASS(debugDrawPass.passConfig, ProfilerGPUMarkerName::DEBUG_DRAW);

    if (debugDrawPass.BeginRenderPass(debugDrawPass.passConfig))
    {
        debugDrawPass.SetupCameraParams(mainCamera, drawCamera);
        debugDrawPass.DrawDebug(drawCamera, this);
        debugDrawPass.EndRenderPass();
    }
#endif

    //GFX_COMPLETE - if required use upscale pass
    if (renderConfig.rescale)
    {
        rescalePass->Draw(this);

        if (GetTxaaEnabled())
            std::swap(ldrCurrent, ldrHistory);
    }

#if LOG_CAMERA_POSITION // set 1 to setup camera position and target
    static uint64 t = 1;
    if (t++ % 100 == 0)
    {
        Vector3 cameraPos = mainCamera->GetPosition();
        Vector3 cameraTarget = mainCamera->GetTarget();
        Logger::Debug("cameraPos = DAVA::Vector3(%.3ff, %.3ff, %.3ff); - %f, %f", cameraPos.x, cameraPos.y, cameraPos.z, mainCamera->GetZNear(), mainCamera->GetZNear());
        Logger::Debug("cameraTarget = DAVA::Vector3(%.3ff, %.3ff, %.3ff);", cameraTarget.x, cameraTarget.y, cameraTarget.z);
    }
#endif

    for (RenderObject* ro : velocityUpdatedObjects)
    {
        ro->UpdatePreviousState();
        ro->RemoveFlag(RenderObject::VELOCITY_UPDATE);
    }
    velocityUpdatedObjects.clear();
}

void RenderSystem::SetAntialiasingAllowed(bool allow)
{
    renderConfig.allowAntialiasing = allow;
}

void RenderSystem::SetRenderPickingPass(bool shouldRender)
{
    renderConfig.shouldRenderPickingPass = shouldRender;
}

void RenderSystem::SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, bool loadContent, const Color& clearColor)
{
    renderConfig.colorTarget = color;
    renderConfig.depthTarget = depthStencil;
    renderConfig.loadContent = loadContent;
    renderConfig.clearColor = clearColor;
}

void RenderSystem::SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format)
{
    renderConfig.renderTargetProperties.width = width;
    renderConfig.renderTargetProperties.height = height;
    renderConfig.renderTargetProperties.format = format;

    renderConfig.basePriority = priority;
    renderConfig.viewport = viewport;
}

rhi::RenderPassConfig& RenderSystem::GetMainPassConfig()
{
    return activeRenderPass->GetPassConfig();
}

void RenderSystem::InvalidateMaterials()
{
    // GFX_COMPLETE  - later think about generic mechanism to invalidate materials on shaders reload
    debugDrawer->InvalidateMaterials();
    postEffectRenderer->InvalidateMaterials();
    cubemapRenderer->InvalidateMaterials();
    reflectionRenderer->InvalidateMaterials();
    lightShadowSystem.InvalidateMaterials();

    if (rescalePass)
        rescalePass->InvalidateMaterials();
}
}
