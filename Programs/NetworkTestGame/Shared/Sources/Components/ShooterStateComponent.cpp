#include "Components/ShooterStateComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

REGISTER_CLASS(ShooterStateComponent);

DAVA_VIRTUAL_REFLECTION_IMPL(ShooterStateComponent)
{
    using namespace DAVA;

    ReflectionRegistrator<ShooterStateComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("RaycastAttackFrameId", &ShooterStateComponent::raycastAttackFrameId)[M::Replicable(), M::HiddenField()]
    .ConstructorByPointer()
    .End();
}

DAVA::Component* ShooterStateComponent::Clone(DAVA::Entity* toEntity)
{
    ShooterStateComponent* component = new ShooterStateComponent();
    component->SetEntity(toEntity);
    return component;
}