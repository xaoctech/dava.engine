#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/Observer.h"
#include "Entity/SceneSystem.h"
#include "Entity/SingletonComponent.h"
#include "Entity/SystemManager.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Render/RenderBase.h"
#include "Scene3D/ComponentGroup.h"
#include "Scene3D/Entity.h"
#include "Scene3D/EntityGroup.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Scene3D/SceneFileV2.h"

namespace DAVA
{
/**
    \defgroup scene3d 3D Engine
  */

class Texture;
class StaticMesh;
class DataNode;
class ShadowVolumeNode;
class Light;
class ShadowRect;
class QuadTree;
class Component;
class RenderSystem;
class RenderUpdateSystem;
class TransformSystem;
class DebugRenderSystem;
class EventSystem;
class ParticleEffectSystem;
class UpdateSystem;
class LightUpdateSystem;
class SwitchSystem;
class SoundUpdateSystem;
class ActionUpdateSystem;
class StaticOcclusionSystem;
class StaticOcclusionDebugDrawSystem;
class SpeedTreeUpdateSystem;
class FoliageSystem;
class WindSystem;
class WaveSystem;
class SkeletonSystem;
class MotionSystem;
class AnimationSystem;
class LandscapeSystem;
class LodSystem;
class ParticleEffectDebugDrawSystem;
class GeoDecalSystem;
class SlotSystem;
class TransformSingleComponent;
class ActionsSingleComponent;
class ActionCollectSystem;
class DiffMonitoringSystem;
class MotionSingleComponent;
class PhysicsSystem;
class CollisionSingleComponent;
class EntitiesManager;

class UIEvent;
class RenderPass;

/**
    \ingroup scene3d
    \brief This class is a code of our 3D Engine scene graph. 
    To visualize any 3d scene you'll need to create Scene object. 
    Scene have visible hierarchy and invisible root nodes. You can add as many root nodes as you want, and do not visualize them.
    For example you can have multiple scenes, load them to one scene, and show each scene when it will be required. 
 */
class EntityCache
{
public:
    ~EntityCache();

    void Preload(const FilePath& path);
    void Clear(const FilePath& path);
    void ClearAll();

    Entity* GetOriginal(const FilePath& path);
    Entity* GetClone(const FilePath& path);

protected:
    Map<FilePath, Entity*> cachedEntities;
};

class Scene : public Entity, Observer
{
protected:
    virtual ~Scene();

public:
    enum : uint32
    {
        SCENE_SYSTEM_TRANSFORM_FLAG = 1 << 0,
        SCENE_SYSTEM_RENDER_UPDATE_FLAG = 1 << 1,
        SCENE_SYSTEM_LOD_FLAG = 1 << 2,
        SCENE_SYSTEM_DEBUG_RENDER_FLAG = 1 << 3,
        SCENE_SYSTEM_PARTICLE_EFFECT_FLAG = 1 << 4,
        SCENE_SYSTEM_UPDATEBLE_FLAG = 1 << 5,
        SCENE_SYSTEM_LIGHT_UPDATE_FLAG = 1 << 6,
        SCENE_SYSTEM_SWITCH_FLAG = 1 << 7,
        SCENE_SYSTEM_SOUND_UPDATE_FLAG = 1 << 8,
        SCENE_SYSTEM_ACTION_UPDATE_FLAG = 1 << 9,
        SCENE_SYSTEM_STATIC_OCCLUSION_FLAG = 1 << 11,
        SCENE_SYSTEM_LANDSCAPE_FLAG = 1 << 12,
        SCENE_SYSTEM_FOLIAGE_FLAG = 1 << 13,
        SCENE_SYSTEM_SPEEDTREE_UPDATE_FLAG = 1 << 14,
        SCENE_SYSTEM_WIND_UPDATE_FLAG = 1 << 15,
        SCENE_SYSTEM_WAVE_UPDATE_FLAG = 1 << 16,
        SCENE_SYSTEM_SKELETON_FLAG = 1 << 17,
        SCENE_SYSTEM_ANIMATION_FLAG = 1 << 18,
        SCENE_SYSTEM_SLOT_FLAG = 1 << 19,
        SCENE_SYSTEM_GEO_DECAL_FLAG = 1 << 20,
        SCENE_SYSTEM_SHOOT_FLAG = 1 << 21,
        SCENE_SYSTEM_ACTION_COLLECT_FLAG = 1 << 22,
        SCENE_SYSTEM_DIFF_MONITORING_FLAG = 1 << 23,
        SCENE_SYSTEM_MOTION_FLAG = 1 << 24,

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
        SCENE_SYSTEM_PHYSICS_FLAG = 1 << 19,
#endif
        SCENE_SYSTEM_ALL_MASK = 0xFFFFFFFF
    };

    Scene(uint32 systemsMask = SCENE_SYSTEM_ALL_MASK);
    Scene(const UnorderedSet<FastName>& tags);

    /**
        \brief Function to register entity in scene. This function is called when you add entity to scene.
     */
    void RegisterEntity(Entity* entity);
    /**
        \brief Function to unregister entity from scene. This function is called when you remove entity from scene.
     */
    void UnregisterEntity(Entity* entity);

    /**
        \brief Function to register component in scene. This function is called when you add any component to any entity in scene.
     */
    void RegisterComponent(Entity* entity, Component* component);
    /**
        \brief Function to unregister component from scene. This function is called when you remove any component from any entity in scene.
     */
    void UnregisterComponent(Entity* entity, Component* component);

    virtual void AddSystem(SceneSystem* sceneSystem, SceneSystem* insertBeforeSceneForProcess = nullptr, SceneSystem* insertBeforeSceneForInput = nullptr, SceneSystem* insertBeforeSceneForFixedProcess = nullptr);

    virtual void RemoveSystem(SceneSystem* sceneSystem);

    template <class T>
    T* GetSystem();

    /** Take effect on the next frame. Add tag to scene. All affected systems will be added automatically. */
    void AddTag(FastName tag);

    /** Take effect on the next frame. Remove tag from scene. All affected system will be removed automatically. */
    void RemoveTag(FastName tag);

    bool HasTag(FastName tag) const;

    template <typename Return, typename Cls>
    void RegisterSystemProcess(Return (Cls::*fp)(float32));

    Vector<SceneSystem*> systemsToProcess;
    Vector<SceneSystem*> systemsToInput;
    Vector<SceneSystem*> systemsToFixedProcess;

    TransformSystem* transformSystem = nullptr;
    RenderUpdateSystem* renderUpdateSystem = nullptr;
    LodSystem* lodSystem = nullptr;
    DebugRenderSystem* debugRenderSystem = nullptr;
    EventSystem* eventSystem = nullptr;
    ParticleEffectSystem* particleEffectSystem = nullptr;
    UpdateSystem* updatableSystem = nullptr;
    LightUpdateSystem* lightUpdateSystem = nullptr;
    SwitchSystem* switchSystem = nullptr;
    RenderSystem* renderSystem = nullptr;
    SoundUpdateSystem* soundSystem = nullptr;
    ActionUpdateSystem* actionSystem = nullptr;
    StaticOcclusionSystem* staticOcclusionSystem = nullptr;
    SpeedTreeUpdateSystem* speedTreeUpdateSystem = nullptr;
    FoliageSystem* foliageSystem = nullptr;
    VersionInfo::SceneVersion version;
    WindSystem* windSystem = nullptr;
    WaveSystem* waveSystem = nullptr;
    AnimationSystem* animationSystem = nullptr;
    StaticOcclusionDebugDrawSystem* staticOcclusionDebugDrawSystem = nullptr;
    SkeletonSystem* skeletonSystem = nullptr;
    MotionSystem* motionSystem = nullptr;
    LandscapeSystem* landscapeSystem = nullptr;
    ParticleEffectDebugDrawSystem* particleEffectDebugDrawSystem = nullptr;
    SlotSystem* slotSystem = nullptr;
    ActionCollectSystem* actionCollectSystem;
    GeoDecalSystem* geoDecalSystem = nullptr;
    DiffMonitoringSystem* diffMonitoringSystem = nullptr;
    PhysicsSystem* physicsSystem = nullptr;

    template <class T>
    const T* AquireSingleComponentForRead();
    const SingletonComponent* AquireSingleComponentForRead(const Type* type);
    template <class T>
    T* AquireSingleComponentForWrite();
    SingletonComponent* AquireSingleComponentForWrite(const Type* type);

    /** Get singleton component. Never return nullptr. */
    template <class T>
    T* GetSingletonComponent();

    /** Get singleton component. Never return nullptr. */
    SingletonComponent* GetSingletonComponent(const Type* type);

    /**
        \brief Overloaded GetScene returns this, instead of normal functionality.
     */
    Scene* GetScene() override;

    void HandleEvent(Observable* observable) override; //Handle RenderOptions

    //virtual void StopAllAnimations(bool recursive = true);

    virtual void Update(float32 timeElapsed);
    virtual void Draw();
    void SceneDidLoaded() override;

    Camera* GetCamera(int32 n);
    void AddCamera(Camera* c);
    bool RemoveCamera(Camera* c);
    inline int32 GetCameraCount();

    void SetCurrentCamera(Camera* camera);
    Camera* GetCurrentCamera() const;

    /* 
        This camera is used for visualization setup only. Most system functions use mainCamere, draw camera is used to setup matrices for render. If you do not call this function GetDrawCamera returns currentCamera. 
        You can use SetCustomDrawCamera function if you want to test frustum clipping, and view the scene from different angles.
     */
    void SetCustomDrawCamera(Camera* camera);
    Camera* GetDrawCamera() const;

    void CreateSystems();

    EventSystem* GetEventSystem() const;
    RenderSystem* GetRenderSystem() const;
    AnimationSystem* GetAnimationSystem() const;
    ParticleEffectDebugDrawSystem* GetParticleEffectDebugDrawSystem() const;
#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    PhysicsSystem* GetPhysicsSystem() const;
#endif

    virtual SceneFileV2::eError LoadScene(const DAVA::FilePath& pathname);
    virtual SceneFileV2::eError SaveScene(const DAVA::FilePath& pathname, bool saveForGame = false);

    virtual void OptimizeBeforeExport();

    DAVA::NMaterial* GetGlobalMaterial() const;
    void SetGlobalMaterial(DAVA::NMaterial* globalMaterial);

    void OnSceneReady(Entity* rootNode);

    void Input(UIEvent* event);
    void InputCancelled(UIEvent* event);

    /**
        \brief This functions activate and deactivate scene systems
     */
    virtual void Activate();
    virtual void Deactivate();

    EntityCache cache;

    void SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format);
    void SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, rhi::LoadAction colorLoadAction, const Color& clearColor);

    void SetFixedUpdateTime(float32 time);
    void SetConstantUpdateTime(float32 time);

    void SetPerformFixedProcessOnlyOnce(bool isPerformFixedProcessOnlyOnce);

    void PauseFixedUpdate();
    void UnpauseFixedUpdate();
    bool IsFixedUpdatePaused() const;
    void CreateSystemsByTags();
    Vector<SceneSystem*> systemsVector;

    template <class... Args>
    EntityGroup* AquireEntityGroup();
    template <class Matcher, class... Args>
    EntityGroup* AquireEntityGroupWithMatcher();

    template <class T, class... Args>
    ComponentGroup<T>* AquireComponentGroup();
    template <class Matcher, class T, class... Args>
    ComponentGroup<T>* AquireComponentGroupWithMatcher();

public: // deprecated methods
    DAVA_DEPRECATED(rhi::RenderPassConfig& GetMainPassConfig());

protected:
    void RegisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity);

    bool RemoveSystem(Vector<SceneSystem*>& storage, SceneSystem* system);

    uint32 systemsMask;
    uint32 maxEntityIDCounter;

    float32 sceneGlobalTime = 0.f;

    Vector<Camera*> cameras;

    NMaterial* sceneGlobalMaterial;

    Camera* mainCamera;
    Camera* drawCamera;

    struct FixedUpdate
    {
        float32 constantTime = 1.f / 60.f;
        float32 fixedTime = 1.f / 60.f;
        float32 lastTime = 0.f;
    } fixedUpdate;
    bool pauseFixedUpdate = false;

    bool isPerformFixedProcessOnlyOnce = false;

    friend class Entity;
    DAVA_VIRTUAL_REFLECTION(Scene, Entity);

private:
    void ProcessManuallyAddedSystems(float32 timeElapsed);
    void ProcessSystemsAddedByTags(float32 timeElapsed);
    void CreateSystemsToMethods(const Vector<SystemManager::SceneProcessInfo>& methods);

    void InitLegacyPointers();
    void ProcessChangedTags();

    template <typename T>
    void AddSingletonComponent(T* component);
    void AddSingletonComponent(SingletonComponent* component, const Type* type);

    UnorderedSet<FastName> tags;

    enum TagAction
    {
        ADD,
        REMOVE
    };
    Vector<std::pair<FastName, TagAction>> tagsToChange;

    UnorderedMap<const Type*, SceneSystem*> systemsMap;

    Vector<Function<void(float32)>> systemProcesses;

    EntitiesManager* entitiesManager = nullptr;

    UnorderedMap<const Type*, SingletonComponent*> singletonComponents;

    friend class SnapshotSystemBase;
    friend class NetworkDeltaReplicationSystemServer;
};
}

#include "Scene3D/Private/Scene.hpp"
