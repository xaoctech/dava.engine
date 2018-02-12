#include "PhysicsProjectileSystem.h"

#include "Components/PhysicsProjectileComponent.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Time/SystemTimer.h>

#include <NetworkCore/Scene3D/Components/NetworkTransformComponent.h>
#include <Physics/CollisionSingleComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(PhysicsProjectileSystem)
{
    ReflectionRegistrator<PhysicsProjectileSystem>::Begin()[M::Tags("gm_characters", "physics")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &PhysicsProjectileSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_END, SP::Type::FIXED, 1.0f)]
    .End();
}

namespace PhysicsProjectileSystemDetail
{
template <typename T>
static bool CompareTransform(const T& lhs, const T& rhs, uint32 size, float32 epsilon, uint32 frameId)
{
    for (uint32 i = 0; i < size; ++i)
    {
        if (!FLOAT_EQUAL_EPS(lhs.data[i], rhs.data[i], epsilon))
        {
            Logger::Debug("Transforms aren't equal (compared by physics projectile system), diff: %f, index: %d, frame: %d", std::abs(lhs.data[i] - rhs.data[i]), i, frameId);

            return false;
        }
    }
    return true;
}
}

PhysicsProjectileSystem::PhysicsProjectileSystem(Scene* scene)
    : BaseSimulationSystem(scene, ComponentUtils::MakeMask<NetworkTransformComponent>() | ComponentUtils::MakeMask<PhysicsProjectileComponent>())
{
}

void PhysicsProjectileSystem::Simulate(Entity* entity)
{
    PhysicsProjectileComponent* projectileComponent = entity->GetComponent<PhysicsProjectileComponent>();
    DVASSERT(projectileComponent != nullptr);

    NextState(entity, projectileComponent);
}

void PhysicsProjectileSystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("PhysicsProjectileSystem::ProcessFixed");

    Vector<Entity*> destroyedEntities;
    for (const auto& projectile : entities)
    {
        Simulate(projectile);

        PhysicsProjectileComponent* projectileComponent = projectile->GetComponent<PhysicsProjectileComponent>();

        if (projectileComponent->GetProjectileState() == PhysicsProjectileComponent::eProjectileStates::DESTROYED)
        {
            destroyedEntities.push_back(projectile);
        }
    }

    for (auto entity : destroyedEntities)
    {
        GetScene()->RemoveNode(entity);
    }
}

void PhysicsProjectileSystem::NextState(DAVA::Entity* entity, PhysicsProjectileComponent* projectileComponent)
{
    PhysicsProjectileComponent::eProjectileStates currentState = projectileComponent->GetProjectileState();
    switch (currentState)
    {
    case PhysicsProjectileComponent::eProjectileStates::FLYING:
    {
        CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingletonComponent<CollisionSingleComponent>();
        const bool collided = collisionSingleComponent->GetCollisionsWithEntity(entity).size() > 0;

        if (collided)
        {
            if (projectileComponent->GetProjectileType() == PhysicsProjectileComponent::eProjectileTypes::MISSILE)
            {
                projectileComponent->SetProjectileState(PhysicsProjectileComponent::eProjectileStates::DESTROYED);
            }
            else
            {
                projectileComponent->SetProjectileState(PhysicsProjectileComponent::eProjectileStates::DETONATION_TIMING);
                grenadeDetonationTimers[entity] = 0.0f;
            }
        }
        else
        {
            Vector3 currentPosition = entity->GetComponent<TransformComponent>()->GetPosition();
            float32 distance = Distance(currentPosition, projectileComponent->GetInitialPosition());
            if (distance > 100.0f)
            {
                projectileComponent->SetProjectileState(PhysicsProjectileComponent::eProjectileStates::DESTROYED);
            }
        }
    }
    break;

    case PhysicsProjectileComponent::eProjectileStates::DETONATION_TIMING:
    {
        float32& timer = grenadeDetonationTimers[entity];
        timer += SystemTimer::GetRealFrameDelta();

        if (timer >= 3.0f)
        {
            projectileComponent->SetProjectileState(PhysicsProjectileComponent::eProjectileStates::DESTROYED);
            grenadeDetonationTimers.erase(entity);
        }
    }
    break;

    case PhysicsProjectileComponent::eProjectileStates::DESTROYED:
    {
    }
    break;
    }
}
