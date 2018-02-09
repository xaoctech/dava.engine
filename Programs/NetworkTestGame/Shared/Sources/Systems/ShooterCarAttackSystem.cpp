#include "Systems/ShooterCarAttackSystem.h"
#include "Components/HealthComponent.h"
#include "ShooterConstants.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

#include <Physics/PhysicsSystem.h>
#include <Physics/CollisionSingleComponent.h>
#include <Physics/VehicleCarComponent.h>
#include <Physics/VehicleChassisComponent.h>
#include <Physics/VehicleWheelComponent.h>
#include <Physics/DynamicBodyComponent.h>
#include <Physics/CapsuleShapeComponent.h>
#include <Physics/CharacterControllerComponent.h>
#include <Physics/PhysicsUtils.h>
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
    .Method("ProcessFixed", &ShooterCarAttackSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 19.0f)]
    .End();
}

ShooterCarAttackSystem::ShooterCarAttackSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene, 0)
    , ccts(scene->AquireComponentGroup<ShooterMirroredCharacterComponent, ShooterMirroredCharacterComponent>())
{
}

void ShooterCarAttackSystem::ProcessFixed(DAVA::float32 dt)
{
    using namespace DAVA;
    using namespace ShooterCarAttackSystemDetail;

    CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingletonComponent<CollisionSingleComponent>();
    DVASSERT(collisionSingleComponent != nullptr);

    CharacterMirrorsSingleComponent* mirrorsSingleComponent = GetScene()->GetSingletonComponent<CharacterMirrorsSingleComponent>();
    DVASSERT(mirrorsSingleComponent != nullptr);

    static Vector<ShooterMirroredCharacterComponent*> pushedCctsToRemove;

    for (ShooterMirroredCharacterComponent* mirroredCctComponent : ccts->components)
    {
        Entity* cctMirror = mirrorsSingleComponent->GetMirrorForCharacter(mirroredCctComponent->GetEntity());
        DVASSERT(cctMirror != nullptr);

        DynamicBodyComponent* cctMirrorBodyComponent = cctMirror->GetComponent<DynamicBodyComponent>();
        DVASSERT(cctMirrorBodyComponent != nullptr);

        CapsuleShapeComponent* cctMirrorShapeComponent = cctMirror->GetComponent<CapsuleShapeComponent>();
        DVASSERT(cctMirrorShapeComponent != nullptr);

        CharacterControllerComponent* cctComponent = PhysicsUtils::GetCharacterControllerComponent(mirroredCctComponent->GetEntity());
        if (cctComponent == nullptr)
        {
            continue;
        }

        if (pushedCcts.find(mirroredCctComponent) != pushedCcts.end())
        {
            if (cctMirrorBodyComponent->GetLinearVelocity().Length() < 1.0f)
            {
                cctMirrorBodyComponent->SetIsKinematic(true);
                cctMirrorShapeComponent->SetTypeMask(0);
                cctMirrorShapeComponent->SetTypeMaskToCollideWith(0);
                cctComponent->SetMovementMode(CharacterControllerComponent::MovementMode::Walking);
                cctComponent->SetTypeMask(SHOOTER_CHARACTER_COLLISION_TYPE);
                cctComponent->SetTypeMaskToCollideWith(UINT32_MAX);
                mirroredCctComponent->SetMirrorIsMaster(false);

                pushedCctsToRemove.push_back(mirroredCctComponent);
            }
        }
        else
        {
            Vector<CollisionInfo> collisions = collisionSingleComponent->GetCollisionsWithEntity(mirroredCctComponent->GetEntity());
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
                    HealthComponent* healthComponent = mirroredCctComponent->GetEntity()->GetComponent<HealthComponent>();
                    const uint32 frameId = GetScene()->GetSingletonComponent<NetworkTimeSingleComponent>()->GetFrameId();
                    DVASSERT(healthComponent != nullptr);

                    healthComponent->DecHealth(damage, frameId);

                    if (healthComponent->GetHealth() > 0)
                    {
                        Entity* carPart = IsCar(collisions[0].first) ? collisions[0].first : collisions[0].second;
                        DVASSERT(carPart != nullptr);

                        Vector3 normal = collisions[0].points[0].normal;
                        normal.z = 0.5f;
                        normal.Normalize();

                        Vector3 force = normal * 15000.0f * static_cast<float32>(damage);
                        if (carPart == collisions[0].first)
                        {
                            force.x *= -1.0f;
                            force.y *= -1.0f;
                        }

                        cctMirrorBodyComponent->SetIsKinematic(false);
                        cctMirrorShapeComponent->SetTypeMask(SHOOTER_CHARACTER_COLLISION_TYPE);
                        cctMirrorShapeComponent->SetTypeMaskToCollideWith(UINT32_MAX);
                        cctComponent->SetMovementMode(CharacterControllerComponent::MovementMode::Flying);
                        cctComponent->SetTypeMask(0);
                        cctComponent->SetTypeMaskToCollideWith(0);
                        mirroredCctComponent->SetMirrorIsMaster(true);

                        GetScene()->GetSystem<PhysicsSystem>()->AddForce(cctMirrorBodyComponent, force, physx::PxForceMode::eFORCE);

                        pushedCcts.insert(mirroredCctComponent);
                    }
                }
            }
        }

        for (ShooterMirroredCharacterComponent* pushedCctToRemove : pushedCctsToRemove)
        {
            pushedCcts.erase(pushedCctToRemove);
        }

        pushedCctsToRemove.clear();
    }
}

void ShooterCarAttackSystem::PrepareForRemove()
{
}
