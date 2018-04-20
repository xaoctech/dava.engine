#include "Systems/ShooterExternalImpulseSystem.h"
#include "Components/ExternalImpulseComponent.h"
#include "ShooterConstants.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>

#include <Physics/Controllers/CapsuleCharacterControllerComponent.h>

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterExternalImpulseSystem)
{
    ReflectionRegistrator<ShooterExternalImpulseSystem>::Begin()[M::Tags("gm_shooter")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &ShooterExternalImpulseSystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY, SP::Type::FIXED, 19.1f)]
    .End();
}

ShooterExternalImpulseSystem::ShooterExternalImpulseSystem(Scene* scene)
    : BaseSimulationSystem(scene, DAVA::ComponentMask())
    , entityGroup(scene->AquireEntityGroup<ExternalImpulseComponent, CapsuleCharacterControllerComponent>())
{
}

void ShooterExternalImpulseSystem::ProcessFixed(DAVA::float32 dt)
{
    for (Entity* entity : entityGroup->GetEntities())
    {
        ExternalImpulseComponent* impulseComponent = entity->GetComponent<ExternalImpulseComponent>();
        if (!impulseComponent->IsZero())
        {
            CapsuleCharacterControllerComponent* cctComponent = entity->GetComponent<CapsuleCharacterControllerComponent>();

            // stop flying if touched the ground
            if (impulseComponent->applied && cctComponent->IsGrounded())
            {
                cctComponent->SetMovementMode(CharacterControllerComponent::MovementMode::Walking);
                impulseComponent->Reset();
                continue;
            }

            if (!impulseComponent->applied)
            {
                cctComponent->SetMovementMode(CharacterControllerComponent::MovementMode::Flying);
                impulseComponent->applied = true;
            }

            // calculate and update velocity with gravity
            if (impulseComponent->velocity == Vector3::Zero)
            {
                impulseComponent->velocity =
                impulseComponent->direction * impulseComponent->magnitude / SHOOTER_CAR_IMPULSE_MAGNITUDE_PER_VELOCITY;
            }
            impulseComponent->velocity.z += -9.81f * dt;

            cctComponent->Move(impulseComponent->velocity * dt);
        }
    }
}

void ShooterExternalImpulseSystem::PrepareForRemove()
{
}
