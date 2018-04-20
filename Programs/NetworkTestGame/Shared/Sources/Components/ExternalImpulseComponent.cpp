#include "ExternalImpulseComponent.h"

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>
#include <Logger/Logger.h>

using namespace DAVA;
REGISTER_CLASS(ExternalImpulseComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ExternalImpulseComponent)
{
    ReflectionRegistrator<ExternalImpulseComponent>::Begin()[M::Replicable(M::Privacy::PRIVATE)]
    .ConstructorByPointer()
    .Field("Velocity", &ExternalImpulseComponent::velocity)[M::Replicable()]
    .End();
}

Component* ExternalImpulseComponent::Clone(Entity* toEntity)
{
    ExternalImpulseComponent* component = new ExternalImpulseComponent();
    component->SetEntity(toEntity);
    return component;
}

bool ExternalImpulseComponent::IsZero() const
{
    return (magnitude == 0.f || direction == Vector3::Zero) && velocity == Vector3::Zero;
}

void ExternalImpulseComponent::Reset()
{
    magnitude = 0.f;
    direction = Vector3::Zero;
    velocity = Vector3::Zero;
    applied = false;
}
