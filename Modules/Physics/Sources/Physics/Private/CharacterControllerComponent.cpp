#include "Physics/CharacterControllerComponent.h"
#include "Physics/Private/PhysicsMath.h"

#include <Scene3D/Scene.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/CameraComponent.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Time/SystemTimer.h>
#include <Logger/Logger.h>

#include <physx/characterkinematic/PxController.h>

// For now use default filters for all moving
static physx::PxControllerFilters filters;

namespace DAVA
{
void CharacterControllerComponent::Move(Vector3 displacement)
{
    if (controller != nullptr)
    {
        physx::PxControllerCollisionFlags resultFlags = controller->move(PhysicsMath::Vector3ToPxVec3(displacement), 0.0f, SystemTimer::GetRealFrameDelta(), filters);
        grounded = (resultFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN);

        SyncEntityTransform();
    }
}

void CharacterControllerComponent::SimpleMove(Vector3 displacement)
{
    if (controller != nullptr)
    {
        const Vector3 displacementAlongUp = GetUpDirection().DotProduct(displacement) * GetUpDirection();
        displacement -= displacementAlongUp;

        displacement += PhysicsMath::PxVec3ToVector3(controller->getScene()->getGravity()) * SystemTimer::GetRealFrameDelta();

        physx::PxControllerCollisionFlags resultFlags = controller->move(PhysicsMath::Vector3ToPxVec3(displacement), 0.0f, SystemTimer::GetRealFrameDelta(), filters);
        grounded = (resultFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN);

        SyncEntityTransform();
    }
}

void CharacterControllerComponent::Teleport(Vector3 worldPosition)
{
    if (controller != nullptr)
    {
        controller->setPosition(PhysicsMath::Vector3ToPxExtendedVec3(worldPosition));

        SyncEntityTransform();
    }
}

bool CharacterControllerComponent::IsGrounded() const
{
    return grounded;
}

Vector3 CharacterControllerComponent::GetUpDirection() const
{
    return upDirection;
}

void CharacterControllerComponent::SetUpDirection(Vector3 newUpDirection)
{
    newUpDirection.Normalize();
    upDirection = newUpDirection;

    ScheduleUpdate();
}

void CharacterControllerComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    archive->SetVector3("characterController.up", upDirection);
}

void CharacterControllerComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
    upDirection = archive->GetVector3("characterController.up", Vector3::UnitZ);
}

void CharacterControllerComponent::ScheduleUpdate()
{
    if (controller != nullptr)
    {
        Entity* entity = GetEntity();
        DVASSERT(entity != nullptr);

        Scene* scene = entity->GetScene();
        DVASSERT(scene != nullptr);

        scene->physicsSystem->SheduleUpdate(this);
    }
}

void CharacterControllerComponent::CopyFieldsToComponent(CharacterControllerComponent* dest)
{
    dest->upDirection = upDirection;
}

void CharacterControllerComponent::SyncEntityTransform()
{
    DVASSERT(controller != nullptr);

    Entity* entity = GetEntity();
    DVASSERT(entity != nullptr);

    Matrix4 transform = entity->GetWorldTransform();
    transform.SetTranslationVector(PhysicsMath::PxExtendedVec3ToVector3(controller->getPosition()));
    entity->SetWorldTransform(transform);
}

DAVA_VIRTUAL_REFLECTION_IMPL(CharacterControllerComponent)
{
    ReflectionRegistrator<CharacterControllerComponent>::Begin()
    .Field("Up direction", &CharacterControllerComponent::GetUpDirection, &CharacterControllerComponent::SetUpDirection)
    .End();
}

} // namespace DAVA