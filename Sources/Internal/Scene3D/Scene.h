#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastTags.h"
#include "Base/Observer.h"
#include "Entity/EntityCache.h"
#include "Entity/SingleComponent.h"
#include "Entity/SystemManager.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
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

class Component;
class EntitiesManager;
class EventSystem;
class RenderPass;
class RenderSystem;
class SceneSystem;
class Texture;
class UIEvent;

/**
    \ingroup scene3d
    \brief This class is a code of our 3D Engine scene graph. 
    To visualize any 3d scene you'll need to create Scene object. 
    Scene have visible hierarchy and invisible root nodes. You can add as many root nodes as you want, and do not visualize them.
    For example you can have multiple scenes, load them to one scene, and show each scene when it will be required. 

    Scene may have scene systems - special classes, marked methods of which will be executed on scene events (update, fixed update, input).
    Scene may have scene systems added by tags or manually added systems, but not both.
    If you want to combine them, you can do:
        ```
        bool exactMatch = true;
        auto systems = systemManager->GetSystems("tag", exactMatch);
        for (const Type *system : systems)
            scene->AddSystemManually(system);
        scene->AddSystemManually(mySystem1);
        scene->AddSystemManually(mySystem2);
        ```

    If you want to switch your scene from tags to manually added systems, you can remove all tags and add your systems on the next update:
        ```
        scene->RemoveTags(scene->GetTags());
        // skip one scene update
        scene->AddSystemManually(mySystem);
        ```
    Other way also works:
        ```
        for (const Type *system : mySystems)
            scene->RemoveSystemManually(system);
        // skip one scene update
        scene->AddTags("tag1", "tag2", ..., "tagN");
        ```
 */

class Scene : public Entity, Observer
{
protected:
    ~Scene() override;

public:
    Scene();
    Scene(const FastTags& tags, bool createSystems = true);

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

    template <class T>
    T* GetSystem();

    /**
        Add tags to scene. All affected systems will be added automatically, but their processes will not be executed on the current update.

        Usage example:
            `scene->AddTags("tag")` or `scene->AddTags(FastName("tag"))` for single tag.
            `scene->AddTags({"tag1", FastName("tag2"), "tag3", "tag4"})` for multiple tags.
    */
    void AddTags(const FastTags& tags);

    /**
        Remove tags from scene. All affected system will be removed automatically. Will take effect on the next frame.

        Usage example:
            `scene->RemoveTags("tag")` or `scene->RemoveTags(FastName("tag"))` for single tag.
            `scene->RemoveTags({"tag1", FastName("tag2"), "tag3", "tag4"})` for multiple tags.
    */
    void RemoveTags(const FastTags& tags);

    /**
        Return `true` if scene tags contain `tags`.

        Usage example:
            `scene->HasTags("tag")` or `scene->HasTags(FastName("tag"))` for single tag.
            `scene->HasTags({"tag1", FastName("tag2"), "tag3", "tag4"})` for multiple tags.
    */
    bool HasTags(const FastTags& tags) const;

    /** Return scene tags. */
    const UnorderedSet<FastName>& GetTags();

    /** Add system to scene manually, system processes will not be executed on the current update. */
    void AddSystemManually(const Type* systemType);

    /** Remove system from scene manually. Will take effect on the next frame. */
    void RemoveSystemManually(const Type* systemType);

    EventSystem* eventSystem = nullptr;
    RenderSystem* renderSystem = nullptr;

    VersionInfo::SceneVersion version;

    /** Get singleton component. Never return nullptr. */
    template <class T>
    T* GetSingleComponent();

    /** Get singleton component. Never return nullptr. */
    SingleComponent* GetSingleComponent(const Type* type);

    template <class T>
    const T* GetSingleComponentForRead(const SceneSystem* user);

    const SingleComponent* GetSingleComponentForRead(const Type* type, const SceneSystem* user);

    template <class T>
    T* GetSingleComponentForWrite(const SceneSystem* user);

    SingleComponent* GetSingleComponentForWrite(const Type* type, const SceneSystem* user);

    /**
        \brief Overloaded GetScene returns this, instead of normal functionality.
     */
    Scene* GetScene() override;

    void HandleEvent(Observable* observable) override; //Handle RenderOptions

    virtual void Update(float32 timeElapsed);
    virtual void Draw();
    void OnSceneLoaded() override;

    Camera* GetCamera(int32 n);
    void AddCamera(Camera* c);
    bool RemoveCamera(Camera* c);
    inline int32 GetCameraCount();

    void SetCurrentCamera(Camera* camera);
    Camera* GetCurrentCamera() const;

    /*
        This camera is used for visualization setup only. Most system functions use mainCamera, draw camera is used to setup matrices for render.
        If you do not call this function GetDrawCamera returns currentCamera. 
        You can use SetCustomDrawCamera function if you want to test frustum clipping, and view the scene from different angles.
    */
    void SetCustomDrawCamera(Camera* camera);
    Camera* GetDrawCamera() const;

    EntitiesManager* GetEntitiesManager() const;

    EventSystem* GetEventSystem() const;
    RenderSystem* GetRenderSystem() const;

    virtual SceneFileV2::eError LoadScene(const FilePath& pathname);
    virtual SceneFileV2::eError SaveScene(const FilePath& pathname, bool saveForGame = false);

    virtual void OptimizeBeforeExport();

    NMaterial* GetGlobalMaterial() const;
    void SetGlobalMaterial(NMaterial* globalMaterial);

    void OnSceneReady(Entity* rootNode);

    void Input(UIEvent* event);
    void InputCancelled(UIEvent* event);

    /**
        \brief This functions activate and deactivate scene systems
     */
    virtual void Activate();
    virtual void Deactivate();

    void SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format);
    void SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, rhi::LoadAction colorLoadAction, const Color& clearColor);

    void SetFixedUpdateTime(float32 time);
    void SetFixedUpdateAdjustment(float32 time);

    float32 GetFixedUpdateTime() const;
    float32 GetFixedUpdateOverlap() const;

    void SetPerformFixedProcessOnlyOnce(bool isPerformFixedProcessOnlyOnce);

    void PauseFixedUpdate();
    void UnpauseFixedUpdate();
    bool IsFixedUpdatePaused() const;

    template <class... Args>
    EntityGroup* AquireEntityGroup();
    template <class Matcher, class... Args>
    EntityGroup* AquireEntityGroupWithMatcher();
    EntityGroupOnAdd* AquireEntityGroupOnAdd(EntityGroup* eg, SceneSystem* sceneSystem);

    template <class T, class... Args>
    ComponentGroup<T>* AquireComponentGroup();
    template <class MaskMatcher, class TypeMatcher, class T, class... Args>
    ComponentGroup<T>* AquireComponentGroupWithMatcher();
    template <class T>
    ComponentGroupOnAdd<T>* AquireComponentGroupOnAdd(ComponentGroup<T>* cg, SceneSystem* sceneSystem);

    const Vector<SceneSystem*>& GetSystems() const;

    Signal<SceneSystem*> systemAdded;
    Signal<SceneSystem*> systemRemoved;

    EntityCache cache;

    // TODO: all the stuff below should be private, but due to some modules design it can't be.

    [[deprecated]] void CreateDelayedSystems();

    /** Clear single components that used only by fixed processes. */
    void ClearFixedProcessesSingleComponents();

    /** Clear single components that used by all processes. */
    void ClearAllProcessesSingleComponents();

    using ProcessSystemPair = std::pair<const SystemManager::SystemProcess*, SceneSystem*>;

    Vector<ProcessSystemPair> processes;
    Vector<ProcessSystemPair> fixedProcesses;
    Vector<ProcessSystemPair> inputProcesses;

protected:
    void RegisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity);
    void RegisterSingleComponentsInSystem(SceneSystem* system);

    uint32 maxEntityIDCounter = 0;

    float32 sceneGlobalTime = 0.f;

    UnorderedMap<const Type*, SceneSystem*> systemsMap;
    Vector<Camera*> cameras;

    NMaterial* sceneGlobalMaterial = nullptr;

    Camera* mainCamera = nullptr;
    Camera* drawCamera = nullptr;

    struct FixedUpdate
    {
        float32 fixedTime = 1.f / 60.f;
        float32 adjustment = 0.f;
        float32 accumulatedTime = 0.f;
        float32 overlap = 0.f;
        bool paused = false;
        bool onlyOnce = false;
        bool firstUpdate = true;
    } fixedUpdate;

private:
    void AddSystem(const Type* systemType);

    void RemoveSystem(const Type* systemType);

    void CorrectSystemsList(const UnorderedSet<const Type*>& systems);

    void ExecuteFixedProcesses(float32 timeElapsed);

    void ExecuteProcesses(float32 timeElapsed);

    void AddSingleComponent(SingleComponent* component, const Type* type);

    void OnNewSystemRegistered(const Type* systemType, const SystemManager::SystemInfo* systemInfo);

    void QueryRemovedSystems();

private:
    UnorderedSet<FastName> sceneTags;

    EntitiesManager* entitiesManager = nullptr;
    const SystemManager* systemManager = nullptr;

    UnorderedSet<FastName> tagsToRemove;

    UnorderedSet<const Type*> systemsToRemove;

    Vector<SceneSystem*> systemsVector;

    UnorderedMap<const Type*, SingleComponent*> singleComponents;
    Vector<ClearableSingleComponent*> clearableSingleComponentsAll; // Vector of single components to be cleared after all processes
    Vector<ClearableSingleComponent*> clearableSingleComponentsFixed; // Vector of single components to be cleared after fixed processes

#if defined(__DAVAENGINE_DEBUG__)
    // For validating usages of single components.

    UnorderedMap<const Type* /* single component type */, UnorderedSet<const SceneSystem*>> clearableSingleComponentsReaders;
    UnorderedMap<const Type* /* single component type */, UnorderedSet<const SceneSystem*>> clearableSingleComponentsWriters;

    void HandleSingleComponentRead(const SceneSystem* system, const Type* singleComponentType);
    void HandleSingleComponentWrite(const SceneSystem* system, const Type* singleComponentType);

    void UnregisterSingleComponentUser(const SceneSystem* system);

    void ValidateSingleComponentsAccessOrder();

    bool pendingSingleComponentsAccessValidation = false;
#endif

    friend class SnapshotSystemBase;
    friend class SnapshotSystem2;
    friend class NetworkDeltaReplicationSystemServer;

    friend class Entity;
    DAVA_VIRTUAL_REFLECTION(Scene, Entity);
};
} // namespace DAVA

#include "Scene3D/Private/Scene.hpp"
