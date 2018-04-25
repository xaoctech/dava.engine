#include "Systems/ShooterCarAttackSystem.h"
#include "Components/HealthComponent.h"
#include "Components/ExternalImpulseComponent.h"
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
    ReflectionRegistrator<ShooterCarAttackSystem>::Begin()[M::SystemTags("gm_shooter", "server")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterCarAttackSystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::Gameplay, SPI::Type::Fixed, 19.0f)]
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

    const CollisionSingleComponent* collisionSingleComponent = GetScene()->GetSingleComponent<CollisionSingleComponent>();
    DVASSERT(collisionSingleComponent != nullptr);

    for (CapsuleCharacterControllerComponent* cctComponent : ccts->components)
    {
        Entity* cctEntity = cctComponent->GetEntity();
        DVASSERT(cctEntity != nullptr);

        DynamicBodyComponent* cctMirrorBodyComponent = cctEntity->GetComponent<DynamicBodyComponent>();
        DVASSERT(cctMirrorBodyComponent != nullptr);

        ExternalImpulseComponent* impulseComponent = cctEntity->GetComponent<ExternalImpulseComponent>();
        DVASSERT(impulseComponent != nullptr);

        if (impulseComponent->IsZero())
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

                        if (IsCar(collisions[0].first))
                        {
                            normal.x *= -1.0f;
                            normal.y *= -1.0f;
                        }

                        impulseComponent->magnitude = impulseMagnitude;
                        impulseComponent->direction = normal;
                    }
                }
            }
        }
    }
}

void ShooterCarAttackSystem::PrepareForRemove()
{
}
