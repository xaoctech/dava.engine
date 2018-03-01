#include "Scene3D/Scene.h"

#include "Asset/AbstractAssetLoader.h"
#include "Concurrency/Thread.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Entity/ComponentUtils.h"
#include "FileSystem/FileSystem.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Image/Image.h"
#include "Render/MipmapReplacer.h"
#include "Render/RenderOptions.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Components/SingleComponents/ChangedSystemsSingleComponent.h"
#include "Scene3D/Components/SingleComponents/VTSingleComponent.h"
#include "Scene3D/Components/SingleComponents/LightmapSingleComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/DecalComponent.h"
#include "Scene3D/Components/EnvironmentComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/PostEffectComponent.h"
#include "Scene3D/Components/PostEffectDebugComponent.h"
#include "Scene3D/Components/ReflectionComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/SlotComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/VTDecalComponent.h"
#include "Scene3D/Components/SplineComponent.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/Components/LandscapeComponent.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Scene3D/Lod/LodSystem.h"
#include "Scene3D/Private/EntitiesManager.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Systems/ActionUpdateSystem.h"
#include "Scene3D/Systems/AnimationSystem.h"
#include "Scene3D/Systems/DebugRenderSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Scene3D/Systems/GeoDecalSystem.h"
#include "Scene3D/Systems/LandscapeSystem.h"
#include "Scene3D/Systems/LightUpdateSystem.h"
#include "Scene3D/Systems/MotionSystem.h"
#include "Scene3D/Systems/ParticleEffectDebugDrawSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Systems/RenderUpdateSystem.h"
#include "Scene3D/Systems/Controller/RotationControllerSystem.h"
#include "Scene3D/Systems/SkeletonSystem.h"
#include "Scene3D/Systems/AnimationSystem.h"
#include "Scene3D/Systems/LandscapeSystem.h"
#include "Scene3D/Systems/PostEffectSystem.h"
#include "Scene3D/Systems/PostEffectDebugSystem.h"
#include "Scene3D/Systems/DecalSystem.h"
#include "Scene3D/Systems/VTDecalSystem.h"
#include "Scene3D/Systems/VTSplineDecalSystem.h"
#include "Scene3D/Systems/SlotSystem.h"
#include "Scene3D/Systems/SoundUpdateSystem.h"
#include "Scene3D/Systems/ParticleEffectDebugDrawSystem.h"
#include "Scene3D/Systems/GeoDecalSystem.h"
#include "Scene3D/Systems/SlotSystem.h"
#include "Scene3D/Systems/ActionCollectSystem.h"
#include "Scene3D/Systems/EnvironmentSystem.h"

#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Components/SingleComponents/ActionsSingleComponent.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Concurrency/Thread.h"

#include "Sound/SoundSystem.h"

#include "Scene3D/Systems/SpeedTreeUpdateSystem.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"
#include "Scene3D/Systems/SwitchSystem.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Systems/UpdateSystem.h"
#include "Scene3D/Systems/WaveSystem.h"
#include "Scene3D/Systems/WindSystem.h"
#include "Scene3D/Systems/StaticLightingSystem.h"
#include "Scene3D/Systems/StreamingSystem.h"
#include "Engine/Engine.h"
#include "Sound/SoundSystem.h"
#include "Time/SystemTimer.h"
#include "UI/UIEvent.h"
#include "Render/Renderer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Scene3D/Systems/ReflectionSystem.h"
#include "Scene3D/Systems/LightRenderSystem.h"
#include "Utils/Utils.h"
#include "Entity/SystemManager.h"
#include "Engine/EngineContext.h"
#include "Logger/Logger.h"

#if defined(__DAVAENGINE_PHYSICS_DEBUG_DRAW_ENABLED__)
#include "PhysicsDebug/PhysicsDebugDrawSystem.h"
#endif

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
#include <Physics/WASDPhysicsControllerSystem.h>
#include <Physics/PhysicsSystem.h>
#include <Physics/CollisionSingleComponent.h>
#include <Physics/ConvexHullShapeComponent.h>
#endif

#include <functional>
#include "Asset/AssetManager.h"

namespace DAVA
{
//TODO: remove this crap with shadow color
EntityCache::~EntityCache()
{
    ClearAll();
}

void EntityCache::Preload(const FilePath& path)
{
    Scene* scene = new Scene(Scene::SceneSystemsPolicy::WithoutSystems);
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(path))
    {
        Entity* srcRootEntity = scene;

        // try to perform little optimization:
        // if scene has single node with identity transform
        // we can skip this entity and move only its children
        if (1 == srcRootEntity->GetChildrenCount())
        {
            Entity* child = srcRootEntity->GetChild(0);
            if (1 == child->GetComponentCount())
            {
                TransformComponent* tr = srcRootEntity->GetComponent<TransformComponent>();
                if (nullptr != tr && tr->GetLocalTransform() == Matrix4::IDENTITY)
                {
                    srcRootEntity = child;
                }
            }
        }

        auto count = srcRootEntity->GetChildrenCount();

        Vector<Entity*> tempV;
        tempV.reserve(count);
        for (auto i = 0; i < count; ++i)
        {
            tempV.push_back(srcRootEntity->GetChild(i));
        }

        Entity* dstRootEntity = new Entity();
        for (auto i = 0; i < count; ++i)
        {
            dstRootEntity->AddNode(tempV[i]);
        }

        dstRootEntity->ResetID();
        dstRootEntity->SetName(scene->GetName());
        cachedEntities[path] = dstRootEntity;
    }

    SafeRelease(scene);
}

Entity* EntityCache::GetOriginal(const FilePath& path)
{
    Entity* ret = nullptr;

    if (cachedEntities.find(path) == cachedEntities.end())
    {
        Preload(path);
    }

    auto i = cachedEntities.find(path);
    if (i != cachedEntities.end())
    {
        ret = i->second;
    }

    return ret;
}

Entity* EntityCache::GetClone(const FilePath& path)
{
    Entity* ret = nullptr;

    Entity* orig = GetOriginal(path);
    if (nullptr != orig)
    {
        ret = orig->Clone();
    }

    return ret;
}

void EntityCache::Clear(const FilePath& path)
{
    auto i = cachedEntities.find(path);
    if (i != cachedEntities.end())
    {
        SafeRelease(i->second);
        cachedEntities.erase(i);
    }
}

void EntityCache::ClearAll()
{
    for (auto& i : cachedEntities)
    {
        SafeRelease(i.second);
    }
    cachedEntities.clear();
}

DAVA_VIRTUAL_REFLECTION_IMPL(Scene)
{
    ReflectionRegistrator<Scene>::Begin()
    .End();
}

Scene::Scene()
    : Scene(SceneSystemsPolicy::DefaultSystems)
{
}

Scene::Scene(SceneSystemsPolicy policy)
    : Entity()
    , maxEntityIDCounter(0)
    , sceneGlobalMaterial(0)
    , mainCamera(0)
    , drawCamera(0)
    , entitiesManager(new EntitiesManager)
{
    static uint32 idCounter = 0;
    sceneId = ++idCounter;

    renderSystem = new RenderSystem();
    eventSystem = new EventSystem();

    if (policy != SceneSystemsPolicy::WithoutSystems)
    {
        CreateSystems();
    }

    // this will force scene to create hidden global material
    SetGlobalMaterial(nullptr);

    RenderOptions* options = Renderer::GetOptions();
    options->AddObserver(this);
}

Scene::Scene(const UnorderedSet<FastName>& tags_)
    : Scene()
{
    tags.insert(FastName("base"));

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    tags.insert(FastName("physics"));
#endif

    tags.insert(tags_.begin(), tags_.end());
}

NMaterial* Scene::GetGlobalMaterial() const
{
    return sceneGlobalMaterial;
}

void Scene::SetGlobalMaterial(NMaterial* globalMaterial)
{
    SafeRelease(sceneGlobalMaterial);
    sceneGlobalMaterial = SafeRetain(globalMaterial);

    renderSystem->SetGlobalMaterial(sceneGlobalMaterial);

    if (nullptr != particleEffectSystem)
        particleEffectSystem->SetGlobalMaterial(sceneGlobalMaterial);
}

void Scene::SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format)
{
    renderSystem->SetMainPassProperties(priority, viewport, width, height, format);
}

void Scene::SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, bool loadContent, const Color& clearColor)
{
    renderSystem->SetMainRenderTarget(color, depthStencil, loadContent, clearColor);
}

void Scene::SetFixedUpdateTime(float32 time)
{
    fixedUpdate.fixedTime = time;
}

void Scene::SetConstantUpdateTime(float32 time)
{
    fixedUpdate.constantTime = time;
}

void Scene::SetPerformFixedProcessOnlyOnce(bool isPerformFixedProcessOnlyOnce_)
{
    isPerformFixedProcessOnlyOnce = isPerformFixedProcessOnlyOnce_;
}

void Scene::CreateSystems()
{
    if (!tags.empty())
    {
        DVASSERT(tags.empty());
        return;
    }

    // TODO UVR
    //AddSystem(new StreamingSystem(this));
    AddSystem(new ActionCollectSystem(this));
    AddSystem(new StaticOcclusionSystem(this));
    AddSystem(new AnimationSystem(this));
    AddSystem(new EnvironmentSystem(this));
    AddSystem(new StaticLightingSystem(this));
    AddSystem(new MotionSystem(this));

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    AddSystem(new WASDPhysicsControllerSystem(this));
    AddSystem(new PhysicsSystem(this));
#if defined(__DAVAENGINE_PHYSICS_DEBUG_DRAW_ENABLED__)
    AddSystem(new PhysicsDebugDrawSystem(this));
#endif
#endif

    AddSystem(new SkeletonSystem(this));
    AddSystem(new SlotSystem(this));
    AddSystem(new TransformSystem(this));
    AddSystem(new LodSystem(this));
    AddSystem(new SwitchSystem(this));
    AddSystem(new ParticleEffectSystem(this));
    AddSystem(new SoundUpdateSystem(this));
    AddSystem(new StaticOcclusionDebugDrawSystem(this));
    AddSystem(new RenderUpdateSystem(this));
    AddSystem(new UpdateSystem(this));
    AddSystem(new LightUpdateSystem(this));
    AddSystem(new ActionUpdateSystem(this));
    AddSystem(new DebugRenderSystem(this));
    AddSystem(new LandscapeSystem(this));
    AddSystem(new FoliageSystem(this));
    AddSystem(new SpeedTreeUpdateSystem(this));
    AddSystem(new WindSystem(this));
    AddSystem(new WaveSystem(this));

    postEffectSystem = new PostEffectSystem(this);
    AddSystem(postEffectSystem);

    postEffectDebugSystem = new PostEffectDebugSystem(this);
    AddSystem(postEffectDebugSystem);

    reflectionSystem = new ReflectionSystem(this);
    AddSystem(reflectionSystem);

    lightRenderSystem = new LightRenderSystem(this);
    AddSystem(lightRenderSystem);

    decalSystem = new DecalSystem(this);
    AddSystem(decalSystem);

    vtDecalSystem = new VTDecalSystem(this);
    AddSystem(vtDecalSystem);

    vtSplineDecalSystem = new VTSplineDecalSystem(this);
    AddSystem(vtSplineDecalSystem);

    AddSystem(new GeoDecalSystem(this));
    //AddSystem(new ParticleEffectDebugDrawSystem(this));

    InitLegacyPointers();
}

Scene::~Scene()
{
    entitiesManager->UpdateCaches();

    Renderer::GetOptions()->RemoveObserver(this);

    for (Camera*& c : cameras)
        SafeRelease(c);
    cameras.clear();

    renderSystem->PrepareForShutdown();

    size_t size = systemsVector.size();

    for (size_t k = 0; k < size; ++k)
    {
        systemsVector[k]->PrepareForRemove();
    }

    for (size_t k = 0; k < size; ++k)
    {
        SafeDelete(systemsVector[k]);
    }
    systemsVector.clear();
    systemsMap.clear();

    // Reinit with nullptrs
    InitLegacyPointers();

    SafeRelease(mainCamera);
    SafeRelease(drawCamera);
    for (Camera*& c : cameras)
        SafeRelease(c);
    cameras.clear();

    RemoveAllChildren();
    SafeRelease(sceneGlobalMaterial);

    for (auto& pair : singletonComponents)
    {
        SafeDelete(pair.first);
    }

    singletonComponents.clear();

    systemsToProcess.clear();
    systemsToInput.clear();
    systemsToFixedProcess.clear();
    cache.ClearAll();

    SafeDelete(eventSystem);
    SafeDelete(renderSystem);
    SafeDelete(entitiesManager);
}

void Scene::RegisterEntity(Entity* entity)
{
    if (entity->GetID() == 0 ||
        entity->GetSceneID() == 0 ||
        entity->GetSceneID() != sceneId)
    {
        entity->SetID(++maxEntityIDCounter);
        entity->SetSceneID(sceneId);
    }

    for (auto& system : systemsVector)
    {
        system->RegisterEntity(entity);
    }

    entitiesManager->RegisterEntity(entity);

#if 0
    DAVA::RenderComponent* render = static_cast<DAVA::RenderComponent*>(entity->GetComponent<RenderComponent>());
    if (render != nullptr && render->GetRenderObject()->GetType() == RenderObject::TYPE_MESH)
    {
        if (entity->GetComponent<StaticBodyComponent>() == nullptr
            && entity->GetComponent<ConvexHullShapeComponent>() == nullptr
            //            && entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT) == nullptr
            //            && entity->GetComponent(Component::BOX_SHAPE_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::CAPSULE_SHAPE_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::SPHERE_SHAPE_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::PLANE_SHAPE_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::MESH_SHAPE_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::HEIGHT_FIELD_SHAPE_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::VEHICLE_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::VEHICLE_CHASSIS_COMPONENT) == nullptr &&
            //            entity->GetComponent(Component::VEHICLE_WHEEL_COMPONENT) == nullptr
            )
        {
            StaticBodyComponent* sb = new StaticBodyComponent();
            ConvexHullShapeComponent* ch = new ConvexHullShapeComponent();

            entity->AddComponent(sb);
            entity->AddComponent(ch);
        }
    }
#endif
}

void Scene::UnregisterEntity(Entity* entity)
{
    for (auto& system : systemsVector)
    {
        system->UnregisterEntity(entity);
    }

    entitiesManager->UnregisterEntity(entity);
}

void Scene::RegisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity)
{
    system->RegisterEntity(entity);
    for (int32 i = 0, sz = entity->GetChildrenCount(); i < sz; ++i)
        RegisterEntitiesInSystemRecursively(system, entity->GetChild(i));
}

void Scene::RegisterComponent(Entity* entity, Component* component)
{
    DVASSERT(entity && component);
    uint32 systemsCount = static_cast<uint32>(systemsVector.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systemsVector[k]->RegisterComponent(entity, component);
    }

    entitiesManager->RegisterComponent(entity, component);
}

void Scene::UnregisterComponent(Entity* entity, Component* component)
{
    DVASSERT(entity && component);
    uint32 systemsCount = static_cast<uint32>(systemsVector.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systemsVector[k]->UnregisterComponent(entity, component);
    }

    entitiesManager->UnregisterComponent(entity, component);
}

void Scene::AddSystem(SceneSystem* sceneSystem, SceneSystem* insertBeforeSceneForProcess /*= nullptr*/, SceneSystem* insertBeforeSceneForInput /*= nullptr*/, SceneSystem* insertBeforeSceneForFixedProcess /*= nullptr*/)
{
    if (!tags.empty())
    {
        DVASSERT(tags.empty(), "Scene was created by tags, system will not be added.");
        return;
    }

    systemsVector.push_back(sceneSystem);
    const Type* systemType = ReflectedTypeDB::GetByPointer(sceneSystem)->GetType();
    systemsMap[systemType] = sceneSystem;

    auto insertSystemBefore = [sceneSystem](Vector<SceneSystem*>& container, SceneSystem* beforeThisSystem)
    {
        if (beforeThisSystem != nullptr)
        {
            Vector<SceneSystem*>::iterator itEnd = container.end();
            for (Vector<SceneSystem*>::iterator it = container.begin(); it != itEnd; ++it)
            {
                if (beforeThisSystem == (*it))
                {
                    container.insert(it, sceneSystem);
                    return true;
                }
            }
        }
        else
        {
            container.push_back(sceneSystem);
            return true;
        }

        return false;
    };

    bool wasInsertedForProcess = insertSystemBefore(systemsToProcess, insertBeforeSceneForProcess);
    DVASSERT(wasInsertedForProcess);

    bool wasInsertedForInput = insertSystemBefore(systemsToInput, insertBeforeSceneForInput);
    DVASSERT(wasInsertedForInput);

    bool wasInserted = insertSystemBefore(systemsToFixedProcess, insertBeforeSceneForFixedProcess);
    DVASSERT(wasInserted);

    sceneSystem->SetScene(this);
    RegisterEntitiesInSystemRecursively(sceneSystem, this);

    GetSingletonComponent<ChangedSystemsSingleComponent>()->addedSystems.push_back(sceneSystem);
}

void Scene::RemoveSystem(SceneSystem* sceneSystem)
{
    sceneSystem->PrepareForRemove();

    RemoveSystem(systemsToProcess, sceneSystem);
    RemoveSystem(systemsToInput, sceneSystem);
    RemoveSystem(systemsToFixedProcess, sceneSystem);
    const Type* systemType = ReflectedTypeDB::GetByPointer(sceneSystem)->GetType();
    systemsMap.erase(systemType);

    bool removed = RemoveSystem(systemsVector, sceneSystem);
    if (removed)
    {
        sceneSystem->SetScene(nullptr);
        GetSingletonComponent<ChangedSystemsSingleComponent>()->removedSystems.push_back(sceneSystem);
    }
    else
    {
        DVASSERT(false, "Failed to remove system from scene");
    }
}

void Scene::AddTag(FastName tag)
{
    if (tags.empty())
    {
        DVASSERT(!tags.empty());
        return;
    }

    tagsToChange.emplace_back(tag, TagAction::ADD);
}

void Scene::RemoveTag(FastName tag)
{
    DVASSERT(!tags.empty() && tag != FastName("base"));

    tagsToChange.emplace_back(tag, TagAction::REMOVE);
}

bool Scene::HasTag(FastName tag) const
{
    return (tags.find(tag) == tags.end());
}

bool Scene::RemoveSystem(Vector<SceneSystem*>& storage, SceneSystem* system)
{
    Vector<SceneSystem*>::iterator endIt = storage.end();
    for (Vector<SceneSystem*>::iterator it = storage.begin(); it != endIt; ++it)
    {
        if (*it == system)
        {
            storage.erase(it);
            return true;
        }
    }

    return false;
}

void Scene::InitLegacyPointers()
{
    // TODO: destroy this legacy

    actionCollectSystem = GetSystem<ActionCollectSystem>();
    staticOcclusionSystem = GetSystem<StaticOcclusionSystem>();
    animationSystem = GetSystem<AnimationSystem>();
    motionSystem = GetSystem<MotionSystem>();

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
    physicsSystem = GetSystem<PhysicsSystem>();
#endif

    skeletonSystem = GetSystem<SkeletonSystem>();
    slotSystem = GetSystem<SlotSystem>();
    transformSystem = GetSystem<TransformSystem>();
    lodSystem = GetSystem<LodSystem>();
    switchSystem = GetSystem<SwitchSystem>();
    particleEffectSystem = GetSystem<ParticleEffectSystem>();
    soundSystem = GetSystem<SoundUpdateSystem>();
    renderUpdateSystem = GetSystem<RenderUpdateSystem>();
    updatableSystem = GetSystem<UpdateSystem>();
    lightUpdateSystem = GetSystem<LightUpdateSystem>();
    actionSystem = GetSystem<ActionUpdateSystem>();
    debugRenderSystem = GetSystem<DebugRenderSystem>();
    landscapeSystem = GetSystem<LandscapeSystem>();
    foliageSystem = GetSystem<FoliageSystem>();
    speedTreeUpdateSystem = GetSystem<SpeedTreeUpdateSystem>();
    windSystem = GetSystem<WindSystem>();
    waveSystem = GetSystem<WaveSystem>();
    //diffMonitoringSystem = GetSystem<DiffMonitoringSystem>();
    staticOcclusionDebugDrawSystem = GetSystem<StaticOcclusionDebugDrawSystem>();
    particleEffectDebugDrawSystem = GetSystem<ParticleEffectDebugDrawSystem>();
    geoDecalSystem = GetSystem<GeoDecalSystem>();
    // TODO UVR
    //streamingSystem = GetSystem<StreamingSystem>();
}

void Scene::ProcessChangedTags()
{
    if (tagsToChange.empty())
    {
        return;
    }

    SystemManager* sm = GetEngineContext()->systemManager;

    bool updateSystemsList = false;

    for (const auto& p : tagsToChange)
    {
        FastName tag = p.first;
        auto it = tags.find(tag);

        if (p.second == TagAction::ADD && it == tags.end())
        {
            updateSystemsList = tags.insert(tag).second || updateSystemsList;
        }
        else if (p.second == TagAction::REMOVE && it != tags.end())
        {
            for (const auto& sp : systemsMap)
            {
                const auto& tagsForSystem = sm->GetTagsForSystem(sp.first);
                if (std::find(tagsForSystem.begin(), tagsForSystem.end(), tag) != tagsForSystem.end())
                {
                    RemoveSystem(sp.second);
                    entitiesManager->UpdateCaches();
                }
            }
            tags.erase(it);
        }
    }

    tagsToChange.clear();

    if (updateSystemsList)
    {
        CreateSystemsByTags();
    }

    InitLegacyPointers();
}

void Scene::CreateSystemsToMethods(const Vector<SystemManager::SceneProcessInfo>& methods)
{
    for (const auto& p : methods)
    {
        if (systemsMap.find(p.systemType) != systemsMap.end())
        {
            continue;
        }

        const ReflectedType* reflType = ReflectedTypeDB::GetByType(p.systemType);
        const ReflectedStructure* structure = reflType->GetStructure();

        const auto& systemTags = structure->meta->GetMeta<M::Tags>()->tags;

        bool shouldAddSystem = std::all_of(systemTags.begin(), systemTags.end(), [this](FastName tag) { return tags.find(tag) != tags.end(); });

        if (shouldAddSystem)
        {
            Any obj = reflType->CreateObject(ReflectedType::CreatePolicy::ByPointer, this);
            SceneSystem* sceneSystem = static_cast<SceneSystem*>(obj.Get<void*>());
            systemsMap[p.systemType] = sceneSystem;
            systemsVector.push_back(sceneSystem);
            sceneSystem->SetScene(this);
            RegisterEntitiesInSystemRecursively(sceneSystem, this);
            GetSingletonComponent<ChangedSystemsSingleComponent>()->addedSystems.push_back(sceneSystem);
        }
    }
}

void Scene::ProcessManuallyAddedSystems(float32 timeElapsed)
{
    if (!pauseFixedUpdate)
    {
        if (isPerformFixedProcessOnlyOnce)
        {
            for (SceneSystem* system : systemsToFixedProcess)
            {
                system->ProcessFixed(fixedUpdate.constantTime);
                entitiesManager->UpdateCaches();
            }
        }
        else //call ProcessFixed N times where N = (timeSinceLastProcessFixed + timeElapsed) / fixedUpdate.constantTime;
        {
            fixedUpdate.lastTime += timeElapsed;
            while (fixedUpdate.lastTime >= fixedUpdate.fixedTime)
            {
                for (SceneSystem* system : systemsToFixedProcess)
                {
                    system->ProcessFixed(fixedUpdate.constantTime);
                    entitiesManager->UpdateCaches();
                }
                if (pauseFixedUpdate)
                {
                    break;
                }
                fixedUpdate.lastTime -= fixedUpdate.fixedTime;
            }
        }
    }

    for (SceneSystem* system : systemsToProcess)
    {
        if (updatableSystem != nullptr && system == transformSystem)
        {
            updatableSystem->UpdatePreTransform(timeElapsed);
            transformSystem->Process(timeElapsed);
            updatableSystem->UpdatePostTransform(timeElapsed);
            entitiesManager->UpdateCaches();
        }
        else if (system == lodSystem)
        {
            if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LODS))
            {
                lodSystem->Process(timeElapsed);
                entitiesManager->UpdateCaches();
            }
        }
        else
        {
            system->Process(timeElapsed);
            entitiesManager->UpdateCaches();
        }
    }
}

void Scene::ProcessSystemsAddedByTags(float32 timeElapsed)
{
    ProcessChangedTags(); // Check if tags changed. Correct systems list according to changed tags.

    SystemManager* sm = GetEngineContext()->systemManager;

    if (!pauseFixedUpdate)
    {
        auto ProcessFixedMethods = [this, sm]() {
            for (const auto& p : sm->GetFixedProcessMethods())
            {
                auto it = systemsMap.find(p.systemType);
                if (it != systemsMap.end())
                {
                    p.method->InvokeWithCast(it->second, fixedUpdate.constantTime);
                    entitiesManager->UpdateCaches();
                }
            }
        };

        if (isPerformFixedProcessOnlyOnce)
        {
            ProcessFixedMethods();
        }
        else //call ProcessFixed N times where N = (timeSinceLastProcessFixed + timeElapsed) / fixedUpdate.constantTime;
        {
            fixedUpdate.lastTime += timeElapsed;
            while (fixedUpdate.lastTime >= fixedUpdate.fixedTime)
            {
                ProcessFixedMethods();
                if (pauseFixedUpdate)
                {
                    break;
                }
                fixedUpdate.lastTime -= fixedUpdate.fixedTime;
            }
        }
    }

    for (const auto& p : sm->GetProcessMethods())
    {
        auto it = systemsMap.find(p.systemType);
        if (it == systemsMap.end())
        {
            continue;
        }

        SceneSystem* system = it->second;

        if (updatableSystem != nullptr && system == transformSystem)
        {
            updatableSystem->UpdatePreTransform(timeElapsed);
            transformSystem->Process(timeElapsed);
            updatableSystem->UpdatePostTransform(timeElapsed);
            entitiesManager->UpdateCaches();
        }
        else if (system == lodSystem)
        {
            if (Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_LODS))
            {
                lodSystem->Process(timeElapsed);
                entitiesManager->UpdateCaches();
            }
        }
        else
        {
            p.method->InvokeWithCast(system, timeElapsed);
            entitiesManager->UpdateCaches();
        }
    }
}

void Scene::CreateSystemsByTags()
{
    // TODO: cache methods ptrs
    // TODO: get rid of public pointers.
    // TODO: UnregisterComponent(,) for all single components?
    // TODO: runtime system registration (after init), signals in system manager?

    if (tags.empty())
    {
        DVASSERT(!tags.empty());
        return;
    }

    SystemManager* sm = GetEngineContext()->systemManager;

    // Add systems without methods.
    for (const auto& p : sm->GetSystemsWithoutProcessMethods())
    {
        if (systemsMap.find(p.systemType) == systemsMap.end())
        {
            const ReflectedType* reflType = ReflectedTypeDB::GetByType(p.systemType);
            const ReflectedStructure* structure = reflType->GetStructure();

            const auto& systemTags = structure->meta->GetMeta<M::Tags>()->tags;

            bool shouldAddSystem = std::all_of(systemTags.begin(), systemTags.end(), [this](FastName tag) { return tags.find(tag) != tags.end(); });

            if (shouldAddSystem)
            {
                Any obj = reflType->CreateObject(ReflectedType::CreatePolicy::ByPointer, this);
                SceneSystem* sceneSystem = static_cast<SceneSystem*>(obj.Get<void*>());
                systemsMap[p.systemType] = sceneSystem;
                systemsVector.push_back(sceneSystem);
                sceneSystem->SetScene(this);
                RegisterEntitiesInSystemRecursively(sceneSystem, this);
                GetSingletonComponent<ChangedSystemsSingleComponent>()->addedSystems.push_back(sceneSystem);
            }
        }
    }

    CreateSystemsToMethods(sm->GetProcessMethods());
    CreateSystemsToMethods(sm->GetFixedProcessMethods());

    InitLegacyPointers();
}

void Scene::AddSingletonComponent(SingletonComponent* component, const Type* type)
{
    DVASSERT(std::find_if(singletonComponents.begin(), singletonComponents.end(), [type](const auto& x) { return x.second == type; }) == singletonComponents.end());

    singletonComponents.emplace_back(component, type);

    uint32 systemsCount = static_cast<uint32>(systemsVector.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systemsVector[k]->RegisterSingleComponent(component);
    }
}

void Scene::RemoveSingletonComponent(SingletonComponent* component)
{
    for (size_t i = 0; i < singletonComponents.size(); ++i)
    {
        if (singletonComponents[i].first == component)
        {
            uint32 systemsCount = static_cast<uint32>(systemsVector.size());
            for (uint32 k = 0; k < systemsCount; ++k)
            {
                systemsVector[k]->UnregisterSingleComponent(component);
            }

            RemoveExchangingWithLast(singletonComponents, i);
            delete component;

            break;
        }
    }
}

SingletonComponent* Scene::GetSingletonComponent(const Type* type)
{
    SingletonComponent* result = nullptr;
    for (auto& pair : singletonComponents)
    {
        if (pair.second == type)
        {
            result = pair.first;
            break;
        }
    }

    if (result == nullptr)
    {
        result = static_cast<SingletonComponent*>(ComponentUtils::CreateByType(type));
        DVASSERT(result != nullptr);
        AddSingletonComponent(result, type);
    }

    return result;
}

Scene* Scene::GetScene()
{
    return this;
}

void Scene::AddCamera(Camera* camera)
{
    if (camera)
    {
        camera->Retain();
        cameras.push_back(camera);
    }
}

bool Scene::RemoveCamera(Camera* c)
{
    const auto& it = std::find(cameras.begin(), cameras.end(), c);
    if (it != cameras.end())
    {
        SafeRelease(*it);
        cameras.erase(it);
        return true;
    }
    return false;
}

Camera* Scene::GetCamera(int32 n)
{
    if (n >= 0 && n < static_cast<int32>(cameras.size()))
        return cameras[n];

    return nullptr;
}

void Scene::Update(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_UPDATE);

    if (!tags.empty())
    {
        ProcessSystemsAddedByTags(timeElapsed);
    }
    else
    {
        ProcessManuallyAddedSystems(timeElapsed);
    }

    GetSingletonComponent<ActionsSingleComponent>()->Clear();
    GetSingletonComponent<TransformSingleComponent>()->Clear();
    GetSingletonComponent<ChangedSystemsSingleComponent>()->Clear();
    GetSingletonComponent<LightmapSingleComponent>()->Clear();
    GetSingletonComponent<VTSingleComponent>()->Clear();

    sceneGlobalTime += timeElapsed;
}

void Scene::Draw()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_DRAW)

    //GFX_COMPLETE - at least move it somewhere out of Scene::Draw (may be RenderSystem::Render)
    //TODO: re-think configuring global dynamic bindings
    static Color defShadowColor(1.f, 0.f, 0.f, 1.f);
    static Color defWaterClearColor(0.f, 0.f, 0.f, 0.f);
    static Vector4 vDef(1.0f, 1.0f, 1.0f, 1.0f);

    const float32* shadowDataPtr = defShadowColor.color;
    const float32* waterDataPtr = defWaterClearColor.color;
    if (sceneGlobalMaterial && sceneGlobalMaterial->HasLocalProperty(DAVA::NMaterialParamName::DEPRECATED_SHADOW_COLOR_PARAM))
        shadowDataPtr = sceneGlobalMaterial->GetLocalPropValue(DAVA::NMaterialParamName::DEPRECATED_SHADOW_COLOR_PARAM);
    if (sceneGlobalMaterial && sceneGlobalMaterial->HasLocalProperty(DAVA::NMaterialParamName::WATER_CLEAR_COLOR))
        waterDataPtr = sceneGlobalMaterial->GetLocalPropValue(DAVA::NMaterialParamName::WATER_CLEAR_COLOR);

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_COLOR, shadowDataPtr, reinterpret_cast<pointer_size>(shadowDataPtr));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WATER_CLEAR_COLOR, waterDataPtr, reinterpret_cast<pointer_size>(waterDataPtr));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_GLOBAL_TIME, &sceneGlobalTime, reinterpret_cast<pointer_size>(&sceneGlobalTime));

    //GFX_COMPLETE - Actually bad idea to draw shadow with identity
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_VIEW, Matrix4::IDENTITY.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_PARAMETERS, vDef.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    //GFX_COMPLETE - Vitaly - double check the logic of this params, as without assertion it would be hard to understend why local probes work incorrect
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::LOCAL_PROBE_CAPTURE_POSITION_IN_WORLDSPACE, Matrix4::IDENTITY.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::LOCAL_PROBE_CAPTURE_WORLD_TO_LOCAL_MATRIX, Matrix4::IDENTITY.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

    //GFX_COMPLETE - move this to development - really useful
    //in case we don't have light we are to bind some default values to prevent assertions
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_POSITION, &Vector4::Zero, reinterpret_cast<pointer_size>(&Vector4::Zero));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_LIGHT0_COLOR, &Color::Black, reinterpret_cast<pointer_size>(&Color::Black));

    skeletonSystem->DrawSkeletons(renderSystem->GetDebugDrawer());
    //renderSystem->DebugDrawHierarchy(renderSystem->GetMainCamera()->GetViewProjMatrix());

    renderSystem->Render();

    if (particleEffectDebugDrawSystem != nullptr)
        particleEffectDebugDrawSystem->Draw();
}

void Scene::SceneDidLoaded()
{
    maxEntityIDCounter = 0;

    std::function<void(Entity*)> findMaxId = [&](Entity* entity)
    {
        if (maxEntityIDCounter < entity->id)
            maxEntityIDCounter = entity->id;
        for (auto child : entity->children) findMaxId(child);
    };

    findMaxId(this);

    uint32 systemsCount = static_cast<uint32>(systemsVector.size());
    for (uint32 k = 0; k < systemsCount; ++k)
    {
        systemsVector[k]->SceneDidLoaded();
    }
}

void Scene::SetCurrentCamera(Camera* _camera)
{
    SafeRelease(mainCamera);
    mainCamera = SafeRetain(_camera);
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

Camera* Scene::GetCurrentCamera() const
{
    return mainCamera;
}

void Scene::SetCustomDrawCamera(Camera* _camera)
{
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(_camera);
}

Camera* Scene::GetDrawCamera() const
{
    return drawCamera;
}

EventSystem* Scene::GetEventSystem() const
{
    return eventSystem;
}

RenderSystem* Scene::GetRenderSystem() const
{
    return renderSystem;
}

AnimationSystem* Scene::GetAnimationSystem() const
{
    return animationSystem;
}

ParticleEffectDebugDrawSystem* Scene::GetParticleEffectDebugDrawSystem() const
{
    return particleEffectDebugDrawSystem;
}

#if defined(__DAVAENGINE_PHYSICS_ENABLED__)
PhysicsSystem* Scene::GetPhysicsSystem() const
{
    return physicsSystem;
}
#endif

SceneFileV2::eError Scene::LoadScene(const DAVA::FilePath& pathname)
{
    SceneFileV2::eError ret = SceneFileV2::ERROR_FAILED_TO_CREATE_FILE;

    RemoveAllChildren();
    SetName(pathname.GetFilename().c_str());

    if (pathname.IsEqualToExtension(".sc2"))
    {
        ScopedPtr<SceneFileV2> file(new SceneFileV2());
        file->EnableDebugLog(false);
        ret = file->LoadScene(pathname, this);
    }

    return ret;
}

SceneFileV2::eError Scene::SaveScene(const DAVA::FilePath& pathname, bool saveForGame /*= false*/)
{
    std::function<void(Entity*)> resolveId = [&](Entity* entity)
    {
        if (0 == entity->id)
            entity->id = ++maxEntityIDCounter;
        for (auto child : entity->children) resolveId(child);
    };

    resolveId(this);

    ScopedPtr<SceneFileV2> file(new SceneFileV2());
    file->EnableDebugLog(false);
    file->EnableSaveForGame(saveForGame);
    return file->SaveScene(pathname, this);
}

void Scene::ConstructFromPrefab(const Asset<Prefab>& prefab)
{
    RemoveAllChildren();

    if (prefab != nullptr)
    {
        AssetFileInfo fileInfo = GetEngineContext()->assetManager->GetAssetFileInfo(prefab);
        SetName(fileInfo.fileName.c_str());

        Vector<Entity*> prefabEntities = prefab->GetPrefabEntities();
        for (Entity* prefabChildren : prefabEntities)
        {
            ScopedPtr<Entity> entityClone(prefabChildren->Clone());
            AddEntity(entityClone);
        }
    }
}

void Scene::OptimizeBeforeExport()
{
    List<NMaterial*> materials;
    GetDataNodes(materials);

    const auto RemoveMaterialFlag = [](NMaterial* material, const FastName& flagName) {
        if (material->HasLocalFlag(flagName))
        {
            material->RemoveFlag(flagName);
        }
    };

    Entity::OptimizeBeforeExport();
}

void Scene::OnSceneReady(Entity* rootNode)
{
}

void Scene::Input(DAVA::UIEvent* event)
{
    if (!tags.empty())
    {
        RotationControllerSystem* rcs = GetSystem<RotationControllerSystem>();

        if (rcs != nullptr)
        {
            rcs->Input(event);
        }

        return;
    }

    for (SceneSystem* system : systemsToInput)
    {
        system->Input(event);
    }
}

void Scene::InputCancelled(UIEvent* event)
{
    if (!tags.empty())
    {
        RotationControllerSystem* rcs = GetSystem<RotationControllerSystem>();

        if (rcs != nullptr)
        {
            rcs->InputCancelled(event);
        }

        return;
    }

    for (SceneSystem* system : systemsToInput)
    {
        system->InputCancelled(event);
    }
}

void Scene::HandleEvent(Observable* observable)
{
    RenderOptions* options = dynamic_cast<RenderOptions*>(observable);

    if (options->IsOptionEnabled(RenderOptions::REPLACE_LIGHTMAP_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterialTextureName::TEXTURE_LIGHTMAP);
    if (options->IsOptionEnabled(RenderOptions::REPLACE_ALBEDO_MIPMAPS))
        MipMapReplacer::ReplaceMipMaps(this, NMaterialTextureName::TEXTURE_ALBEDO);

    if (options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && !staticOcclusionDebugDrawSystem)
    {
        if (!tags.empty())
        {
            AddTag(FastName("static_occlusion_debug"));
        }
        else
        {
            staticOcclusionDebugDrawSystem = new StaticOcclusionDebugDrawSystem(this);
            AddSystem(staticOcclusionDebugDrawSystem, renderUpdateSystem);
        }
    }
    else if (!options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && staticOcclusionDebugDrawSystem)
    {
        if (!tags.empty())
        {
            RemoveTag(FastName("static_occlusion_debug"));
        }
        else
        {
            RemoveSystem(staticOcclusionDebugDrawSystem);
            SafeDelete(staticOcclusionDebugDrawSystem);
        }
    }

    if (DAVA::Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_DRAW_PARTICLES) && particleEffectDebugDrawSystem == nullptr)
    {
        if (!tags.empty())
        {
            AddTag(FastName("particle_effect_debug"));
        }
        else
        {
            particleEffectDebugDrawSystem = new ParticleEffectDebugDrawSystem(this);
            AddSystem(particleEffectDebugDrawSystem, 0);
        }
    }
    else if (!DAVA::Renderer::GetOptions()->IsOptionEnabled(RenderOptions::DEBUG_DRAW_PARTICLES) && particleEffectDebugDrawSystem != nullptr)
    {
        if (!tags.empty())
        {
            RemoveTag(FastName("particle_effect_debug"));
        }
        else
        {
            RemoveSystem(particleEffectDebugDrawSystem);
            SafeDelete(particleEffectDebugDrawSystem);
        }
    }
}

void Scene::Activate()
{
    for (auto system : systemsVector)
    {
        system->Activate();
    }
}

void Scene::Deactivate()
{
    for (auto system : systemsVector)
    {
        system->Deactivate();
    }
}

void Scene::PauseFixedUpdate()
{
    pauseFixedUpdate = true;
}

void Scene::UnpauseFixedUpdate()
{
    pauseFixedUpdate = false;
}

bool Scene::IsFixedUpdatePaused() const
{
    return pauseFixedUpdate;
}
};
