#include "PowerupCatcherComponent.h"
#include "Reflection/ReflectionRegistrator.h"

using namespace DAVA;

REGISTER_CLASS(PowerupCatcherComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(PowerupCatcherComponent)
{
    ReflectionRegistrator<PowerupCatcherComponent>::Begin()[M::Replicable(M::Privacy::SERVER_ONLY)]
    .ConstructorByPointer()
    .End();
}

PowerupCatcherComponent::PowerupCatcherComponent()
{
}

Component* PowerupCatcherComponent::Clone(Entity* toEntity)
{
    PowerupCatcherComponent* component = new PowerupCatcherComponent();
    component->SetEntity(toEntity);
    return component;
}
