#include "SpeedModifierComponent.h"
#include "Reflection/ReflectionRegistrator.h"

using namespace DAVA;

REGISTER_CLASS(SpeedModifierComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(SpeedModifierComponent)
{
    ReflectionRegistrator<SpeedModifierComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .Field("Factor", &SpeedModifierComponent::factor)[M::Replicable()]
    .ConstructorByPointer()
    .End();
}

SpeedModifierComponent::SpeedModifierComponent()
{
}

Component* SpeedModifierComponent::Clone(Entity* toEntity)
{
    SpeedModifierComponent* component = new SpeedModifierComponent();
    component->SetEntity(toEntity);
    return component;
}
