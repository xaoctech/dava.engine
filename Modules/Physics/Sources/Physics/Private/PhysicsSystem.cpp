#include "Physics/Private/PhysicsSystem.h"
#include "Physics/PhysicsModule.h"
#include "Physics/PhysicsConfigs.h"
#include "Physics/PhysicsComponent.h"
#include "Physics/CollisionComponent.h"

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
namespace PhysicsSystemDetail
{
template <typename T>
void EraseComponent(T* component, Vector<T*>& pendingComponents, Vector<T*>& components)
{
    auto addIter = std::find(pendingComponents.begin(), pendingComponents.end(), component);
    if (addIter != pendingComponents.end())
    {
        RemoveExchangingWithLast(pendingComponents, std::distance(pendingComponents.begin(), addIter));
    }
    else
    {
        auto iter = std::find(components.begin(), components.end(), component);
        DVASSERT(iter != components.end());
        RemoveExchangingWithLast(components, std::distance(components.begin(), iter));
    }
}
} // namespace

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
    auto processEntity = [this](Entity* entity, uint32 componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            RegisterComponent(entity, entity->GetComponent(componentType, i));
        }
    };

    processEntity(entity, Component::STATIC_BODY_COMPONENT);
    processEntity(entity, Component::DYNAMIC_BODY_COMPONENT);
    processEntity(entity, Component::COLLISION_COMPONENT);
}

void PhysicsSystem::UnregisterEntity(Entity* entity)
{
    auto processEntity = [this](Entity* entity, uint32 componentType)
    {
        for (uint32 i = 0; i < entity->GetComponentCount(componentType); ++i)
        {
            UnregisterComponent(entity, entity->GetComponent(componentType, i));
        }
    };

    processEntity(entity, Component::STATIC_BODY_COMPONENT);
    processEntity(entity, Component::DYNAMIC_BODY_COMPONENT);
    processEntity(entity, Component::COLLISION_COMPONENT);
}

void PhysicsSystem::RegisterComponent(Entity* entity, Component* component)
{
    uint32 componentType = component->GetType();
    if (componentType == Component::STATIC_BODY_COMPONENT || componentType == Component::DYNAMIC_BODY_COMPONENT)
    {
        pendingAddPhysicsComponents.push_back(static_cast<PhysicsComponent*>(component));
    }

    if (componentType == Component::COLLISION_COMPONENT)
    {
        pendingAddCollisionComponents.push_back(static_cast<CollisionComponent*>(component));
    }
}

void PhysicsSystem::UnregisterComponent(Entity* entity, Component* component)
{
    uint32 componentType = component->GetType();
    if (componentType == Component::STATIC_BODY_COMPONENT || componentType == Component::DYNAMIC_BODY_COMPONENT)
    {
        PhysicsComponent* physicsComponent = static_cast<PhysicsComponent*>(component);
        PhysicsSystemDetail::EraseComponent(physicsComponent, pendingAddPhysicsComponents, physicsComponents);

        physx::PxRigidActor* actor = physicsComponent->GetPxActor()->is<physx::PxRigidActor>();
        if (actor != nullptr)
        {
            physx::PxU32 shapesCount = actor->getNbShapes();
            Vector<physx::PxShape*> shapes(shapesCount, nullptr);
            actor->getShapes(shapes.data(), shapesCount);

            for (physx::PxShape* shape : shapes)
            {
                DVASSERT(shape != nullptr);
                actor->detachShape(*shape);
            }

            physicsScene->removeActor(*physicsComponent->GetPxActor());
            physicsComponent->ReleasePxActor();
        }
    }

    if (componentType == Component::COLLISION_COMPONENT)
    {
        CollisionComponent* collisionComponent = static_cast<CollisionComponent*>(component);
        PhysicsSystemDetail::EraseComponent(collisionComponent, pendingAddCollisionComponents, collisionComponents);

        physx::PxShape* shape = collisionComponent->GetPxShape();
        if (shape != nullptr)
        {
            physx::PxRigidActor* actor = shape->getActor();
            if (actor != nullptr)
            {
                actor->detachShape(*shape);
            }
            collisionComponent->ReleasePxShape();
        }
    }
}

void PhysicsSystem::Process(float32 timeElapsed)
{
    if (isSimulationRunning && FetchResults(false))
    {
        isSimulationRunning = false;
    }

    Physics* physics = GetEngineContext()->moduleManager->GetModule<Physics>();
    for (PhysicsComponent* component : pendingAddPhysicsComponents)
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

        Entity* entity = component->GetEntity();
        for (uint32 i = 0; i < entity->GetComponentCount(Component::COLLISION_COMPONENT); ++i)
        {
            CollisionComponent* collision = static_cast<CollisionComponent*>(entity->GetComponent(Component::COLLISION_COMPONENT, i));
            physx::PxShape* shape = collision->GetPxShape();
            if (shape != nullptr)
            {
                physx::PxRigidActor* rigidActor = createdActor->is<physx::PxRigidActor>();
                rigidActor->attachShape(*shape);
            }
        }

        physicsScene->addActor(*(component->GetPxActor()));
        physicsComponents.push_back(component);
    }
    pendingAddPhysicsComponents.clear();

    auto attachShapeFn = [](physx::PxShape* shape, PhysicsComponent* component)
    {
        if (component != nullptr)
        {
            physx::PxRigidActor* actor = component->GetPxActor()->is<physx::PxRigidActor>();
            DVASSERT(actor != nullptr);
            actor->attachShape(*shape);
        }
    };

    for (CollisionComponent* component : pendingAddCollisionComponents)
    {
        physx::PxShape* shape = physics->CreateBoxShape(true);
        component->SetPxShape(shape);

        Entity* entity = component->GetEntity();
        attachShapeFn(shape, static_cast<PhysicsComponent*>(entity->GetComponent(Component::STATIC_BODY_COMPONENT, 0)));
        attachShapeFn(shape, static_cast<PhysicsComponent*>(entity->GetComponent(Component::DYNAMIC_BODY_COMPONENT, 0)));

        collisionComponents.push_back(component);
    }
    pendingAddCollisionComponents.clear();

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
