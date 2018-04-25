#include "Scene3D/Scene.h"

#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Engine/EngineContext.h"
#include "Entity/ComponentUtils.h"
#include "Entity/SceneSystem.h"
#include "Entity/SystemProcessInfo.h"

#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Private/EntitiesManager.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/ParticleEffectDebugDrawSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Systems/StaticOcclusionSystem.h"

#include "Reflection/ReflectionRegistrator.h"

#include "Render/MipmapReplacer.h"
#include "Render/Renderer.h"
#include "Render/RenderOptions.h"
#include "Render/Highlevel/RenderSystem.h"

#include "Utils/StringFormat.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(Scene)
{
    ReflectionRegistrator<Scene>::Begin()
    .End();
}

Scene::Scene()
    : entitiesManager(new EntitiesManager())
    , systemManager(GetEngineContext()->systemManager)
{
    static uint32 idCounter = 0;
    sceneId = ++idCounter;

    systemRemoved.Connect(entitiesManager, &EntitiesManager::OnSystemRemoved);

    renderSystem = new RenderSystem();
    eventSystem = new EventSystem();

    // this will force scene to create hidden global material
    SetGlobalMaterial(nullptr);

    RenderOptions* options = Renderer::GetOptions();
    options->AddObserver(this);
}

Scene::Scene(const FastTags& tags, bool createSystems /* = true */)
    : Scene()
{
    if (createSystems)
    {
        AddTags(tags);
    }
    else
    {
        sceneTags = tags;
    }
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

    ParticleEffectSystem* const particleEffectSystem = GetSystem<ParticleEffectSystem>();

    if (nullptr != particleEffectSystem)
    {
        particleEffectSystem->SetGlobalMaterial(sceneGlobalMaterial);
    }
}

void Scene::SetMainPassProperties(uint32 priority, const Rect& viewport, uint32 width, uint32 height, PixelFormat format)
{
    renderSystem->SetMainPassProperties(priority, viewport, width, height, format);
}

void Scene::SetMainRenderTarget(rhi::HTexture color, rhi::HTexture depthStencil, rhi::LoadAction colorLoadAction, const Color& clearColor)
{
    renderSystem->SetMainRenderTarget(color, depthStencil, colorLoadAction, clearColor);
}

void Scene::SetFixedUpdateTime(float32 time)
{
    fixedUpdate.fixedTime = time;
}

void Scene::SetFixedUpdateAdjustment(float32 time)
{
    fixedUpdate.adjustment = time;
}

float32 Scene::GetFixedUpdateTime() const
{
    return fixedUpdate.fixedTime;
}

float32 Scene::GetFixedUpdateOverlap() const
{
    return fixedUpdate.overlap;
}

void Scene::SetPerformFixedProcessOnlyOnce(bool isPerformFixedProcessOnlyOnce)
{
    fixedUpdate.onlyOnce = isPerformFixedProcessOnlyOnce;
}

Scene::~Scene()
{
    entitiesManager->UpdateCaches();

    Renderer::GetOptions()->RemoveObserver(this);

    renderSystem->PrepareForShutdown();

    auto systemsMapCopy = systemsMap;
    for (const auto& p : systemsMapCopy)
    {
        const Type* systemType = p.first;
        RemoveSystem(systemType);
    }
    systemAdded.DisconnectAll();
    systemRemoved.DisconnectAll();

    for (auto& p : singleComponents)
    {
        SafeDelete(p.second);
    }

    SafeRelease(mainCamera);
    SafeRelease(drawCamera);
    for (Camera*& c : cameras)
    {
        SafeRelease(c);
    }

    RemoveAllChildren();
    SafeRelease(sceneGlobalMaterial);

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

    for (SceneSystem* system : systemsVector)
    {
        system->RegisterEntity(entity);
    }

    entitiesManager->RegisterEntity(entity);
}

void Scene::UnregisterEntity(Entity* entity)
{
    for (SceneSystem* system : systemsVector)
    {
        system->UnregisterEntity(entity);
    }

    entitiesManager->UnregisterEntity(entity);
}

void Scene::RegisterEntitiesInSystemRecursively(SceneSystem* system, Entity* entity)
{
    system->RegisterEntity(entity);
    for (int32 i = 0, size = entity->GetChildrenCount(); i < size; ++i)
    {
        RegisterEntitiesInSystemRecursively(system, entity->GetChild(i));
    }
}

void Scene::RegisterSingleComponentsInSystem(SceneSystem* system)
{
    for (const auto& p : singleComponents)
    {
        SingleComponent* singleComponent = p.second;
        system->RegisterSingleComponent(singleComponent);
    }
}

void Scene::RegisterComponent(Entity* entity, Component* component)
{
    DVASSERT(nullptr != entity && nullptr != component);

    static bool entered = false;

    DVASSERT(entered == false, "Scene::RegisterComponent should not be called recursively");

    entered = true;

    for (SceneSystem* system : systemsVector)
    {
        system->RegisterComponent(entity, component);
    }

    entitiesManager->RegisterComponent(entity, component);

    entered = false;
}

void Scene::UnregisterComponent(Entity* entity, Component* component)
{
    DVASSERT(nullptr != entity && nullptr != component);

    for (SceneSystem* system : systemsVector)
    {
        system->UnregisterComponent(entity, component);
    }

    entitiesManager->UnregisterComponent(entity, component);
}

void Scene::AddSystem(const Type* systemType)
{
    DVASSERT(nullptr != systemType);

    if (systemsMap.find(systemType) == systemsMap.end())
    {
        const SystemManager::SystemInfo* const systemInfo = systemManager->GetSystemInfo(systemType);

        if (nullptr != systemInfo)
        {
            // All the checks can be avoided here since they are performed by SystemManager in registration step.
            const ReflectedType* const reflectedType = ReflectedTypeDB::GetByType(systemType);
            Any object = reflectedType->CreateObject(ReflectedType::CreatePolicy::ByPointer, this);

            SceneSystem* system = static_cast<SceneSystem*>(object.Get<void*>());
            system->SetScene(this);

            for (const SystemManager::SystemProcess& systemProcess : systemInfo->processMethods)
            {
                auto insertProcess = [](auto& sortedContainer, const ProcessSystemPair& process) {
                    auto position = std::lower_bound(begin(sortedContainer), end(sortedContainer), process, [](const ProcessSystemPair& l, const ProcessSystemPair& r) {
                        const SystemProcessInfo& left = l.first->info;
                        const SystemProcessInfo& right = r.first->info;
                        return left < right;
                    });
                    DVASSERT(position == end(sortedContainer) || position->first->info != process.first->info);
                    sortedContainer.insert(position, process);
                };

                const ProcessSystemPair process = std::make_pair(&systemProcess, system);

                switch (systemProcess.info.type)
                {
                case SPI::Type::Normal:
                    insertProcess(processes, process);
                    break;
                case SPI::Type::Fixed:
                    insertProcess(fixedProcesses, process);
                    break;
                case SPI::Type::Input:
                    insertProcess(inputProcesses, process);
                    break;
                default:
                    DVASSERT(false, "Unhandled case.");
                }
            }

            systemsMap[systemType] = system;
            systemsVector.push_back(system);

            RegisterSingleComponentsInSystem(system);
            RegisterEntitiesInSystemRecursively(system, this);

            systemAdded.Emit(system);
        }
        else
        {
            DVASSERT(false, Format("Unable to get info for system of type `%s`.", systemType->GetName()).c_str());
        }
    }
    else
    {
        DVASSERT(false, Format("System of type `%s` already exists in the scene.", systemType->GetName()).c_str());
    }
}

void Scene::RemoveSystem(const Type* systemType)
{
    const auto it = systemsMap.find(systemType);

    if (it != systemsMap.end())
    {
        SceneSystem* system = it->second;

        system->PrepareForRemove();

        auto cleanUp = [system](auto& v) { v.erase(std::remove_if(begin(v), end(v), [system](const auto& p) { return p.second == system; }), end(v)); };

        cleanUp(fixedProcesses);
        cleanUp(processes);
        cleanUp(inputProcesses);

        systemsMap.erase(it);
        systemsVector.erase(std::remove(begin(systemsVector), end(systemsVector), system), end(systemsVector));

        systemRemoved.Emit(system);

        SafeDelete(system);

        entitiesManager->UpdateCaches();
    }
    else
    {
        DVASSERT(false, Format("System of type `%s` doesn't exist in the scene.", systemType->GetName()).c_str());
    }
}

void Scene::AddTags(const FastTags& tags)
{
    if (!sceneTags.empty() || systemsMap.empty())
    {
        const bool allTagsAreNewToScene = std::all_of(cbegin(tags.tags), cend(tags.tags), [& sceneTags = this->sceneTags](const FastName& tag) {
            return sceneTags.find(tag) == sceneTags.end();
        });

        if (allTagsAreNewToScene)
        {
            sceneTags.insert(cbegin(tags.tags), cend(tags.tags));

            const Vector<const Type*> systems = systemManager->GetSystems(sceneTags);

            CorrectSystemsList({ cbegin(systems), cend(systems) });
        }
        else
        {
            DVASSERT(allTagsAreNewToScene, "Some tags are not unique to scene, function will have no effect.");
        }
    }
    else
    {
        DVASSERT(false, "Tags are forbidden for scene with manually added systems.");
    }
}

void Scene::RemoveTags(const FastTags& tags)
{
    if (HasTags(tags))
    {
        for (const FastName& tag : tags.tags)
        {
            const bool successfulInsert = tagsToRemove.insert(tag).second;
            DVASSERT(successfulInsert, "Tag is already marked to be removed.");
        }
    }
    else
    {
        DVASSERT(false, "Tags marked to be removed are not found in the scene tags, function will have no effect.");
    }
}

bool Scene::HasTags(const FastTags& tags) const
{
    DVASSERT(!tags.tags.empty());

    return std::all_of(cbegin(tags.tags), cend(tags.tags), [& st = sceneTags](const FastName& tag) { return st.find(tag) != st.end(); });
}

const UnorderedSet<FastName>& Scene::GetTags()
{
    return sceneTags;
}

void Scene::AddSystemManually(const Type* systemType)
{
    if (sceneTags.empty())
    {
        if (systemsMap.find(systemType) == systemsMap.end())
        {
            AddSystem(systemType);
        }
        else
        {
            DVASSERT(false, "System with given type is already exists in the scene, function will have no effect.");
        }
    }
    else
    {
        DVASSERT(false, "Manual modifications of systems list are forbidden for scene with systems added by tags.");
    }
}

void Scene::RemoveSystemManually(const Type* systemType)
{
    if (sceneTags.empty())
    {
        const bool successfulInsert = systemsToRemove.insert(systemType).second;
        DVASSERT(successfulInsert, "System is already marked to be removed.");
    }
    else
    {
        DVASSERT(false, "Manual modifications of systems list are forbidden for scene with systems added by tags.");
    }
}

void Scene::CorrectSystemsList(const UnorderedSet<const Type*>& systems)
{
    auto filterRemovedSystems = [this](const auto& systems)
    {
        Vector<const Type*> removedSystems;

        for (const auto& p : systemsMap)
        {
            const Type* system = p.first;

            if (systems.find(system) == systems.end())
            {
                removedSystems.push_back(system);
            }
        }

        return removedSystems;
    };

    auto filterAddedSystems = [this](const auto& systems)
    {
        Vector<const Type*> addedSystems;

        std::copy_if(cbegin(systems), cend(systems), std::back_inserter(addedSystems), [this](const Type* system) {
            return systemsMap.find(system) == systemsMap.end();
        });

        return addedSystems;
    };

    for (const Type* systemType : filterRemovedSystems(systems))
    {
        RemoveSystem(systemType);
    }

    for (const Type* systemType : filterAddedSystems(systems))
    {
        AddSystem(systemType);
    }
}

void Scene::ExecuteFixedProcesses(float32 timeElapsed)
{
    auto fixedProcessesCopy = fixedProcesses; // Iterate over copy to allow immediate systems creation.

    if (!fixedUpdate.paused)
    {
        auto ProcessFixedMethods = [this, &fixedProcessesCopy]() {
            for (const ProcessSystemPair& p : fixedProcessesCopy)
            {
                const AnyFn& fixedProcess = p.first->process;
                SceneSystem* system = p.second;
                fixedProcess.InvokeWithCast(system, fixedUpdate.fixedTime);
                entitiesManager->UpdateCaches();
            }
            ClearFixedProcessesSingleComponents();
        };

        fixedUpdate.accumulatedTime += timeElapsed;

        if (fixedUpdate.onlyOnce)
        {
            if (fixedUpdate.accumulatedTime >= fixedUpdate.fixedTime || fixedUpdate.firstUpdate)
            {
                DVASSERT(!Renderer::IsInitialized() || (fixedUpdate.fixedTime >= 1.f / Renderer::GetDesiredFPS()));

                ProcessFixedMethods();

                fixedUpdate.overlap = 1.0f;
                fixedUpdate.accumulatedTime -= fixedUpdate.fixedTime;
            }
        }
        else //call ProcessFixed N times where N = (timeSinceLastProcessFixed + timeElapsed) / fixedUpdate.constantTime;
        {
            while (fixedUpdate.accumulatedTime > 0)
            {
                ProcessFixedMethods();

                fixedUpdate.accumulatedTime -= (fixedUpdate.fixedTime + fixedUpdate.adjustment);

                if (fixedUpdate.paused)
                {
                    break;
                }
            }

            fixedUpdate.overlap = (fixedUpdate.fixedTime + fixedUpdate.accumulatedTime + fixedUpdate.adjustment) / fixedUpdate.fixedTime;
            //Logger::Info("FixedUpdate call count = %u, adjustment = %f, overlap = %f", fuCount, fixedUpdate.adjustment, fixedUpdate.overlap);
        }
    }
}

void Scene::ExecuteProcesses(float32 timeElapsed)
{
    auto processesCopy = processes; // Iterate over copy to allow immediate systems creation.

    for (const ProcessSystemPair& p : processesCopy)
    {
        const AnyFn& process = p.first->process;
        SceneSystem* system = p.second;

        process.InvokeWithCast(system, timeElapsed);

        entitiesManager->UpdateCaches();
    }

    ClearAllProcessesSingleComponents();
}

void Scene::OnNewSystemRegistered(const Type* systemType, const SystemManager::SystemInfo* systemInfo)
{
    const UnorderedSet<FastName>& systemTags = systemInfo->tags->tags;

    const bool tagsMatch = std::all_of(cbegin(systemTags), cend(systemTags), [this](const FastName& tag) { return sceneTags.find(tag) != sceneTags.end(); });

    if (tagsMatch && (systemsMap.find(systemType) == systemsMap.end()))
    {
        CorrectSystemsList({ systemType });
    }
}

void Scene::QueryRemovedSystems()
{
    if (!tagsToRemove.empty())
    {
        for (const FastName& tag : std::exchange(tagsToRemove, {}))
        {
            const bool successfulErase = sceneTags.erase(tag) != 0;
            DVASSERT(successfulErase);
        }

        Vector<const Type*> systems;

        if (!sceneTags.empty())
        {
            systems = systemManager->GetSystems(sceneTags);
        }

        CorrectSystemsList({ cbegin(systems), cend(systems) });
    }
    else if (!systemsToRemove.empty())
    {
        UnorderedSet<const Type*> newSystemsList;

        for (const auto& p : systemsMap)
        {
            const Type* systemType = p.first;
            newSystemsList.insert(systemType);
        }

        for (const Type* systemType : std::exchange(systemsToRemove, {}))
        {
            const bool successfulErase = newSystemsList.erase(systemType) != 0;
            DVASSERT(successfulErase);
        }

        CorrectSystemsList(newSystemsList);
    }
}

EntityGroupOnAdd* Scene::AquireEntityGroupOnAdd(EntityGroup* eg, SceneSystem* sceneSystem)
{
    return entitiesManager->AquireEntityGroupOnAdd(eg, sceneSystem);
}

void Scene::AddSingleComponent(SingleComponent* component, const Type* type)
{
    DVASSERT(component != nullptr);
    DVASSERT(singleComponents.find(type) == singleComponents.end());

    singleComponents[type] = component;

    // If it's a clearable single components, put it to according vector
    ClearableSingleComponent* clearableSingleComponent = dynamic_cast<ClearableSingleComponent*>(component);
    if (clearableSingleComponent != nullptr)
    {
        if (clearableSingleComponent->GetUsage() == ClearableSingleComponent::Usage::FixedProcesses)
        {
            clearableSingleComponentsFixed.push_back(clearableSingleComponent);
        }
        else
        {
            DVASSERT(clearableSingleComponent->GetUsage() == ClearableSingleComponent::Usage::AllProcesses);
            clearableSingleComponentsAll.push_back(clearableSingleComponent);
        }
    }

    for (SceneSystem* system : systemsVector)
    {
        system->RegisterSingleComponent(component);
    }
}

SingleComponent* Scene::GetSingleComponent(const Type* type)
{
    SingleComponent* result = nullptr;

    auto it = singleComponents.find(type);
    if (it != singleComponents.end())
    {
        result = it->second;
    }
    else
    {
        result = static_cast<SingleComponent*>(ComponentUtils::Create(type));
        AddSingleComponent(result, type);
    }

    return result;
}

const SingleComponent* Scene::GetSingleComponentForRead(const Type* type, const SceneSystem* user)
{
    DVASSERT(type != nullptr);
    DVASSERT(user != nullptr);

#if defined(__DAVAENGINE_DEBUG__)
    HandleSingleComponentRead(user, type);
#endif

    const SingleComponent* singleComponent = GetSingleComponent(type);
    return singleComponent;
}

SingleComponent* Scene::GetSingleComponentForWrite(const Type* type, const SceneSystem* user)
{
    DVASSERT(type != nullptr);
    DVASSERT(user != nullptr);

#if defined(__DAVAENGINE_DEBUG__)
    HandleSingleComponentWrite(user, type);
#endif

    SingleComponent* singleComponent = GetSingleComponent(type);
    return singleComponent;
}

void Scene::CreateDelayedSystems()
{
    if (systemsMap.empty())
    {
        const Vector<const Type*> systems = systemManager->GetSystems(sceneTags);

        CorrectSystemsList({ cbegin(systems), cend(systems) });
    }
    else
    {
        DVASSERT(false, "Systems already created, function will have no effect.");
    }
}

void Scene::ClearFixedProcessesSingleComponents()
{
    for (ClearableSingleComponent* c : clearableSingleComponentsFixed)
    {
        c->Clear();
    }
}

void Scene::ClearAllProcessesSingleComponents()
{
    for (ClearableSingleComponent* c : clearableSingleComponentsAll)
    {
        c->Clear();
    }
}

Scene* Scene::GetScene()
{
    return this;
}

void Scene::AddCamera(Camera* camera)
{
    if (nullptr != camera)
    {
        camera->Retain();
        cameras.push_back(camera);
    }
}

bool Scene::RemoveCamera(Camera* c)
{
    const auto it = std::find(cameras.begin(), cameras.end(), c);
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
    {
        return cameras[n];
    }

    return nullptr;
}

void Scene::Update(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_UPDATE);

    ExecuteFixedProcesses(timeElapsed);

    ExecuteProcesses(timeElapsed);

    if (processes.empty())
    {
        entitiesManager->UpdateCaches();
    }

    QueryRemovedSystems();

    sceneGlobalTime += timeElapsed;

    fixedUpdate.firstUpdate = false;

#if defined(__DAVAENGINE_DEBUG__)
    ValidateSingleComponentsAccessOrder();
#endif
}

void Scene::Draw()
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_DRAW)

    //TODO: re-think configuring global dynamic bindings
    static Color defShadowColor(1.f, 0.f, 0.f, 1.f);
    static Color defWaterClearColor(0.f, 0.f, 0.f, 0.f);

    const float32* shadowDataPtr = defShadowColor.color;
    const float32* waterDataPtr = defWaterClearColor.color;
    if (sceneGlobalMaterial && sceneGlobalMaterial->HasLocalProperty(NMaterialParamName::DEPRECATED_SHADOW_COLOR_PARAM))
        shadowDataPtr = sceneGlobalMaterial->GetLocalPropValue(NMaterialParamName::DEPRECATED_SHADOW_COLOR_PARAM);
    if (sceneGlobalMaterial && sceneGlobalMaterial->HasLocalProperty(NMaterialParamName::WATER_CLEAR_COLOR))
        waterDataPtr = sceneGlobalMaterial->GetLocalPropValue(NMaterialParamName::WATER_CLEAR_COLOR);

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_SHADOW_COLOR, shadowDataPtr, reinterpret_cast<pointer_size>(shadowDataPtr));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WATER_CLEAR_COLOR, waterDataPtr, reinterpret_cast<pointer_size>(waterDataPtr));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_GLOBAL_TIME, &sceneGlobalTime, reinterpret_cast<pointer_size>(&sceneGlobalTime));

    renderSystem->Render();

    ParticleEffectDebugDrawSystem* const particleEffectDebugDrawSystem = GetSystem<ParticleEffectDebugDrawSystem>();

    if (nullptr != particleEffectDebugDrawSystem)
    {
        particleEffectDebugDrawSystem->Draw();
    }
}

void Scene::OnSceneLoaded()
{
    maxEntityIDCounter = 0;

    std::function<void(Entity*)> findMaxId = [&](Entity* entity)
    {
        if (maxEntityIDCounter < entity->id)
        {
            maxEntityIDCounter = entity->id;
        }
        for (Entity* child : entity->children)
        {
            findMaxId(child);
        }
    };

    findMaxId(this);

    for (SceneSystem* system : systemsVector)
    {
        system->OnSceneLoaded();
    }
}

void Scene::SetCurrentCamera(Camera* camera)
{
    SafeRelease(mainCamera);
    mainCamera = SafeRetain(camera);
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(camera);
}

Camera* Scene::GetCurrentCamera() const
{
    return mainCamera;
}

void Scene::SetCustomDrawCamera(Camera* camera)
{
    SafeRelease(drawCamera);
    drawCamera = SafeRetain(camera);
}

Camera* Scene::GetDrawCamera() const
{
    return drawCamera;
}

EntitiesManager* Scene::GetEntitiesManager() const
{
    return entitiesManager;
}

EventSystem* Scene::GetEventSystem() const
{
    return eventSystem;
}

RenderSystem* Scene::GetRenderSystem() const
{
    return renderSystem;
}

SceneFileV2::eError Scene::LoadScene(const FilePath& pathname)
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

SceneFileV2::eError Scene::SaveScene(const FilePath& pathname, bool saveForGame /*= false*/)
{
    std::function<void(Entity*)> resolveId = [&](Entity* entity)
    {
        if (0 == entity->id)
        {
            entity->id = ++maxEntityIDCounter;
        }
        for (Entity* child : entity->children)
        {
            resolveId(child);
        }
    };

    resolveId(this);

    ScopedPtr<SceneFileV2> file(new SceneFileV2());
    file->EnableDebugLog(false);
    file->EnableSaveForGame(saveForGame);
    return file->SaveScene(pathname, this);
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

    for (NMaterial* material : materials)
    {
        RemoveMaterialFlag(material, NMaterialFlagName::FLAG_ILLUMINATION_USED);
        RemoveMaterialFlag(material, NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_CASTER);
        RemoveMaterialFlag(material, NMaterialFlagName::FLAG_ILLUMINATION_SHADOW_RECEIVER);

        if (material->HasLocalProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE))
        {
            material->RemoveProperty(NMaterialParamName::PARAM_LIGHTMAP_SIZE);
        }
    }

    Entity::OptimizeBeforeExport();
}

void Scene::OnSceneReady(Entity* rootNode)
{
}

void Scene::Input(UIEvent* event)
{
    auto inputProcessesCopy = inputProcesses; // Iterate over copy to allow immediate systems creation.

    for (const ProcessSystemPair& p : inputProcessesCopy)
    {
        const AnyFn& inputProcess = p.first->process;
        SceneSystem* system = p.second;

        const bool inputHandled = inputProcess.InvokeWithCast(system, event).Get<bool>();

        entitiesManager->UpdateCaches();

        if (inputHandled)
        {
            break;
        }
    }
}

void Scene::InputCancelled(UIEvent* event)
{
    auto inputProcessesCopy = inputProcesses; // Iterate over copy to allow immediate systems creation.

    for (const ProcessSystemPair& p : inputProcessesCopy)
    {
        SceneSystem* system = p.second;

        system->InputCancelled(event);

        entitiesManager->UpdateCaches();
    }
}

void Scene::HandleEvent(Observable* observable)
{
    //                                   DEPRECATED. STOP ADDING STUFF HERE.

    RenderOptions* options = dynamic_cast<RenderOptions*>(observable);

    if (options->IsOptionEnabled(RenderOptions::REPLACE_LIGHTMAP_MIPMAPS))
    {
        MipMapReplacer::ReplaceMipMaps(this, NMaterialTextureName::TEXTURE_LIGHTMAP);
    }
    if (options->IsOptionEnabled(RenderOptions::REPLACE_ALBEDO_MIPMAPS))
    {
        MipMapReplacer::ReplaceMipMaps(this, NMaterialTextureName::TEXTURE_ALBEDO);
    }

    const FastName staticOcclusionDebugTag("static_occlusion_debug");
    const FastName particleEffectDebugTag("particle_effect_debug");

    if (options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && nullptr == GetSystem<StaticOcclusionDebugDrawSystem>())
    {
        (!sceneTags.empty() || systemsMap.empty()) ? AddTags(staticOcclusionDebugTag) : AddSystem(Type::Instance<StaticOcclusionDebugDrawSystem>());
    }
    else if (!options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_STATIC_OCCLUSION) && nullptr != GetSystem<StaticOcclusionDebugDrawSystem>())
    {
        !sceneTags.empty() ? RemoveTags(staticOcclusionDebugTag) : RemoveSystem(Type::Instance<StaticOcclusionDebugDrawSystem>());
    }

    if (options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_PARTICLES) && nullptr == GetSystem<ParticleEffectDebugDrawSystem>())
    {
        (!sceneTags.empty() || systemsMap.empty()) ? AddTags(particleEffectDebugTag) : AddSystem(Type::Instance<ParticleEffectDebugDrawSystem>());
    }
    else if (!options->IsOptionEnabled(RenderOptions::DEBUG_DRAW_PARTICLES) && nullptr != GetSystem<ParticleEffectDebugDrawSystem>())
    {
        !sceneTags.empty() ? RemoveTags(particleEffectDebugTag) : RemoveSystem(Type::Instance<ParticleEffectDebugDrawSystem>());
    }

    //                                   DEPRECATED. STOP ADDING STUFF HERE.
}

void Scene::Activate()
{
    for (SceneSystem* system : systemsVector)
    {
        system->Activate();
    }
}

void Scene::Deactivate()
{
    for (SceneSystem* system : systemsVector)
    {
        system->Deactivate();
    }
}

void Scene::PauseFixedUpdate()
{
    fixedUpdate.paused = true;
}

void Scene::UnpauseFixedUpdate()
{
    fixedUpdate.paused = false;
}

bool Scene::IsFixedUpdatePaused() const
{
    return fixedUpdate.paused;
}

const Vector<SceneSystem*>& Scene::GetSystems() const
{
    return systemsVector;
}

#if defined(__DAVAENGINE_DEBUG__)
void Scene::HandleSingleComponentRead(const SceneSystem* system, const Type* singleComponentType)
{
    DVASSERT(nullptr != system && nullptr != singleComponentType);
    const bool successfulInsert = clearableSingleComponentsReaders[singleComponentType].insert(system).second;
    if (successfulInsert)
    {
        pendingSingleComponentsAccessValidation = true;
    }
}

void Scene::HandleSingleComponentWrite(const SceneSystem* system, const Type* singleComponentType)
{
    DVASSERT(nullptr != system && nullptr != singleComponentType);
    const bool successfulInsert = clearableSingleComponentsWriters[singleComponentType].insert(system).second;
    if (successfulInsert)
    {
        pendingSingleComponentsAccessValidation = true;
    }
}

void Scene::UnregisterSingleComponentUser(const SceneSystem* system)
{
    DVASSERT(nullptr != system);

    for (auto& p : clearableSingleComponentsReaders)
    {
        p.second.erase(system);
    }

    for (auto& p : clearableSingleComponentsWriters)
    {
        p.second.erase(system);
    }
}

void Scene::ValidateSingleComponentsAccessOrder()
{
    if (!pendingSingleComponentsAccessValidation)
    {
        return;
    }

    pendingSingleComponentsAccessValidation = false;

    Vector<ProcessSystemPair> allProcesses;
    allProcesses.reserve(fixedProcesses.size() + processes.size() + inputProcesses.size());

    for (const auto& v : { &fixedProcesses, &processes, &inputProcesses })
    {
        allProcesses.insert(cend(allProcesses), cbegin(*v), cend(*v));
    }

    UnorderedMap<const SceneSystem*, std::pair<uint32, uint32>> indexCache;

    auto minMaxIndex = [&allProcesses, &indexCache](const SceneSystem* system)
    {
        const auto cacheIt = indexCache.find(system);

        if (cacheIt != cend(indexCache))
        {
            return cacheIt->second;
        }

        uint32 minIndex = std::numeric_limits<uint32>::max();
        uint32 maxIndex = std::numeric_limits<uint32>::max();

        auto it = std::find_if(cbegin(allProcesses), cend(allProcesses), [system](const ProcessSystemPair& p) { return system == p.second; });

        if (it != cend(allProcesses))
        {
            auto maxIt = std::find_if(crbegin(allProcesses), std::make_reverse_iterator(it), [system](const ProcessSystemPair& p) { return system == p.second; });
            minIndex = static_cast<uint32>(std::distance(cbegin(allProcesses), it));
            maxIndex = static_cast<uint32>(std::distance(cbegin(allProcesses), (maxIt + 1).base()));
        }

        return (indexCache[system] = std::make_pair(minIndex, maxIndex));
    };

    uint32 fixedProcessesSize = static_cast<uint32>(fixedProcesses.size());

    for (ClearableSingleComponent* singleComponent : clearableSingleComponentsFixed)
    {
        for (const auto& singleComponentUser : { &clearableSingleComponentsReaders, &clearableSingleComponentsWriters })
        {
            const auto it = singleComponentUser->find(singleComponent->GetType());

            if (it != cend(*singleComponentUser))
            {
                for (const SceneSystem* system : it->second)
                {
                    const Type* const systemType = ReflectedTypeDB::GetByPointer(system)->GetType();
                    const uint32 maxIndex = minMaxIndex(system).second;
                    DVASSERT(maxIndex < fixedProcessesSize || std::numeric_limits<uint32>::max() == maxIndex, Format("Single component (%s), which is cleared after all fixed processes, is used by a system (%s) that has a non-fixed process method", singleComponent->GetType()->GetName(), systemType->GetName()).c_str());
                }
            }
        }
    }

    if (clearableSingleComponentsReaders.empty() || clearableSingleComponentsWriters.empty())
    {
        return;
    }

    for (const auto& p : clearableSingleComponentsWriters)
    {
        if (!p.second.empty())
        {
            const auto it = clearableSingleComponentsReaders.find(p.first);

            if (it != cend(clearableSingleComponentsReaders) && !it->second.empty())
            {
                int64 maxWriteIndex = -1;
                const SceneSystem* maxWriteSystem = nullptr;

                for (const SceneSystem* system : p.second)
                {
                    const int64 writeIndex = static_cast<int64>(minMaxIndex(system).second);
                    if (std::numeric_limits<uint32>::max() != writeIndex && writeIndex > maxWriteIndex)
                    {
                        maxWriteIndex = writeIndex;
                        maxWriteSystem = system;
                    }
                }

                DVASSERT(nullptr != maxWriteSystem && maxWriteIndex >= 0);

                const auto minReadIndexIt = std::min_element(cbegin(it->second), cend(it->second), [&minMaxIndex](const SceneSystem* l, const SceneSystem* r) {
                    return minMaxIndex(l).first < minMaxIndex(r).first;
                });
                const uint32 minReadIndex = minMaxIndex(*minReadIndexIt).first;

                const Type* const maxWriteSystemType = ReflectedTypeDB::GetByPointer(maxWriteSystem)->GetType();
                const Type* const minReadSystemType = ReflectedTypeDB::GetByPointer(*minReadIndexIt)->GetType();

                DVASSERT(minReadIndex > static_cast<uint32>(maxWriteIndex), Format("Detected read access by system `%s` to single component `%s` before last write to this component (which is performed by `%s`)", minReadSystemType->GetName(), p.first->GetName(), maxWriteSystemType->GetName()).c_str());
            }
        }
    }
}
#endif
} // namespace DAVA
