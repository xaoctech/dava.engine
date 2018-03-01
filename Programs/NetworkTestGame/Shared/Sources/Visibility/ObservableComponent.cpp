#include "ObservableComponent.h"

#include <Reflection/ReflectionRegistrator.h>

using namespace DAVA;

REGISTER_CLASS(ObservableComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(ObservableComponent)
{
    ReflectionRegistrator<ObservableComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

ObservableComponent::ObservableComponent()
{
}

Component* ObservableComponent::Clone(Entity* toEntity)
{
    ObservableComponent* component = new ObservableComponent();
    component->SetEntity(toEntity);
    return component;
}
