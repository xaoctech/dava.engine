#include "Systems/ShooterCarAttackSystem.h"
#include "Components/HealthComponent.h"
#include "ShooterConstants.h"
#include "ShooterUtils.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/CollisionSingleComponent.h>
#include <Physics/Vehicles/VehicleCarComponent.h>
#include <Physics/Vehicles/VehicleChassisComponent.h>
#include <Physics/Vehicles/VehicleWheelComponent.h>
#include <Physics/Core/DynamicBodyComponent.h>
#include <Physics/Core/CapsuleShapeComponent.h>
#include <Physics/Controllers/CapsuleCharacterControllerComponent.h>
#include <Physics/Core/PhysicsUtils.h>

#include <NetworkPhysics/CharacterMirrorsSingleComponent.h>
#include <NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h>

namespace ShooterCarAttackSystemDetail
{
bool IsCar(DAVA::Entity* entity)
{
    using namespace DAVA;

    return
    entity->GetComponent<VehicleCarComponent>() != nullptr ||
    entity->GetComponent<VehicleChassisComponent>() != nullptr ||
    entity->GetComponent<VehicleWheelComponent>() != nullptr;
}
}

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterCarAttackSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<ShooterCarAttackSystem>::Begin()[M::Tags("gm_shooter", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterCarAttackSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 19.0f)]
    .End();
}

ShooterCarAttackSystem::ShooterCarAttackSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, DAVA::ComponentMask())
    , ccts(scene->AquireComponentGroup<DAVA::CapsuleCharacterControllerComponent, DAVA::CapsuleCharacterControllerComponent>())
{
}

void ShooterCarAttackSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;
    using namespace ShooterCarAttackSystemDetail;

    const CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingleComponentForRead<CollisionSingleComponent>(this);
    DVASSERT(collisionSingleComponent != nullptr);

    static Vector<CapsuleCharacterControllerComponent*> pushedCctsToRemove;

    for (auto& kvp : pushedCcts)
    {
        CapsuleCharacterControllerComponent* cct = kvp.first;
        if (cct->IsGrounded())
        {
            cct->SetMovementMode(CharacterControllerComponent::MovementMode::Walking);
            pushedCctsToRemove.push_back(cct);
        }
    }

    for (CapsuleCharacterControllerComponent* pushedCctToRemove : pushedCctsToRemove)
    {
        pushedCcts.erase(pushedCctToRemove);
    }
    pushedCctsToRemove.clear();

    for (auto& kvp : pushedCcts)
    {
        CapsuleCharacterControllerComponent* cct = kvp.first;
        Vector3& velocity = kvp.second;

        velocity.z -= 9.81f * dt;
        cct->Move(velocity * dt);
    }

    for (CapsuleCharacterControllerComponent* cctComponent : ccts->components)
    {
        Entity* cctEntity = cctComponent->GetEntity();
        DVASSERT(cctEntity != nullptr);

        DynamicBodyComponent* cctMirrorBodyComponent = cctEntity->GetComponent<DynamicBodyComponent>();
        DVASSERT(cctMirrorBodyComponent != nullptr);

        if (pushedCcts.find(cctComponent) == pushedCcts.end())
        {
            Vector<CollisionInfo> collisions = collisionSingleComponent->GetCollisionsWithEntity(cctEntity);
            if (collisions.size() > 0)
            {
                float32 impulseMagnitude = 0.0f;
                for (const CollisionInfo& collision : collisions)
                {
                    if (IsCar(collision.first) || IsCar(collision.second))
                    {
                        for (const CollisionPoint& collisionPoint : collision.points)
                        {
                            impulseMagnitude += collisionPoint.impulse.Length();
                        }
                    }
                }

                uint32 damage = static_cast<uint32>(impulseMagnitude / SHOOTER_CAR_IMPULSE_MAGNITUDE_PER_DAMAGE);

                if (damage > 0)
                {
                    HealthComponent* healthComponent = cctComponent->GetEntity()->GetComponent<HealthComponent>();
                    const uint32 frameId = GetScene()->GetSingleComponent<NetworkTimeSingleComponent>()->GetFrameId();
                    DVASSERT(healthComponent != nullptr);

                    healthComponent->DecHealth(damage, frameId);

                    if (healthComponent->GetHealth() > 0)
                    {
                        Vector3 normal = collisions[0].points[0].normal;
                        if (damage >= 2)
                        {
                            normal.z = 0.75f;
                            normal.Normalize();
                        }

                        Vector3 velocity = normal * static_cast<float32>(damage) * 2.0f;
                        if (IsCar(collisions[0].first))
                        {
                            velocity.x *= -1.0f;
                            velocity.y *= -1.0f;
                        }

                        cctComponent->SetMovementMode(CharacterControllerComponent::MovementMode::Flying);
                        pushedCcts[cctComponent] = velocity;
                    }
                }
            }
        }
    }
}

void ShooterCarAttackSystem::PrepareForRemove()
{
}
