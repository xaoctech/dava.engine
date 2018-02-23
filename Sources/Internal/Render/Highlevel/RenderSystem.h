#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Render/Highlevel/LightShadowSystem.h"
#include "Render/Highlevel/IRenderUpdatable.h"
#include "Render/Highlevel/VisibilityQuadTree.h"
#include "Render/Highlevel/GeoDecalManager.h"
#include "Render/RenderHelper.h"

namespace DAVA
{
class RenderPass;
class LDRForwardPass;
class HDRForwardPass;
class HDRDeferredPass;
class HDRDeferredFetchPass;
class PickingRenderPass;
class RescalePass;
class MainShadowPass;
class RenderLayer;
class RenderObject;
class RenderBatch;
class Entity;
class Camera;
class Light;
class Landscape;
class ParticleEmitterSystem;
class RenderHierarchy;
class NMaterial;
class PostEffectRenderer;
class ReflectionRenderer;
class CubemapRenderer;
class VTDecalManager;

//GFX_COMPLETE - temporary here
struct RenderSystemConfiguration
{
    RenderTargetProperites renderTargetProperties;
    rhi::HTexture colorTarget;
    rhi::HTexture depthTarget;
    Rect viewport;
    Rect scaledViewport;
    int32 basePriority = 0;
    Color clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
    bool allowAntialiasing = false;
    bool loadContent = false;
    bool storeDepth = false;
    bool rescale = false; //upscale or DRR
    bool forceRescale = false; //used to apply rescale even in 1 to 1 case - RE for debug draw
    bool shouldRenderPickingPass = false; // is renderSystem should render a PickingPass
};

class RenderSystem
{
public:
    RenderSystem();
    virtual ~RenderSystem();

    const RenderSystemConfiguration& GetConfiguration() const;

    /**
        \brief Register render objects for permanent rendering
     */
    void RenderPermanent(RenderObject* renderObject);

    /**
        \brief Unregister render objects for permanent rendering
     */
    void RemoveFromRender(RenderObject* renderObject);

    /**
        \brief Register batch
     */
    void RegisterBatch(RenderBatch* batch);
    /**
        \brief Unregister batch
     */
    void UnregisterBatch(RenderBatch* batch);

    void RegisterMaterial(NMaterial* material);
    void UnregisterMaterial(NMaterial* material);

    void PrepareForShutdown();

    /**
        \brief Set main camera
     */
    void SetMainCamera(Camera* camera);
    Camera* GetMainCamera() const;
    void SetDrawCamera(Camera* camera);
    Camera* GetDrawCamera() const;

    void SetGlobalMaterial(NMaterial* material);
    NMaterial* GetGlobalMaterial() const;

    void Update(float32 timeElapsed);
    void Render();

    void MarkForUpdate(RenderObject* renderObject);

    /**
        \brief This is required for objects that needs permanent update every frame like
        Landscape and Particles.
     */
    void RegisterForUpdate(IRenderUpdatable* renderObject);
    void UnregisterFromUpdate(IRenderUpdatable* renderObject);

    void AddLight(Light* light);
    void RemoveLight(Light* light);
    Vector<Light*>& GetLights();
    void SetForceUpdateLights();
    void UpdateNearestLights(RenderObject* renderObject);

    void SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, bool loadContent, const Color& clearColor);
    void SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format);
    void SetAntialiasingAllowed(bool allowed);
    void SetRenderPickingPass(bool shouldRender);

    void DebugDrawHierarchy(const Matrix4& cameraMatrix);

    bool IsRenderHierarchyInitialized() const;

    /**
        \brief Get Render Hierarchy. It allow you to work with current render hierarchy and perform all main tasks with geometry on the level.
     */
    RenderHierarchy* GetRenderHierarchy() const;
    VTDecalManager* GetVTDecalManager() const;
    RenderHelper* GetDebugDrawer() const;
    ReflectionRenderer* GetReflectionRenderer() const;
    CubemapRenderer* GetCubemapRenderer() const;
    PostEffectRenderer* GetPostEffectRenderer() const;

    void InvalidateMaterials();

    Vector<RenderObject*>& GetRenderObjectArray();

    const Vector<Light*> GetDynamicLights() const
    {
        return dynamicLights;
    }

    const Vector<Landscape*> GetLandscapes() const
    {
        return allLandscapes;
    }

    inline GeoDecalManager* GetGeoDecalManager() const
    {
        return geoDecalManager;
    }

public:
    DAVA_DEPRECATED(rhi::RenderPassConfig& GetMainPassConfig());

private:
    void UpdateSceneLights();

    void FindNearestLights(RenderObject* renderObject);
    void AddRenderObject(RenderObject* renderObject);
    void RemoveRenderObject(RenderObject* renderObject);
    void PrebuildMaterial(NMaterial* material);

    void ConfigureActivePass();

private:
    friend class RenderPass;

    Vector<IRenderUpdatable*> objectsForUpdate;
    Vector<RenderObject*> objectsForPermanentUpdate;
    Vector<RenderObject*> markedObjects;
    Vector<RenderObject*> renderObjectArray;

    Vector<Light*> allLights;
    Vector<Light*> dynamicLights;
    Vector<Light*> lightsToUpdate;
    Vector<Light*> lightsProducingShadow;
    Light* sunLight = nullptr;

    Vector<Landscape*> allLandscapes; // GFX_COMPLETE GFX_COMPLETE GFX_COMPLETE (maximum 3 landscapes)

    RenderPass* activeRenderPass = nullptr;
    PickingRenderPass* pickingRenderPass = nullptr;
    RenderSystemConfiguration renderConfig;
    RenderFlow currentRenderFlow = RenderFlow::Undefined;

    LightShadowSystem lightShadowSystem;

    RenderHierarchy* renderHierarchy = nullptr;
    VTDecalManager* vtDecalManager = nullptr;

    Camera* mainCamera = nullptr;
    Camera* drawCamera = nullptr;
    NMaterial* globalMaterial = nullptr;
    RenderHelper* debugDrawer = nullptr;
    PostEffectRenderer* postEffectRenderer = nullptr;
    ReflectionRenderer* reflectionRenderer = nullptr;
    CubemapRenderer* cubemapRenderer = nullptr;
    GeoDecalManager* geoDecalManager = nullptr;
    RescalePass* rescalePass = nullptr;

    bool hierarchyInitialized = false;
    bool forceUpdateLights = false;
};

inline void RenderSystem::SetMainCamera(Camera* _camera)
{
    SafeRelease(mainCamera);
    mainCamera = SafeRetain(_camera);
}

inline void RenderSystem::SetDrawCamera(Camera* _camera)
{
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

inline Camera* RenderSystem::GetMainCamera() const
{
    return mainCamera;
}

inline Camera* RenderSystem::GetDrawCamera() const
{
    return drawCamera;
}

inline RenderHierarchy* RenderSystem::GetRenderHierarchy() const
{
    return renderHierarchy;
}

inline VTDecalManager* RenderSystem::GetVTDecalManager() const
{
    return vtDecalManager;
}

inline bool RenderSystem::IsRenderHierarchyInitialized() const
{
    return hierarchyInitialized;
}

inline RenderHelper* RenderSystem::GetDebugDrawer() const
{
    return debugDrawer;
}

inline ReflectionRenderer* RenderSystem::GetReflectionRenderer() const
{
    return reflectionRenderer;
}

inline CubemapRenderer* RenderSystem::GetCubemapRenderer() const
{
    return cubemapRenderer;
}

inline PostEffectRenderer* RenderSystem::GetPostEffectRenderer() const
{
    return postEffectRenderer;
}

inline Vector<RenderObject*>& RenderSystem::GetRenderObjectArray()
{
    return renderObjectArray;
}

inline const RenderSystemConfiguration& RenderSystem::GetConfiguration() const
{
    return renderConfig;
}
}
