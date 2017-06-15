#include "Physics/Private/PhysicsSystem.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsConfigs.h"
#include "Physics/PhysicsComponent.h"

#include <Scene3D/Entity.h>
#include <Entity/Component.h>

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <ModuleManager/ModuleManager.h>
#include <FileSystem/KeyedArchive.h>
#include <Utils/Utils.h>

#include <physx/PxScene.h>
#include <physx/PxRigidActor.h>
#include <PxShared/foundation/PxAllocatorCallback.h>
#include <PxShared/foundation/PxFoundation.h>

namespace DAVA
{
PhysicsSystem::PhysicsSystem(Scene* scene)
    : SceneSystem(scene)
{
    const KeyedArchive* options = Engine::Instance()->GetOptions();

    simulationBlockSize = options->GetUInt32("physics.simulationBlockSize", 16 * 1024 * 512);
    DVASSERT((simulationBlockSize % (16 * 1024)) == 0);

    const EngineContext* ctx = GetEngineContext();
    Physics* physics = ctx->moduleManager->GetModule<Physics>();
    physx::PxAllocatorCallback& allocator = physics->GetFoundation()->getAllocatorCallback();
    simulationBlock = allocator.allocate(simulationBlockSize, "SimulationBlock", __FILE__, __LINE__);

    PhysicsSceneConfig sceneConfig;
    sceneConfig.gravity = options->GetVector3("physics.gravity", Vector3(0, 0, -9.81f));
    sceneConfig.threadCount = options->GetUInt32("physics.threadCount", 2);
    physicsScene = physics->CreateScene(sceneConfig);
}

PhysicsSystem::~PhysicsSystem()
{
    if (isSimulationRunning)
    {
        FetchResults(true);
    }
    DVASSERT(simulationBlock != nullptr);

    const EngineContext* ctx = GetEngineContext();
    Physics* physics = ctx->moduleManager->GetModule<Physics>();
    physx::PxAllocatorCallback& allocator = physics->GetFoundation()->getAllocatorCallback();
    allocator.deallocate(simulationBlock);
    simulationBlock = nullptr;
    physicsScene->release();
}

void PhysicsSystem::RegisterEntity(Entity* entity)
{
    for (uint32 i = 0; i < entity->GetComponentCount(Component::STATIC_BODY_COMPONENT); ++i)
    {
        RegisterComponent(entity, entity->GetComponent(Component::STATIC_BODY_COMPONENT, i));
    }

    for (uint32 i = 0; i < entity->GetComponentCount(Component::DYNAMIC_BODY_COMPONENT); ++i)
    {
        RegisterComponent(entity, entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT, i));
    }
}

void PhysicsSystem::UnregisterEntity(Entity* entity)
{
    for (uint32 i = 0; i < entity->GetComponentCount(Component::STATIC_BODY_COMPONENT); ++i)
    {
        UnregisterComponent(entity, entity->GetComponent(Component::STATIC_BODY_COMPONENT, i));
    }

    for (uint32 i = 0; i < entity->GetComponentCount(Component::DYNAMIC_BODY_COMPONENT); ++i)
    {
        UnregisterComponent(entity, entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT, i));
    }
}

void PhysicsSystem::RegisterComponent(Entity* entity, Component* component)
{
    uint32 componentType = component->GetType();
    if (componentType == Component::STATIC_BODY_COMPONENT || componentType == Component::DYNAMIC_BODY_COMPONENT)
    {
        pendingAddComponents.push_back(static_cast<PhysicsComponent*>(component));
    }
}

void PhysicsSystem::UnregisterComponent(Entity* entity, Component* component)
{
    uint32 componentType = component->GetType();
    if (componentType == Component::STATIC_BODY_COMPONENT || componentType == Component::DYNAMIC_BODY_COMPONENT)
    {
        PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(component);
        auto addIter = std::find(pendingAddComponents.begin(), pendingAddComponents.end(), physicsComponent);
        if (addIter != pendingAddComponents.end())
        {
            RemoveExchangingWithLast(pendingAddComponents, std::distance(pendingAddComponents.begin(), addIter));
        }
        else
        {
            auto iter = std::find(components.begin(), components.end(), physicsComponent);
            DVASSERT(iter != components.end());
            RemoveExchangingWithLast(components, std::distance(components.begin(), iter));
        }

        physicsScene->removeActor(*physicsComponent->GetPxActor());
        physicsComponent->ReleasePxActor();
    }
}

void PhysicsSystem::Process(float32 timeElapsed)
{
    if (isSimulationRunning && FetchResults(false))
    {
        isSimulationRunning = false;
    }

    Physics* physics = GetEngineContext()->moduleManager->GetModule<Physics>();
    for (PhysicsComponent* component : pendingAddComponents)
    {
        uint32 componentType = component->GetType();
        physx::PxActor* createdActor = nullptr;
        if (componentType == Component::STATIC_BODY_COMPONENT)
        {
            createdActor = physics->CreateStaticActor();
        }
        else
        {
            DVASSERT(componentType == Component::DYNAMIC_BODY_COMPONENT);
            createdActor = physics->CreateDynamicActor();
        }

        component->SetPxActor(createdActor);
        physicsScene->addActor(*(component->GetPxActor()));
        components.push_back(component);
    }
    pendingAddComponents.clear();

    if (isSimulationEnabled == false)
    {
        return;
    }

    if (isSimulationRunning == false)
    {
        physicsScene->simulate(timeElapsed, nullptr, simulationBlock, simulationBlockSize);
        isSimulationRunning = true;
    }
}

void PhysicsSystem::SetSimulationEnabled(bool isEnabled)
{
    if (isSimulationEnabled != isEnabled)
    {
        if (isSimulationRunning == true)
        {
            DVASSERT(isSimulationEnabled == true);
            bool success = FetchResults(true);
            DVASSERT(success == true);
        }

        isSimulationEnabled = isEnabled;
    }
}

bool PhysicsSystem::IsSimulationEnabled() const
{
    return isSimulationEnabled;
}

bool PhysicsSystem::FetchResults(bool block)
{
    DVASSERT(isSimulationRunning);
    return physicsScene->fetchResults(block);
}

} // namespace DAVA
