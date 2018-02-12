#include "PowerupComponent.h"
#include "Reflection/ReflectionRegistrator.h"

using namespace DAVA;

REGISTER_CLASS(PowerupComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(PowerupComponent)
{
    ReflectionRegistrator<PowerupComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("Descriptor", &PowerupComponent::descriptor)[M::Replicable()]
    .ConstructorByPointer()
    .End();
}

PowerupComponent::PowerupComponent()
{
}

Component* PowerupComponent::Clone(Entity* toEntity)
{
    PowerupComponent* component = new PowerupComponent();
    component->SetEntity(toEntity);
    return component;
}
