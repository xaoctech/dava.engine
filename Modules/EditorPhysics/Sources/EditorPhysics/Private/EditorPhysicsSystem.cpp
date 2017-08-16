#include "EditorPhysics/Private/EditorPhysicsSystem.h"

#include <Physics/PhysicsSystem.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/SphereShapeComponent.h>

#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/SingleComponents/CollisionSingleComponent.h>
#include <Entity/Component.h>

#include <physx/PxRigidDynamic.h>

EditorPhysicsSystem::EditorPhysicsSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
    scene->physicsSystem->SetDebugDrawEnabled(true);
    scene->physicsSystem->SetSimulationEnabled(false);
}

void EditorPhysicsSystem::RegisterEntity(DAVA::Entity* entity)
{
    if (entity->GetComponentCount(DAVA::Component::DYNAMIC_BODY_COMPONENT) > 0 ||
        entity->GetComponentCount(DAVA::Component::STATIC_BODY_COMPONENT) > 0)
    {
        EntityInfo& info = transformMap[entity];
        info.originalTransform = entity->GetWorldTransform();
        info.isLocked = entity->GetLocked();
    }
}

void EditorPhysicsSystem::UnregisterEntity(DAVA::Entity* entity)
{
    auto iter = transformMap.find(entity);
    if (iter != transformMap.end())
    {
        transformMap.erase(iter);
    }
}

void EditorPhysicsSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    DAVA::uint32 componentType = component->GetType();
    if (componentType == DAVA::Component::STATIC_BODY_COMPONENT ||
        componentType == DAVA::Component::DYNAMIC_BODY_COMPONENT)
    {
        RegisterEntity(entity);
    }
}

void EditorPhysicsSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    DAVA::uint32 componentType = component->GetType();
    if (componentType == DAVA::Component::STATIC_BODY_COMPONENT ||
        componentType == DAVA::Component::DYNAMIC_BODY_COMPONENT)
    {
        DAVA::uint32 actorCount = entity->GetComponentCount(DAVA::Component::STATIC_BODY_COMPONENT);
        actorCount += entity->GetComponentCount(DAVA::Component::DYNAMIC_BODY_COMPONENT);
        if (actorCount == 1)
        {
            UnregisterEntity(entity);
        }
    }
}

void EditorPhysicsSystem::Process(DAVA::float32 timeElapsed)
{
    using namespace DAVA;

    Scene* scene = GetScene();
    while (pendingRemove.size() > 50)
    {
        auto iter = pendingRemove.begin();
        Entity* e = *iter;
        pendingRemove.erase(iter);
        scene->RemoveNode(e);
    }

    if (state != eSimulationState::STOPPED)
    {
        for (auto& node : transformMap)
        {
            if (node.first->GetLocked() == false)
            {
                node.first->SetLocked(true);
            }
        }
    }

    CollisionSingleComponent* collisions = GetScene()->collisionSingleComponent;
    if (collisions != nullptr)
    {
        for (CollisionInfo& collisionPair : collisions->collisions)
        {
            SphereShapeComponent* sphere1 = static_cast<SphereShapeComponent*>(collisionPair.first->GetComponent(Component::SPHERE_SHAPE_COMPONENT, 0));
            if (sphere1 != nullptr && sphere1->GetName() == FastName("bullet"))
            {
                pendingRemove.emplace(collisionPair.first);
            }

            SphereShapeComponent* sphere2 = static_cast<SphereShapeComponent*>(collisionPair.second->GetComponent(Component::SPHERE_SHAPE_COMPONENT, 0));
            if (sphere2 != nullptr && sphere2->GetName() == FastName("bullet"))
            {
                pendingRemove.emplace(collisionPair.second);
            }
        }
    }
}

bool EditorPhysicsSystem::Input(DAVA::UIEvent* uie)
{
    if (GetSimulationState() == eSimulationState::PLAYING)
    {
        if (uie->phase == DAVA::UIEvent::Phase::ENDED && uie->mouseButton == DAVA::eMouseButtons::LEFT)
        {
            DAVA::Scene* scene = GetScene();

            static DAVA::int32 bulletIndex = 0;
            DAVA::Entity* bullet = new DAVA::Entity();
            bullet->SetName(DAVA::FastName(DAVA::Format("bullet_%d", bulletIndex++)));

            DAVA::Camera* camera = scene->GetCurrentCamera();
            DAVA::Vector3 position = camera->GetPosition() + (5 * camera->GetDirection());
            DAVA::Matrix4 bulletTransform = DAVA::Matrix4::MakeTranslation(position);
            bullet->SetLocalTransform(bulletTransform);

            DAVA::DynamicBodyComponent* bodyComponent = new DAVA::DynamicBodyComponent();
            bodyComponent->SetCCDEnabled(true);
            bullet->AddComponent(bodyComponent);

            DAVA::SphereShapeComponent* bulletShape = new DAVA::SphereShapeComponent();
            bulletShape->SetName(DAVA::FastName("bullet"));
            bulletShape->SetRadius(0.25f);
            bullet->AddComponent(bulletShape);

            scene->AddNode(bullet);

            scene->physicsSystem->AddForce(bodyComponent, 3 * camera->GetDirection(), physx::PxForceMode::eIMPULSE);
        }

        return true;
    }

    return false;
}

void EditorPhysicsSystem::SetSimulationState(eSimulationState newState)
{
    if (newState == state)
    {
        return;
    }

    DAVA::PhysicsSystem* physicsSystem = GetScene()->physicsSystem;

    switch (newState)
    {
    case eSimulationState::PLAYING:
        DVASSERT(physicsSystem->IsSimulationEnabled() == false);
        if (state == eSimulationState::STOPPED)
        {
            StoreActualTransform();
        }
        physicsSystem->SetSimulationEnabled(true);
        break;
    case eSimulationState::PAUSED:
        DVASSERT(physicsSystem->IsSimulationEnabled() == true);
        DVASSERT(state == eSimulationState::PLAYING);
        physicsSystem->SetSimulationEnabled(false);
        break;
    case eSimulationState::STOPPED:
        DVASSERT(physicsSystem->IsSimulationEnabled() == true || state == eSimulationState::PAUSED);
        physicsSystem->SetSimulationEnabled(false);
        RestoreTransform();
        break;
    default:
        break;
    }

    state = newState;
}

EditorPhysicsSystem::eSimulationState EditorPhysicsSystem::GetSimulationState() const
{
    return state;
}

void EditorPhysicsSystem::StoreActualTransform()
{
    for (auto& node : transformMap)
    {
        node.second.originalTransform = node.first->GetWorldTransform();
        node.second.isLocked = node.first->GetLocked();
    }
}

void EditorPhysicsSystem::RestoreTransform()
{
    for (auto& node : transformMap)
    {
        node.first->SetWorldTransform(node.second.originalTransform);
        node.first->SetLocked(node.second.isLocked);

        DAVA::PhysicsComponent* component = static_cast<DAVA::PhysicsComponent*>(node.first->GetComponent(DAVA::Component::DYNAMIC_BODY_COMPONENT, 0));
        if (component != nullptr)
        {
            physx::PxRigidDynamic* actor = component->GetPxActor()->is<physx::PxRigidDynamic>();
            if (actor != nullptr && actor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION) == false)
            {
                actor->setLinearVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
                actor->setAngularVelocity(physx::PxVec3(0.0f, 0.0f, 0.0f));
            }
        }
    }
}
