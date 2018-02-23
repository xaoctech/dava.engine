#include "Components/ShooterAimComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Scene3D/Entity.h>

REGISTER_CLASS(ShooterAimComponent);

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterAimComponent)
{
    using namespace DAVA;

    ReflectionRegistrator<ShooterAimComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("finalAngleX", &ShooterAimComponent::GetFinalAngleX, &ShooterAimComponent::SetFinalAngleX)[M::Replicable(), M::HiddenField()]
    .Field("finalAngleZ", &ShooterAimComponent::GetFinalAngleZ, &ShooterAimComponent::SetFinalAngleZ)[M::Replicable(), M::HiddenField()]
    .Field("currentAngleX", &ShooterAimComponent::GetCurrentAngleX, &ShooterAimComponent::SetCurrentAngleX)[M::Replicable(), M::HiddenField()]
    .Field("currentAngleZ", &ShooterAimComponent::GetCurrentAngleZ, &ShooterAimComponent::SetCurrentAngleZ)[M::Replicable(), M::HiddenField()]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* ShooterAimComponent::Clone(DAVA::Entity* toEntity)
{
    ShooterAimComponent* component = new ShooterAimComponent();
    component->SetFinalAngleX(GetFinalAngleX());
    component->SetFinalAngleZ(GetFinalAngleZ());
    component->SetCurrentAngleX(GetCurrentAngleX());
    component->SetCurrentAngleZ(GetCurrentAngleZ());

    component->SetEntity(toEntity);

    return component;
}

void ShooterAimComponent::SetFinalAngleX(DAVA::float32 value)
{
    static const DAVA::float32 MAX_ANGLE_X = DAVA::PI / 3.0f;
    finalAngleX = DAVA::Clamp(value, -MAX_ANGLE_X, MAX_ANGLE_X);
}

DAVA::float32 ShooterAimComponent::GetFinalAngleX() const
{
    return finalAngleX;
}

void ShooterAimComponent::SetFinalAngleZ(DAVA::float32 value)
{
    finalAngleZ = value;
}

DAVA::float32 ShooterAimComponent::GetFinalAngleZ() const
{
    return finalAngleZ;
}

void ShooterAimComponent::SetCurrentAngleX(DAVA::float32 value)
{
    currentAngleX = value;
}

DAVA::float32 ShooterAimComponent::GetCurrentAngleX() const
{
    return currentAngleX;
}

void ShooterAimComponent::SetCurrentAngleZ(DAVA::float32 value)
{
    currentAngleZ = value;
}

DAVA::float32 ShooterAimComponent::GetCurrentAngleZ() const
{
    return currentAngleZ;
}
